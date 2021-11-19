#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <string.h>

#define SEGMENT_SIZE 1024

int fd[2];

void process_A(int* sh_mem) {
	close(fd[0]);
	
	printf("Process A\n");
	int value;
	char c;
	while(1) {
		if(scanf("%d", &value) == 1) { 
			write(fd[1], &value, sizeof(int));
		}
		else { 
			printf("Invalid input. Enter integer value\n");
			while((c = getchar()) != '\n' && c != EOF);
		};
	}
}

void process_B(int* sh_mem) {
	close(fd[1]);

        printf("Process B\n");
	int value;
	while(1) {
		read(fd[0], &value, sizeof(int));
		value *= value;
		memmove(sh_mem, &value, sizeof(int));
	}
}

void process_C(int* sh_mem) {
        printf("Process C\n");
	while(1) {
		sleep(1);
		printf("I am alive! ");
		printf("%d\n", *sh_mem);
	}
}

int create_process(void (*fun_ptr)(int*), int* sh_mem) {
	int pid = fork();
	switch (pid) {
		case -1: {
			printf("An error occured with creating process\n");
                	return -1;	 
		} break;
		case 0: {
			fun_ptr(sh_mem);
			exit(0);	
		} break;
		default: return pid;	
	}
}

int main() {
	int A_pid, B_pid, C_pid, number = 0;		
	int segment_id = shmget (IPC_PRIVATE, SEGMENT_SIZE,
                        IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	int *sh_mem = (int *) shmat(segment_id, NULL, 0);

	printf("Segment ID %d\n", segment_id);
	printf("Attached at %p\n", sh_mem);
	memmove(sh_mem, &number, sizeof(int));

	if (pipe(fd) == -1) {
		printf("An error occured with opening the pipe\n");
		return 1;
	}

	A_pid = create_process(&process_A, sh_mem);
	B_pid = create_process(&process_B, sh_mem);
	C_pid = create_process(&process_C, sh_mem);

	wait(NULL);

	return 0;
}

