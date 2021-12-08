#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define QUEUE_KEY 252525
#define NUM_OF_DATA_TYPES 3

struct mystruct {
	int a;
	int b;
	int c;
};

union data {
	int num;
	char arr[5];
	struct mystruct num3;
};

struct message{
	long mtype;
	uint8_t msg[sizeof(union data)];
};

char *filename[NUM_OF_DATA_TYPES] = {"int.txt", "array.txt", "struct.txt"};
struct msqid_ds qstatus;
pid_t pid[NUM_OF_DATA_TYPES];
int msgid;

char* itoa(int num) {
	int i = 1;	
	int divider = 1;
	if(num == 0) return "0"; 
	while(num/(divider*10) != 0) {
		divider *= 10;
		i++;
	}
	if(num < 0) i++;
	char* str = malloc(i + 1);
	i = 0;
	if(num < 0) {
		str[i] = '-';
		i++;
		num *= -1;
	}
	while(num > 10) {
		str[i] = ((num / divider) + 48);
		i++;
		num = num % divider;
		divider = divider/10;
	}
	str[i] = num + 48;
	i++;
	str[i] = '\0';
	return str;
}

void write_data_to_file(char* data, char* filename) {
	FILE *fp = fopen(filename,"a+");
	if(msgctl(msgid,IPC_STAT,&qstatus)<0){
		perror("Msgctl failed");
		exit(1);
	}
	char s[30];
	strftime(s, 30, "%d.%m.%Y %H:%M:%S", localtime(&(qstatus.msg_rtime)));
	fprintf(fp,"%s : %s\n",s,data);
	fclose(fp);
}

void recieve_int() {
	struct message msg_buf ;
	union data data_buf;

	while(1) {
		if(msgrcv(msgid, &msg_buf, sizeof(union data), 1, MSG_NOERROR) == -1
						&& errno == EIDRM) break;
		memcpy(&data_buf, msg_buf.msg, sizeof(union data));
		printf("Recieved integer: %d\n", data_buf.num);
		write_data_to_file(itoa(data_buf.num),filename[0]);
	}
}

void recieve_arr() {
	struct message msg_buf;
	union data data_buf;

	while(1) {
		if(msgrcv(msgid, &msg_buf, sizeof(union data), 2, MSG_NOERROR) == -1
						&& errno == EIDRM) break;
		memcpy(&data_buf, msg_buf.msg, sizeof(union data));
		printf("Recieved char[5]: %s\n", data_buf.arr);
		write_data_to_file(data_buf.arr,filename[1]);
	}
}

void recieve_struct() {
	struct message msg_buf;
	union data data_buf;

	while(1) {
		if(msgrcv(msgid, &msg_buf, sizeof(union data), 3, MSG_NOERROR) == -1 
						&& errno == EIDRM) break;
		memcpy(&data_buf, msg_buf.msg, sizeof(union data));
		printf("Recieved struct: %d , %d , %d\n", data_buf.num3.a, 
					data_buf.num3.b, data_buf.num3.c);
		char str[45] = "";
		strcat(str,itoa(data_buf.num3.a));
		strcat(str," , ");
		strcat(str,itoa(data_buf.num3.b));
		strcat(str," , ");
		strcat(str,itoa(data_buf.num3.c));
		write_data_to_file(str,filename[2]);
	}
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

void (*recieve_data[NUM_OF_DATA_TYPES])() = {
	&recieve_int, 
	&recieve_arr, 
	&recieve_struct
};

int main(int argc, char **argv) {
	
	char arg = 0;
	while ((arg = getopt(argc,argv,"Di:c:s:")) != -1) {
		switch (arg) {
			case 'D': 
				break;
			case 'i': 
				filename[0] = optarg;
				break;
			case 'c': 
				filename[1] = optarg;
				break;
			case 's': 
				filename[2] = optarg;
				break;
			case '?': 
				printf("Invalid argument!\n");
				return 1;
		};
	};

	key_t msgkey = QUEUE_KEY;
	msgid = msgget(msgkey, IPC_CREAT | 0666/*| IPC_EXCL*/);
	
	for(int i = 0; i < NUM_OF_DATA_TYPES; i++) {
		pid[i] = create_process(recieve_data[i]);
	}

	struct message msg_buf;
	msgrcv(msgid, &msg_buf, sizeof(union data), NUM_OF_DATA_TYPES + 1, MSG_NOERROR);	
	msgctl(msgid,IPC_RMID,NULL);

	printf("Program execution ended.\n");

	return 0;

}
