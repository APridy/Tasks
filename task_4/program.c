#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

int fd[2];

void process_A() {
	close(fd[0]);
	
	printf("Process A\n");
	int value;
	while(1) {
		scanf("%d", &value);
		write(fd[1], &value, sizeof(int));
		//close(fd[1]);
	}
}

void process_B() {
	close(fd[1]);

        printf("Process B\n");
	int value;
	while(1) {
		read(fd[0], &value, sizeof(int));
		//close(fd[0]);
		printf("Got from A: %d\n", value*value);
	}
}

void process_C() {
        printf("Process C\n");
}

int create_process(void (*fun_ptr)() ) {
	int pid = fork();

	if (pid != 0) {			
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

	create_process(&process_A);
	create_process(&process_B);
	create_process(&process_C);

	return 0;
}

