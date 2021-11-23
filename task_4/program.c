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

#define SHMEM_VALUE (*(struct shared_memory*)shmem).value
#define SHMEM_VALUE_CHANGED (*(struct shared_memory*)shmem).val_changed
#define PID_A (*(struct shared_memory*)shmem).pid_A
#define PID_B (*(struct shared_memory*)shmem).pid_B
#define PID_C (*(struct shared_memory*)shmem).pid_C

int fd[2];
struct shared_memory {
	int value;
	bool val_changed;
	pid_t pid_A;
	pid_t pid_B;
	pid_t pid_C;
};
void* shmem;

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
	kill(PID_C, SIGKILL);
        kill(PID_A, SIGKILL);
        kill(PID_B, SIGKILL);
}

void process_B(void* shmem) {
	close(fd[1]);
	int value;
	bool val_changed = true;

	struct sigaction sa = { 0 }; // Process B SIGUSR1 initialization
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = &handle_sigusr1_B;
	sigaction(SIGUSR1, &sa, NULL);

	while (1) {
		read(fd[0], &value, sizeof(int));
		value *= value;
		memcpy(shmem, &value, sizeof(value));
		memcpy(shmem + sizeof(int), &val_changed, sizeof(bool));
	}
}

bool value_changed = false;

void* process_C_1(void* shmem) {
	int value = 0; 
	while (1) {
		if (SHMEM_VALUE_CHANGED) {
			SHMEM_VALUE_CHANGED = false;
			value_changed = true;
			value = SHMEM_VALUE;
		}
		usleep(5000);
	}
}

void* process_C_2(void* shmem) {
	while (1) {
                if (value_changed) {
			value_changed = false;
			printf("Value = %d\n", SHMEM_VALUE);
			if (SHMEM_VALUE == 100) kill(PID_B, SIGUSR1);
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

int main() {
	struct shared_memory shmem_init = { .value = 0, .val_changed = false}; // Shared memory initialization
        int shmem_value = 0;
        int protection = PROT_READ | PROT_WRITE;
        int visibility = MAP_SHARED | MAP_ANONYMOUS;
        shmem = mmap(NULL, sizeof(struct shared_memory), protection, visibility, -1, 0);
        memcpy(shmem, &shmem_init, sizeof(struct shared_memory));

	if (pipe(fd) == -1) { // Open pipe
		printf("An error occured with opening the pipe\n");
		return 1;
	}

	PID_A = create_process(&process_A, shmem);
	PID_B = create_process(&process_B, shmem);
	PID_C = create_process(&process_C, shmem);

	waitpid(PID_C, NULL, 0);
	waitpid(PID_B, NULL, 0);
	waitpid(PID_A, NULL, 0);
	
	munmap(shmem, sizeof(struct shared_memory));

	printf("Program execution is over\n");

	return 0;
}
