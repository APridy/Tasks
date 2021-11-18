#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 


void process_A() {
	printf("Process A\n");
}

void process_B() {
        printf("Process B\n");
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

	create_process(&process_A);
	create_process(&process_B);
	create_process(&process_C);

}

