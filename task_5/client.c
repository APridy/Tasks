#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define QUEUE_KEY 252525
#define NUM_OF_DATA_TYPES 3
#define DELIMITER ":,\n"

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
	printf("\e[1;1H\e[2J"); //ANSI escape codes to clear screen/set carriage to 1,1
}

void clear_stdin() {
	char c;
	while ((c = getchar()) != '\n' && c != EOF);
}

union data scanf_int() {
	union data value;
	printf("Enter integer value\n");
	while (scanf("%d", &(value.num)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	return value;
}

union data scanf_arr() {
	union data value;
	printf("Enter string under 5 characters\n");
	while ((scanf("%s", value.arr) != 1) || strlen(value.arr) > 5) {
		printf("Invalid input. Enter string under 5 characters\n");
		clear_stdin();
	}
	return value;
}

union data scanf_struct() {
	union data value;
	printf("Enter integer value\n");
	while (scanf("%d", &(value.num3.a)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	printf("Enter integer value\n");
	while (scanf("%d", &(value.num3.b)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	printf("Enter integer value\n");
	while (scanf("%d", &(value.num3.c)) != 1) {
		printf("Invalid input. Enter integer value\n");
		clear_stdin();
	}
	return value;
}

union data parse_int(char* str) {
	union data value;
	value.num = atoi(str);
	return value;
}

union data parse_arr(char* str) {
	union data value;
	strcpy(value.arr,str);
	return value;
}

union data parse_struct(char* str) {
	union data value;
	value.num3.a = atoi(str);
	str = strtok(NULL,DELIMITER);
	value.num3.b = atoi(str);
	str = strtok(NULL,DELIMITER);
	value.num3.c = atoi(str);
	return value;
}

void shut_server(int msgid) {
	struct message msg_buf;
	union data data_buf;

	msg_buf.mtype = NUM_OF_DATA_TYPES + 1;
	if (msgsnd(msgid, &msg_buf, sizeof(struct message), IPC_NOWAIT) < 0) {
		printf("Msgsnd error!\n");
		exit(errno);
	}

}

char *data_type[NUM_OF_DATA_TYPES] = {
	"Integer",
	"Char array",
	"Structure with 3 integers"
};

union data (*scanf_data[NUM_OF_DATA_TYPES])() = {
	&scanf_int,
	&scanf_arr,
	&scanf_struct
};

union data (*parse_data[NUM_OF_DATA_TYPES])(char* str) = {&parse_int,&parse_arr, &parse_struct};

int parse_file(char* filename, int msgid) {
	char *line = NULL;
	size_t len = 0;

	FILE *fp;
	if ((fp = fopen(filename,"r")) == NULL) {
		printf("No such file!\n");
		return errno;
	}

	printf("Parsing ""%s""...\n",filename);
	while (getline(&line, &len, fp) != -1) {
		char *type;
		type = strtok(line, DELIMITER);
		for (int i = 0; i <= NUM_OF_DATA_TYPES; i++) {
			if (i == NUM_OF_DATA_TYPES) {
				printf("Invalid type! Line:\n%s\n",line);
				fclose(fp);
				free(line);
				errno = 1;
				return errno;
			}
			if (strcmp(type,data_type[i]) == 0) {
				struct message msg_buf;
				msg_buf.mtype = i + 1;
				union data data_buf = parse_data[i](strtok(NULL,DELIMITER));
				memcpy(msg_buf.msg, &data_buf, sizeof(union data));
				if (msgsnd(msgid, &msg_buf, sizeof(struct message), IPC_NOWAIT) < 0) {
					printf("Msgsnd error!\n");
					return errno;
				}
				break;
			}
		}
	}

	printf("Parsing completed succesfully!\n");
	fclose(fp);
	free(line);
	return 0;
}

int main(int argc, char **argv) {
	char arg = 0;
	char *filename = NULL;
	while ((arg = getopt(argc,argv,"f:")) != -1) {
		switch (arg) {
			case 'f':
				filename = optarg;
				break;
			case '?':
				printf("Invalid argument!\n");
				return 1;
		};
	};

	int msgid;
	key_t msgkey = QUEUE_KEY;
	if ((msgid = msgget(msgkey, 0666)) < 0) {
		printf("Error while connecting to message queue!\n");
		return errno;
	}

	clear_screen();
	printf("Message queue id: %d\n",msgid);

	if ((filename != NULL) && parse_file(filename, msgid)) {
		return errno;
	}

	while (1) {
		printf("Choose what data type to send:\n");
		for (int i = 0; i < NUM_OF_DATA_TYPES; i++) {
			printf("%d: %s\n", i+1, data_type[i]);
		}
		printf("%d: Exit program\n", NUM_OF_DATA_TYPES + 1);
		printf("%d: Exit program & shut server\n", NUM_OF_DATA_TYPES + 2);

		int choice = 0;
		while ((scanf("%d", &choice) != 1) || ((choice < 1) || (choice > NUM_OF_DATA_TYPES + 2))) {
			printf("Invalid input.\nEnter integer value in range  1 - %d\n", NUM_OF_DATA_TYPES);
			clear_stdin();
		}

		struct message msg_buf;
		msg_buf.mtype = choice;
		
		if (choice == NUM_OF_DATA_TYPES + 1) break;
		if (choice == NUM_OF_DATA_TYPES + 2) {
			shut_server(msgid);
			break;
		}

		union data data_buf = scanf_data[choice - 1]();
		memcpy(msg_buf.msg, &data_buf, sizeof(union data));
		if (msgsnd(msgid, &msg_buf, sizeof(struct message), IPC_NOWAIT) < 0) { 
			printf("Msgsnd error!\n");
			return errno;
		}

		clear_screen();
	}

	printf("Program execution ended.\n");

	return 0;
}
