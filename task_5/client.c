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
#include "data_types.c"

#define DELIMITER ":,\n"

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

char *g_parse_filename = NULL;
int g_msgid;

union data (*scanf_data[NUM_OF_DATA_TYPES])() = {
	&scanf_int,
	&scanf_arr,
	&scanf_struct
};

union data (*parse_data[NUM_OF_DATA_TYPES])(char* str) = {
	&parse_int,
	&parse_arr, 
	&parse_struct
};

int parse_file(char* filename, int msgid) {
	char *line,*type;
	size_t len = 0;
	struct message msg_buf;
	union data data_buf;

	FILE *fp;
	if ((fp = fopen(filename,"r")) == NULL) {
		printf("No such file!\n");
		return errno;
	}

	printf("Parsing ""%s""...\n",filename);
	while (getline(&line, &len, fp) != -1) {
		type = strtok(line, DELIMITER);
		for (int i = 0; i <= NUM_OF_DATA_TYPES; i++) {
			if (i == NUM_OF_DATA_TYPES) {
				printf("Invalid type! Line:\n%s\n",line);
				errno = 1;
				goto end_parsing;
			}
			if (strcmp(type,g_data_type[i]) == 0) {
				msg_buf.mtype = i + 1;
				data_buf = parse_data[i](strtok(NULL,DELIMITER));
				memcpy(msg_buf.msg, &data_buf, sizeof(union data));
				if (msgsnd(msgid, &msg_buf, 
					sizeof(struct message), IPC_NOWAIT) < 0) {
					printf("Msgsnd error!\n");
					errno = 1;
					goto end_parsing;
				}
				break;
			}
		}
	}

	printf("Parsing completed succesfully!\n");
	errno = 0;
	end_parsing:
	fclose(fp);
	free(line);
	return errno;
}

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt(argc,argv,"f:")) != -1) {
		switch (arg) {
			case 'f':
				g_parse_filename = optarg;
				break;
			case '?':
				printf("Invalid argument!\n");
				errno = 1;
				return errno;
		};
	};
	return 0;
}

int connect_to_message_queue() {
	if ((g_msgid = msgget(QUEUE_KEY, 0666)) < 0) {
		printf("Error while connecting to message queue!\n");
		return errno;
	}
	return 0;
}

void print_menu() {
	printf("Choose what data type to send:\n");
	for (int i = 0; i < NUM_OF_DATA_TYPES; i++) {
		printf("%d: %s\n", i+1, g_data_type[i]);
	}
	printf("%d: Exit program\n", NUM_OF_DATA_TYPES + 1);
	printf("%d: Exit program & shut server\n", NUM_OF_DATA_TYPES + 2);
}

int scanf_choice() {
	int choice;
	while ((scanf("%d", &choice) != 1) || ((choice < 1) || (choice > NUM_OF_DATA_TYPES + 2))) {
		printf("Invalid input.\nEnter integer value in range  1 - %d\n", NUM_OF_DATA_TYPES);
		clear_stdin();
	}
	return choice;
}

int main(int argc, char **argv) {
	if(parse_args(argc,argv)) return errno;
	if(connect_to_message_queue()) return errno;

	if ((g_filename != NULL) && parse_file(g_parse_filename, g_msgid)) {
		return errno;
	}

	struct message msg_buf;
	union data data_buf;
	bool exit_program = false;
	while (!exit_program) {
		clear_screen();
		print_menu();
		msg_buf.mtype = scanf_choice();
		switch(msg_buf.mtype) {
			case NUM_OF_DATA_TYPES + 2: 
				shut_server(g_msgid);
			case NUM_OF_DATA_TYPES + 1: {
				exit_program = true;
			} break;
			default: {
				data_buf = scanf_data[msg_buf.mtype - 1]();
				memcpy(msg_buf.msg, &data_buf, sizeof(union data));
				if (msgsnd(g_msgid, &msg_buf, 
					sizeof(struct message), IPC_NOWAIT) < 0) {
					printf("Msgsnd error!\n");
					return errno;
				}
			} break;
		}
	}

	printf("Program execution ended.\n");
	return 0;
}
