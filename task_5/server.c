#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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
	key_t msgkey = 252525;
	msgid = msgget(msgkey, IPC_CREAT | 0666/*| IPC_EXCL*/);
	printf("Message queue id: %d\n",msgid);

	return 0;

}
