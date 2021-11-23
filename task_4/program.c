#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

int fd[2];
int A_pid, B_pid, C_pid;

void process_A(void* shmem) {
	close(fd[0]);
	int value;
	char c;

	while (1) {
		if (scanf("%d", &value) == 1) { 
			write(fd[1], &value, sizeof(int));
		}
		else { 
			printf("Invalid input. Enter integer value\n");
			while((c = getchar()) != '\n' && c != EOF);
		};
	}
}

void handle_sigusr1_B() {
	kill(getppid(), SIGUSR1);
}

void process_B(void* shmem) {
	close(fd[1]);
	int value;

	struct sigaction sa = { 0 }; // Process B SIGUSR1 initialization
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = &handle_sigusr1_B;
	sigaction(SIGUSR1, &sa, NULL);

	while (1) {
		read(fd[0], &value, sizeof(int));
		value *= value;
		memcpy(shmem, &value, sizeof(value));
	}
}

bool value_changed = false;

void* process_C_1(void* shmem) {
	int value = 0; 
	while (1) {
		if (value != *(int*)shmem) {
			value_changed = true;
			value = *(int*)shmem;
		}
		usleep(5000);
	}
}

void* process_C_2(void* shmem) {
	while (1) {
                if (value_changed) {
			value_changed = false;
			printf("Value = %d\n", *(int*)shmem);
			if (*(int*)shmem == 100) kill(B_pid, SIGUSR1);
		}
		else {
			printf("I am alive!\n");
		}
		sleep(1);
        }
}

void process_C(void* shmem) {
	pthread_t c_1,c_2;	
	pthread_create(&c_1, NULL, &process_C_1, shmem);	
	pthread_create(&c_2, NULL, &process_C_2, shmem);	

	pthread_join(c_1, NULL);
	pthread_join(c_2, NULL);
}

int create_process(void (*fun_ptr)(void*), void* shmem) {
	int pid = fork();
	switch (pid) {
		case -1: {
			printf("An error occured with creating process\n");
                	return -1;	 
		} break;
		case 0: {
			fun_ptr(shmem);
			exit(0);	
		} break;
		default: return pid;	
	}
}

void handle_sigusr1_parent() {
        kill(C_pid, SIGKILL);
	kill(B_pid, SIGKILL);
	kill(A_pid, SIGKILL);
}

int main() {
	int protection = PROT_READ | PROT_WRITE; // Shared memory initialization
  	int visibility = MAP_SHARED | MAP_ANONYMOUS;
	void* shmem = mmap(NULL, sizeof(int), protection, visibility, -1, 0);
	int shmem_value = 0;		
	memcpy(shmem, &shmem_value, sizeof(shmem_value));
	
	struct sigaction sa = { 0 }; // SIGUSR1 initialization
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = &handle_sigusr1_parent;
        sigaction(SIGUSR1, &sa, NULL);

	if (pipe(fd) == -1) { // Open pipe
		printf("An error occured with opening the pipe\n");
		return 1;
	}

	A_pid = create_process(&process_A, shmem);
	B_pid = create_process(&process_B, shmem);
	C_pid = create_process(&process_C, shmem);

	waitpid(C_pid, NULL, 0);
	waitpid(B_pid, NULL, 0);
	waitpid(A_pid, NULL, 0);
	
	munmap(shmem, sizeof(int));

	printf("Program execution is over\n");

	return 0;
}
