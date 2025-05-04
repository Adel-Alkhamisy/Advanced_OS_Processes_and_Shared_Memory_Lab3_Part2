/**
 * Parent-Child Banking Simulation Using Shared Memory
 * 
 * This program simulates a banking system where two processes interact with a shared account:
 * - Parent Process ("Dear old Dad"): Periodically checks the account balance and may deposit
 *   money if the balance is low (≤ $100) and the deposit amount is even.
 * - Child Process ("Poor Student"): Periodically attempts to withdraw random amounts from
 *   the account if sufficient funds are available.
 * 
 * The processes coordinate using shared memory with two integers:
 * - sharedMemory[0] (BankAccount): Represents the bank account balance
 * - sharedMemory[1] (Turn): Acts as a synchronization flag (turn indicator)
 *   to prevent race conditions between processes
 * 
 * Each process performs 25 operations with random delays between actions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

void ChildProcess(int sharedMemory[]);    // Student process function
void ParentProcess(int sharedMemory[]);   // Dad process function

int main(int argc, char *argv[])
{
    int    sharedMemoryID;        // ID for the shared memory segment
    int    *sharedMemoryPointer;  // Pointer to the shared memory
    pid_t  processID;             // Process ID after fork
    int    exitStatus;            // Exit status of child process
    
    // Create shared memory segment with size for 4 integers
    sharedMemoryID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
    if (sharedMemoryID < 0) {
        printf("*** shmget error (main) ***\n");
        exit(1);
    }
    printf("Process has received a shared memory of four integers...\n");
    
    // Attach the shared memory segment
    sharedMemoryPointer = (int *) shmat(sharedMemoryID, NULL, 0);
    if (*sharedMemoryPointer == -1) {
        printf("*** shmat error (main) ***\n");
        exit(1);
    }
    printf("Process has attached the shared memory...\n");
    
    // Initialize the BankAccount and Turn indicator
    sharedMemoryPointer[0] = 0;   // Initial bank balance = 0
    sharedMemoryPointer[1] = 0;   // Initial turn = 0 (parent's turn)
    printf("Bank initialized to: %d\n", sharedMemoryPointer[0]);
    printf("Bank is about to fork a main process...\n");
    
    // Create child process
    processID = fork();
    if (processID < 0) {
        printf("*** fork error (main) ***\n");
        exit(1);
    }
    else if (processID == 0) {
        // Child process (Poor Student) code
        ChildProcess(sharedMemoryPointer);
        exit(0);
    } else {
        // Parent process (Dear old Dad) code
        ParentProcess(sharedMemoryPointer);
    }
    
    // Wait for child process to complete
    wait(&exitStatus);
    printf("Process has detected the completion of its child...\n");
    
    // Clean up shared memory
    shmdt((void *) sharedMemoryPointer);
    printf("Process has detached its shared memory...\n");
    shmctl(sharedMemoryID, IPC_RMID, NULL);
    printf("Process has removed its shared memory...\n");
    printf("Process exits...\n");
    //  exit(0);
}

void ChildProcess(int sharedMemory[])
{
    int BankAccount, withdrawal;  // Local variables for account balance and withdrawal amount
    int i;  // Loop counter (moved outside for loop for C89 compatibility)
    
    // Seed the random number generator with process ID
    srand(getpid());
    
    // Perform 25 withdrawal attempts
    for (i=0; i<25; i++) {
        // Random delay between 0-5 seconds
        sleep(rand()%6);
        
        // Wait until it's the student's turn (Turn == 1)
        while(sharedMemory[1] != 1);
        
        // Read current account balance
        BankAccount = sharedMemory[0];
        
        // Generate random withdrawal amount (0-50)
        withdrawal = rand() % 51;
        printf("Poor Student needs $%d\n", withdrawal);
        
        // Check if sufficient funds are available
        if (withdrawal <= BankAccount) {
            // Process the withdrawal
            BankAccount -= withdrawal;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdrawal, BankAccount);
        } else {
            // Not enough funds for withdrawal
            printf("Poor Student: Not Enough Cash ($%d)\n", BankAccount);
        }
        
        // Update the shared account balance
        sharedMemory[0] = BankAccount;
        
        // Set Turn flag to 0 (parent's turn)
        sharedMemory[1] = 0;
    }
}

void ParentProcess(int sharedMemory[]) {
    int BankAccount, deposit;  // Local variables for account balance and deposit amount
    int i;  // Loop counter (moved outside for loop for C89 compatibility)
    
    // Seed the random number generator with process ID
    srand(getpid());
    
    // Perform 25 deposit attempts
    for (i=0; i<25; i++) {
        // Random delay between 0-5 seconds
        sleep(rand()%6);
        
        // Read current account balance
        BankAccount = sharedMemory[0];
        
        // Wait until it's the parent's turn (Turn == 0)
        while(sharedMemory[1] != 0);
        
        // Only deposit if balance is low (≤ $100)
        if (BankAccount <= 100) {
            // Generate random deposit amount (0-100)
            deposit = rand()%101;
            
            // Only deposit if amount is even
            if (deposit % 2 == 0) {
                BankAccount += deposit;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", deposit, BankAccount);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
            
            // Update the shared account balance
            sharedMemory[0] = BankAccount;
        } else {
            // Skip deposit if balance is already high
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", BankAccount);
        }
        
        // Set Turn flag to 1 (child's turn)
        sharedMemory[1] = 1;
    }
}