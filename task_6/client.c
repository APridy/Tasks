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
#include <arpa/inet.h>
#include "tcp_settings.h"

int main() {
	int socket_id;
	struct sockaddr_in server_info;

	if((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) { //create socket
		printf("Error while creating socket!: %s\n", strerror(errno));
		return -1;
	}

	int option = 1;
	if(setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1) {
		printf("Error while setting socket options!: %s\n", strerror(errno));
		return -1;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_info.sin_port = htons(TCP_PORT);

	if (connect(socket_id, (struct sockaddr*)&server_info, sizeof(server_info))) { //connect to server
		printf("Failed to connect to server!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}
	else {
		printf("Connected to the server!\n");
	}

	char buff[BUFF_SIZE];
	while(1) { //endless loop for writing commands
		printf("Enter \"exit\" to exit program\n");
		printf("Enter \"shut\" to shut server\n");
		printf("Enter the command : ");
		scanf("%[^\n]%*c",buff);

		if (send(socket_id, buff, BUFF_SIZE, 0) == -1) {
			printf("Error while writing to server!: %s\n", strerror(errno));
			return -1;
		}
		if (strncmp(buff, "exit", 4) == 0 || strncmp(buff, "shut", 4) == 0) {
			printf("Client Exit...\n");
			break;
		}

		bzero(buff, BUFF_SIZE);
		bool stop_reading = false;
		while(!stop_reading) { //recieve output from server
			switch(recv(socket_id, buff, BUFF_SIZE, 0)) {
				case 1: //special message to indicate transmission end 
					stop_reading = true; 
					break;
				case -1: 
					printf("Error while reading output!:%s\n", strerror(errno));
					return -1;
				default: //print output to console
					printf("%.*s", BUFF_SIZE, buff);
					bzero(buff, BUFF_SIZE);
					break;
			}
		}
		bzero(buff, BUFF_SIZE);
	}

	close(socket_id);
	printf("Program execution ended.\n");
	return 0;
}
