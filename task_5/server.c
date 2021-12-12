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
#define INT_STR_SIZE 13

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
int msgid;

void write_data_to_file(char* data, char* filename) {
	FILE *fp = fopen(filename,"a+");
	struct msqid_ds qstatus;
	if(msgctl(msgid,IPC_STAT,&qstatus)<0){
		printf("Msgctl error!\n");
		exit(1);
	}
	char s[30];
	strftime(s, 30, "%d.%m.%Y %H:%M:%S", localtime(&(qstatus.msg_rtime)));
	fprintf(fp,"%s : %s\n",s,data);
	fclose(fp);
}

void recieve_int(union data data_buf) {
	char buffer[INT_STR_SIZE];
	printf("Recieved integer: %d\n", data_buf.num);
	snprintf(buffer,INT_STR_SIZE,"%d",data_buf.num);
	write_data_to_file(buffer,filename[0]);
}

void recieve_arr(union data data_buf) {
	printf("Recieved char[5]: %s\n", data_buf.arr);
	write_data_to_file(data_buf.arr,filename[1]);
}

void recieve_struct(union data data_buf) {
	char buffer[INT_STR_SIZE];
	char str[INT_STR_SIZE*3 + 6] = "";
	printf("Recieved struct: %d , %d , %d\n", data_buf.num3.a, 
				data_buf.num3.b, data_buf.num3.c);
	snprintf(buffer,INT_STR_SIZE,"%d",data_buf.num3.a);
	strcat(str,buffer);
	strcat(str," , ");
	snprintf(buffer,INT_STR_SIZE,"%d",data_buf.num3.b);
	strcat(str,buffer);
	strcat(str," , ");
	snprintf(buffer,INT_STR_SIZE,"%d",data_buf.num3.c);
	strcat(str,buffer);
	write_data_to_file(str,filename[2]);
}

void (*recieve_data[NUM_OF_DATA_TYPES])(union data) = {
	&recieve_int, 
	&recieve_arr, 
	&recieve_struct
};

int main(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt(argc,argv,"Di:c:s:")) != -1) {
		switch (arg) {
			case 'D':
				if(fork()) return 0;
				fclose(stdout);
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
				return -1;
		};
	};

	key_t msgkey = QUEUE_KEY;
	if ((msgid = msgget(msgkey, IPC_CREAT | 0666)) < 0) {
		printf("Error while creating message queue!\n");
		return errno;
	}

	struct message msg_buf;
	union data data_buf;

	while (1) {
		if(msgrcv(msgid, &msg_buf, sizeof(union data), 0, MSG_NOERROR) == -1
						&& errno == EIDRM) break;
		memcpy(&data_buf, msg_buf.msg, sizeof(union data));
		if(msg_buf.mtype == NUM_OF_DATA_TYPES + 1) break;
		recieve_data[msg_buf.mtype - 1](data_buf);
	}

	msgctl(msgid,IPC_RMID,NULL);
	printf("Program execution ended.\n");

	return 0;
}
