#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "data_types.c"

#define INT_STR_SIZE 13

bool g_daemon_mode = false;
bool g_exit_program = false;
int g_msgid;

void write_data_to_file(char* data, char* filename) {
	FILE *fp = fopen(filename,"a+");
	struct msqid_ds qstatus;
	if(msgctl(g_msgid,IPC_STAT,&qstatus)<0){
		printf("Msgctl error!\n");
		exit(-1);
	}
	char s[30];
	strftime(s, 30, "%d.%m.%Y %H:%M:%S", localtime(&(qstatus.msg_rtime)));
	fprintf(fp,"%s : %s\n",s,data);
	fclose(fp);
}

void handle_int(union data data_buf) {
	char buffer[INT_STR_SIZE];
	printf("Recieved integer: %d\n", data_buf.num);
	snprintf(buffer,INT_STR_SIZE,"%d",data_buf.num);
	write_data_to_file(buffer,g_filename[0]);
}

void handle_arr(union data data_buf) {
	printf("Recieved char[5]: %s\n", data_buf.arr);
	write_data_to_file(data_buf.arr,g_filename[1]);
}

void handle_struct(union data data_buf) {
	char buffer[INT_STR_SIZE];
	char str[INT_STR_SIZE*3 + 6] = "";
	printf("Recieved struct: %d , %d , %d\n", data_buf.num3.a, 
				data_buf.num3.b, data_buf.num3.c);
	snprintf(str, sizeof(str),": %d , %d , %d", data_buf.num3.a,
				data_buf.num3.b, data_buf.num3.c);
	write_data_to_file(str,g_filename[2]);
}

void handle_exit_flag(union data data_buf) {
	g_exit_program = true;
}

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt(argc,argv,"Di:c:s:")) != -1) {
		switch (arg) {
			case 'D':
				g_daemon_mode = true;
				break;
			case 'i':
				g_filename[0] = optarg;
				break;
			case 'c':
				g_filename[1] = optarg;
				break;
			case 's':
				g_filename[2] = optarg;
				break;
			case '?':
				printf("Invalid argument!\n");
				return -1;
		};
	};
	return 0;
}

int apply_daemon_mode() {
	pid_t pid, sid;
	pid = fork();
	if(pid < 0) {
		printf("Error while forking!\n");
		exit(-1);
	}
	if(pid > 0) exit(0);
	umask(0);
	if((sid = setsid()) < 0) {
		printf("Error while creating SID!");
		exit(-1);
	}
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
}

int create_message_queue() {
	if ((g_msgid = msgget(QUEUE_KEY, IPC_CREAT | 0666)) < 0) {
		printf("Error while creating message queue!: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void (*handle_data[NUM_OF_DATA_TYPES + 1])(union data) = {
	&handle_int, 
	&handle_arr, 
	&handle_struct,
	&handle_exit_flag
};

int main(int argc, char **argv) {
	if(parse_args(argc,argv)) return -1;
	if(g_daemon_mode) apply_daemon_mode();
	if(create_message_queue()) return -1; 

	struct message msg_buf;
	union data data_buf;

	while (!g_exit_program) { //message recieving cycle
		if(msgrcv(g_msgid, &msg_buf, sizeof(union data), 0, MSG_NOERROR) == -1) {
			printf("Error while recieving a message!: %s\n", strerror(errno));
			return -1;
		}
		memcpy(&data_buf, msg_buf.msg, sizeof(union data));
		handle_data[msg_buf.mtype - 1](data_buf);
	}

	msgctl(g_msgid,IPC_RMID,NULL); //remove message queue
	printf("Program execution ended.\n");

	return 0;
}
