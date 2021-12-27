#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include "tcp_settings.h"

int g_socket_id;
int g_thread_mode = 0, g_process_mode = 0;
bool g_shut_server = false;

const struct option long_options[] = {
	{ "pthreads", no_argument, &g_thread_mode, 1 },
	{ "process", no_argument, &g_process_mode, 1 },
	{ NULL, 0, NULL, 0}
};

void handle_sigterm() {}

int create_process(int (*fun_ptr)(int, int), int connection_id, int client_num) {
	int pid = fork();
	switch (pid) {
		case -1: {
				 printf("An error occured with creating process\n");
				 return -1;
			 } break;
		case 0: {
				switch(fun_ptr(connection_id, client_num)) {
					case 1: 
						kill(getppid(), SIGTERM);	
					case 0:
						exit(0);
						break;
					default: 
						exit(-1);
				}
			} break;
		default: return pid;
	}
}

int handle_connection(int connection_id, int client_num) {
	char command[BUFF_SIZE];

	struct sigaction sa = { 0 }; // SIGTERM handler initialization
	sa.sa_handler = &handle_sigterm;
	sigaction(SIGTERM, &sa, NULL);

	while(1) {
		bzero(command, BUFF_SIZE);
		if(recv(connection_id, command, BUFF_SIZE, 0) == -1 && errno == EINTR) {
			break;
		}
		printf("Command from client %d: %s\n", client_num, command);

		if (strncmp("exit", command, 4) == 0) {
			printf("Client %d closed connection!\n", client_num);
			break;
		}
		if (strncmp("shut", command, 4) == 0) {
			printf("Client %d is closing server!\n", client_num);
			return 1;
		}

		FILE *fp;
		char buff[BUFF_SIZE];
		if((fp = popen(command, "r")) == NULL) {
			printf("Failed to run command!\n" );
			strncpy(buff, "Failed to run command!\n", BUFF_SIZE);
			send(connection_id, buff, BUFF_SIZE, 0);
			continue;
		}
		while (fgets(buff, BUFF_SIZE, fp) != NULL) { //write command output into string
			send(connection_id, buff, BUFF_SIZE, 0);
			bzero(buff,BUFF_SIZE);
		}
		bzero(command, BUFF_SIZE);
		pclose(fp);
		send(connection_id, "q", 1, 0); //send quit message to end reading in client
	}

	return 0;
}

void* handle_connection_thread(void* arg) {
	int* connection_id = (int*)arg;
	int* client_num = (int*)(arg + sizeof(int));
	if(handle_connection(*connection_id, *client_num) == 1) {
		printf("Program execution ended.\n"); //shut server after recieving "shut" command
		close(g_socket_id);
		exit(0);
	}
}

int parse_args(int argc, char **argv) {
	char arg = 0;
	while ((arg = getopt_long_only(argc, argv, "", long_options, NULL)) != -1) {
		if(arg == '?') return -1;
	};
	return 0;
}

int main(int argc, char **argv) {
	struct sockaddr_in server_info;

	struct sigaction sa = { 0 }; // SIGTERM handler initialization
	sa.sa_handler = &handle_sigterm;
	sigaction(SIGTERM, &sa, NULL);

	if(parse_args(argc,argv)) return -1;
	if((g_thread_mode  == 1) && (g_process_mode == 1)) {
		printf("Error! Choose only one mode.\n");
		return -1;
	}

	if(g_thread_mode) printf("Initializing server... (multithreading mode)\n");
	else printf("Initializing server... (multiprocessing mode)\n");
	
	if((g_socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error while creating socket!: %s\n", strerror(errno));
		return -1;
	}

	int option = 1;
	if(setsockopt(g_socket_id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
		printf("Error while setting socket options!: %s\n", strerror(errno));
		return -1;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(TCP_PORT);

	if (bind(g_socket_id, (struct sockaddr*)&server_info, sizeof(server_info))) {
		printf("Error while binding socket!: %s\n", strerror(errno));
		close(g_socket_id);
		return -1;
	}
    
	if (listen(g_socket_id, 5)) {
		printf("Error while listening!: %s\n", strerror(errno));
		close(g_socket_id);
		return -1;
	}

	printf("Server initialization completed! Listening...\n");

	int client_num = 1;
	pthread_t threads[255]; 
	pid_t clients[255];

	while(1) {
		struct sockaddr_in client_info;
		int connection_id;
		int len = sizeof(client_info);

		connection_id = accept(g_socket_id, (struct sockaddr*)&client_info, &len); //accepting client connection
		if(connection_id == -1) {
			if(errno == EINTR) {
				for(int i = 0; i < client_num - 1; i++) {
					kill(clients[i], SIGTERM);
				}
				break;
			}
			printf("Server accept error!: %s\n", strerror(errno));
			close(g_socket_id);
			return -1;
		}

		if(!g_thread_mode) { //creating new process/thread
			create_process(&handle_connection, connection_id, client_num);
		}
		else {
			int args[2] = {connection_id, client_num};
			clients[client_num - 1] = pthread_create(&threads[client_num - 1], NULL, handle_connection_thread, &args);
		}

		printf("Client %d connected to the server!\n", client_num);
		client_num++;
	}
	close(g_socket_id);
	printf("Program execution ended\n");
	return 0;
}
