#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SHMEM_FILE "shmem.c"

int fd[2];

void process_A() {
	close(fd[0]);
	
	printf("Process A\n");
	int value;
	char c;
	while(1) {
		if(scanf("%d", &value) == 1) { 
			write(fd[1], &value, sizeof(int));
		}
		else { 
			printf("Invalid input. Enter numeric value\n");
			while((c = getchar()) != '\n' && c != EOF);
		};
	}
}

void process_B() {
	close(fd[1]);

        printf("Process B\n");
	int value;
	while(1) {
		read(fd[0], &value, sizeof(int));
		printf("Got from A: %d\n", value*value);
	}
}

void process_C() {
        printf("Process C\n");
	while(1) {
		sleep(1);
		printf("I am alive!\n");
	}
}

int create_process(void (*fun_ptr)() ) {
	int pid = fork();

	if (pid == 0) {			
		fun_ptr();
		exit(0);
	}

	return pid;
}

int main() {
	int A_pid, B_pid, C_pid;		

	if (pipe(fd) == -1) {

		printf("An error occured when opening the pipe\n");
		return 1;
	}

	A_pid = create_process(&process_A);
	B_pid = create_process(&process_B);
	C_pid = create_process(&process_C);

	wait(NULL);

	return 0;
}

