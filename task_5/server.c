#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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


FILE *fp[NUM_OF_DATA_TYPES];
char *filename[NUM_OF_DATA_TYPES] = {"int.txt", "array.txt", "struct.txt"};

int main(int argc, char **argv) {
	
	char rez = 0;
	while ((rez = getopt(argc,argv,"Di:c:s:")) != -1) {
		switch (rez) {
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
	
	
	for (int i = 0; i < NUM_OF_DATA_TYPES; i++) {
		printf("%s\n",filename[i]);
		fp[i] = fopen(filename[i],"w");
	}
	
	int msgid;
	key_t msgkey = QUEUE_KEY;
	msgid = msgget(msgkey, IPC_CREAT | 0666/*| IPC_EXCL*/);
	printf("Message queue id: %d\n",msgid);
	
	struct msqid_ds qstatus;

	if(msgctl(msgid,IPC_STAT,&qstatus)<0){
		perror("msgctl failed");
		exit(1);
	}
	printf("Real user id of the queue creator: %d\n",qstatus.msg_perm.cuid);
	printf("Real group id of the queue creator: %d\n",qstatus.msg_perm.cgid);
	printf("Effective user id of the queue creator: %d\n",qstatus.msg_perm.uid);
	printf("Effective group id of the queue creator: %d\n",qstatus.msg_perm.gid);
 	printf("Permissions: %d\n",qstatus.msg_perm.mode);
	printf("Message queue id: %d\n",msgid);
	printf("%ld message(s) on queue\n",qstatus.msg_qnum);
	printf("Current number of bytes on queue %ld\n",qstatus.msg_cbytes);
	printf("Maximum number of bytes allowed on the queue%ld\n",qstatus.msg_qbytes);

	printf("\nLast message sent by process :%3d at %s \n",qstatus.msg_lspid,ctime(& (qstatus.msg_stime)));
	printf("Last message received by process :%3d at %s \n",qstatus.msg_lrpid,ctime(& (qstatus.msg_rtime)));
	
	struct message rbuf;
	union data dat = {.num = 0};
	int twelve = 33;
	while(1) {
		msgrcv(msgid, &rbuf, sizeof(struct message), 1, IPC_NOWAIT);
		memcpy(&dat, &rbuf, sizeof(union data));
		//dat.num = rbuf.msg[0];
		printf("%d\n", dat.num);
		printf("%s\n",dat.arr);
		sleep(1);
	}
	
	return 0;
	
}
