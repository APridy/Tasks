#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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

struct message {
	long mtype;
	uint8_t msg[sizeof(union data)];
};

void clear_stdin() {
        char c;
        while((c = getchar()) != '\n' && c != EOF);
}

union data get_integer() {
	union data value;
	while(scanf("%d", &(value.num)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	return value;
}


//FILE *fp[NUM_OF_DATA_TYPES];
char *data_type[NUM_OF_DATA_TYPES] = {"Integer", "Char array", "Structure with 3 integers"};
union data (*get_data[NUM_OF_DATA_TYPES])() = {&get_integer, NULL, NULL};

int main(int argc, char **argv) {

	char rez = 0;
	while ((rez = getopt(argc,argv,"f:")) != -1) {
		switch (rez) {
                        case 'f':
				printf("%s\n",optarg);
                                break;
                        case '?':
                                printf("Invalid argument!\n");
                                return 1;
                };
        };

	int msgid;
	key_t msgkey = QUEUE_KEY;
	msgid = msgget(msgkey, IPC_CREAT | 0666/*| IPC_EXCL*/);
	printf("Message queue id: %d\n",msgid);

	printf("Choose what data type to send:\n");
	for (int i = 0; i < NUM_OF_DATA_TYPES; i++) {
		printf("%d: %s\n", i+1, data_type[i]);
	} 
	
	int choice = 0;
	while((scanf("%d", &choice) != 1) || ((choice < 1) || (choice > NUM_OF_DATA_TYPES))) {
                printf("Invalid input.\nEnter integer value in range  1 - %d\n", NUM_OF_DATA_TYPES);
                clear_stdin();
        }

	struct message sbuf;
	size_t buf_length;
	sbuf.mtype = 1;
	union data dat = get_data[0]();
	printf("%d\n",dat.num);
	memcpy(sbuf.msg, &dat, sizeof(union data));
	buf_length = strlen(sbuf.msg) + 1 ;
	if (msgsnd(msgid, &sbuf, buf_length, IPC_NOWAIT) < 0) exit(1);
	else printf("Message: \"%d\" Sent\n", *(int*)sbuf.msg);
	
	return 0;
}
