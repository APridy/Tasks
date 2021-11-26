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
#define STOP_PROCESS (*(struct shared_memory*)shmem).stop_process

int fd[2];
struct shared_memory {
	int value;
	bool val_changed;
	pid_t pid_A;
	pid_t pid_B;
	pid_t pid_C;
	bool stop_process;
};
void* shmem;

void clear_stdin() {
	char c;
	while((c = getchar()) != '\n' && c != EOF);
}

void handle_sigterm_A() {}

void process_A() {
	close(fd[0]);
	int value;

	struct sigaction sa = { 0 }; // Process A SIGTERM initialization
	sa.sa_handler = &handle_sigterm_A;
	sigaction(SIGTERM, &sa, NULL);
      
	while (!STOP_PROCESS) {
		switch (scanf("%d", &value)) {
			case 1:
				write(fd[1], &value, sizeof(int)); 
				break;
			case EOF: 
				break;	
			default:
				printf("Invalid input. Enter integer value\n");
				clear_stdin();
				break;
		}
	}

	close(fd[1]);
}

void handle_sigusr1_B() {
	STOP_PROCESS = true;
	kill(PID_A, SIGTERM);
	kill(PID_B, SIGTERM);
}

void handle_sigterm_B() {}

void process_B() {
	close(fd[1]);
	int value;
	bool val_changed = true;

	struct sigaction sa = { 0 }; // Process B SIGUSR1 initialization
	sa.sa_handler = &handle_sigusr1_B;
	sigaction(SIGUSR1, &sa, NULL);

	struct sigaction sa1 = { 0 }; // Process B SIGTERM initialization
	sa1.sa_handler = &handle_sigterm_B;
	sigaction(SIGTERM, &sa1, NULL);
		
	while (!STOP_PROCESS) {
		if (read(fd[0], &value, sizeof(int)) != EOF) {
			value *= value;
			memcpy(shmem, &value, sizeof(value));
			memcpy(&SHMEM_VALUE_CHANGED, &val_changed, sizeof(bool));
		}
	}
	close(fd[0]);
}

bool value_changed = false;

void* process_C_1() {
	int value = 0; 
	while (!STOP_PROCESS) {
		if (SHMEM_VALUE_CHANGED) {
			SHMEM_VALUE_CHANGED = false;
			value_changed = true;
			value = SHMEM_VALUE;
		}
		usleep(5000);
	}
}

void* process_C_2() {
	while (!STOP_PROCESS) {
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

void process_C() {
	pthread_t c_1,c_2;	
	pthread_create(&c_1, NULL, &process_C_1, shmem);	
	pthread_create(&c_2, NULL, &process_C_2, shmem);	

	pthread_join(c_1, NULL);
	pthread_join(c_2, NULL);
}

int create_process(void (*fun_ptr)()) {
	int pid = fork();
	switch (pid) {
		case -1: {
			printf("An error occured with creating process\n");
			return -1;	 
		} break;
		case 0: {
			fun_ptr();
			exit(0);	
		} break;
		default: return pid;	
	}
}

int main() {
	struct shared_memory shmem_init = { .value = 0, .val_changed = false, .stop_process = false}; // Shared memory initialization
	int shmem_value = 0;
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_SHARED | MAP_ANONYMOUS;

	if((shmem = mmap(NULL, sizeof(struct shared_memory), protection, visibility, -1, 0)) == MAP_FAILED) { 
	printf("An error occured while mapping shared memory");
		return 1;
	}

	memcpy(shmem, &shmem_init, sizeof(struct shared_memory));
	
	if (pipe(fd) == -1) { // Open pipe
		printf("An error occured while opening the pipe\n");
		return 2;
	}

	PID_A = create_process(&process_A);
	PID_B = create_process(&process_B);
	PID_C = create_process(&process_C);

	waitpid(PID_C, NULL, 0);
	waitpid(PID_B, NULL, 0);
	waitpid(PID_A, NULL, 0);
	
	if((munmap(shmem, sizeof(struct shared_memory))) == -1) {
		printf("An error occured while unmapping shared memory");
		return 3;
	}

	printf("Program execution is over\n");
	return 0;
}
