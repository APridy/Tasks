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

#define TCP_PORT 5665
#define BUFF_SIZE 80
#define SERVER_IP "127.0.0.1"

int main() {
	int socket_id, connection_id;
	struct sockaddr_in server_info, client_info;

	//close(socket_id);
	if((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error while creating socket!: %s\n", strerror(errno));
		return -1;
	}

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_info.sin_port = htons(TCP_PORT);
	if (connect(socket_id, (struct sockaddr*)&server_info, sizeof(server_info))) {
		printf("Failed to connect to server!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}
	else {
		printf("Connected to the server!\n");
	}
	printf("Enter \"exit\" to exit program\n");
	char buff[BUFF_SIZE];
	while(1) {
		bzero(buff, BUFF_SIZE);
		printf("Enter the string : ");
		scanf("%[^\n]%*c",buff);
		write(socket_id, buff, BUFF_SIZE);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
		bzero(buff, sizeof(buff));
		int n = 0;
		while(1) {
			//read(socket_id, buff, BUFF_SIZE);
			//if(strlen(buff) == 0) break;
			n = read(socket_id, buff, BUFF_SIZE);
			//if(n == 0) break;
			//buff[BUFF_SIZE] = '\0';
			printf("%.*s", BUFF_SIZE, buff);
			//printf("\n\n%d\n\n",n);
			bzero(buff, BUFF_SIZE);
		}
		//printf("%s\n", buff);
	}

	close(socket_id);
	printf("Program execution ended.\n");
	return 0;
}
