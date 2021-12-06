#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
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

void clear_screen() {
	printf("\e[1;1H\e[2J");
}

void clear_stdin() {
	char c;
	while((c = getchar()) != '\n' && c != EOF);
}

union data scanf_int() {
	union data value;
	printf("Enter integer value\n");
	while(scanf("%d", &(value.num)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	return value;
}

union data scanf_arr() {
	union data value;
	printf("Enter string under 5 characters\n");
	while((scanf("%s", value.arr) != 1) || strlen(value.arr) > 5) {
		printf("Invalid input. Enter string under 5 characters\n");
		clear_stdin();
	}
	return value;
}

union data scanf_struct() {
	union data value;
	printf("Enter integer value\n");
	while(scanf("%d", &(value.num3.a)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	printf("Enter integer value\n");
	while(scanf("%d", &(value.num3.b)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	printf("Enter integer value\n");
	while(scanf("%d", &(value.num3.c)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	return value;
}

char *data_type[NUM_OF_DATA_TYPES] = {"Integer", "Char array", "Structure with 3 integers"};
union data (*scanf_data[NUM_OF_DATA_TYPES])() = {&scanf_int, &scanf_arr, &scanf_struct};

int main(int argc, char **argv) {

	char arg = 0;
	while ((arg = getopt(argc,argv,"f:")) != -1) {
		switch (arg) {
			case 'f':
				printf("%s\n",optarg);
				break;
			case '?':
				printf("Invalid argument!\n");
				return 1;
		};
	};

	clear_screen();

	int msgid;
	key_t msgkey = QUEUE_KEY;
	msgid = msgget(msgkey, IPC_CREAT | 0666/*| IPC_EXCL*/);
	printf("Message queue id: %d\n",msgid);

	bool exit_program = false;
	while (!exit_program) {

		printf("Choose what data type to send:\n");
		for (int i = 0; i < NUM_OF_DATA_TYPES; i++) {
			printf("%d: %s\n", i+1, data_type[i]);
		}
		printf("%d: Exit program\n", NUM_OF_DATA_TYPES + 1);	

		int choice = 0;
		while ((scanf("%d", &choice) != 1) || ((choice < 1) 
					|| (choice > NUM_OF_DATA_TYPES + 1))) {
			printf("Invalid input.\nEnter integer value in range  1 - %d\n", 
					NUM_OF_DATA_TYPES);
			clear_stdin();
	}

		if(choice == NUM_OF_DATA_TYPES + 1) break;

		struct message buf;
		buf.mtype = choice;
		union data dat = scanf_data[choice - 1]();
		memcpy(buf.msg, &dat, sizeof(union data));
		if (msgsnd(msgid, &buf, sizeof(struct message), IPC_NOWAIT) < 0) exit(1);
		else printf("Message: \"%d\" Sent\n", *(int*)buf.msg);

		clear_screen();
	}

	printf("Program execution ended.\n");

	return 0;
}
