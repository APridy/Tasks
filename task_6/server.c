#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

#define TCP_PORT 5665
#define BUFF_SIZE 80

int g_thread_mode = 0, g_process_mode = 0;

const struct option long_options[] = {
	{ "pthreads", no_argument, &g_thread_mode, 1 },
	{ "process", no_argument, &g_process_mode, 1 },
	{ NULL, 0, NULL, 0}
};

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt_long_only(argc, argv, "", long_options, NULL)) != -1) {
		if(arg == '?') return -1;
	};
	return 0;
}

int main(int argc, char **argv) {
	int socket_id, connection_id;
	struct sockaddr_in server_info, client_info;

	if(parse_args(argc,argv)) return -1;
	printf("Parsing succesful!\n");
	//if(g_thread_mode) printf("Thread\n");
	//if(g_process_mode) printf("Process\n");

	if((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error while creating socket!: %s\n", strerror(errno));
		return -1;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(TCP_PORT);

	if (bind(socket_id, (struct sockaddr*)&server_info, sizeof(server_info))) {
		printf("Error while binding socket!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}
    
	if (listen(socket_id, 5)) {
		printf("Error while listening!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}

	int len = sizeof(client_info);
	connection_id = accept(socket_id, (struct sockaddr*)&client_info, &len);
	if (connection_id < 0) {
		printf("Server accept error!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}

	printf("Client connected to the server!\n");

	char buff[BUFF_SIZE];

	while(1) {
		bzero(buff, BUFF_SIZE);
		read(connection_id, buff, BUFF_SIZE);
		printf("From client: %s\n", buff);
		//write(connection_id, buff, sizeof(buff)); 
		if (strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}
		system(buff);
	}

	close(socket_id);
	printf("Program execution ended\n");
	return 0;
}
