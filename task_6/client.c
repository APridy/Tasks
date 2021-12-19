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
#define SERVER_IP "127.0.0.1"

int main() {
	int socket_id, connection_id;
	struct sockaddr_in server_info, client_info;

	if((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error while creating socket!: %s\n", strerror(errno));
		return -1;
	}
	//bzero(&server_info, sizeof(server_info));

	server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_info.sin_port = htons(TCP_PORT);
	//close(socket_id);
	if (connect(socket_id, (struct sockaddr*)&server_info, sizeof(server_info))) {
		printf("Failed to connect to server!: %s\n", strerror(errno));
		close(socket_id);
		return -1;
	}
	else {
		printf("Connected to the server!\n");
	}

	char buff[80];
	int n;
	while(1) {
		//bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n') {}
		write(socket_id, buff, sizeof(buff));
		//bzero(buff, sizeof(buff));
		read(socket_id, buff, sizeof(buff));
		printf("From Server : %s", buff);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}

	close(socket_id);
	printf("Program execution ended.\n");
	return 0;
}
