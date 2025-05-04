shm_proc: shm_processes.c
	gcc shm_processes.c -D_SVID_SOURCE -D_GNU_SOURCE -Wall -std=c99 -Werror=cpp -pedantic  -o shm_proc

banking_System: banking_System.c
	gcc banking_System.c -D_SVID_SOURCE -D_GNU_SOURCE -Wall -std=c99 -Werror=cpp -pedantic  -o banking_System