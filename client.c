#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8080
#define DATA_LENGHT 1024

void* threading(void * client_socket);

int main(int argc, char const *argv[]) {

int sock = 0, valread;
struct sockaddr_in serv_addr;
char buffer[DATA_LENGHT] = {0};

if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	printf("\n Socket creation error \n");
	return -1;
}

// sets all memory cells to zero
memset(&serv_addr, '0', sizeof(serv_addr));

// sets port and address family
serv_addr.sin_family = AF_INET;
serv_addr.sin_port = htons(atoi(argv[2]));

// Convert IPv4 and IPv6 addresses from text to binary form
if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
{
	printf("\nInvalid address/ Address not supported \n");
	return -1;
}

// connects to the server
if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
{
	printf("\nConnection Failed \n");
	return -1;
}

// create thread that listens to server
pthread_t thread;
pthread_create(&thread, NULL, threading, (void *)&sock);

send(sock, argv[3], 50, 0);

while(1){

	char statement[DATA_LENGHT];

	scanf("%s", statement);
	send(sock, statement, DATA_LENGHT, 0);

	if (!strcmp(statement, "join")){

	    // take group id
	    scanf("%s", statement);
	    send(sock, statement, DATA_LENGHT, 0);
	}
	else if (!strcmp(statement, "send")){

	    // take group id
	    scanf("%s", statement);
	    send(sock, statement, DATA_LENGHT, 0);

	    // take message
	    scanf("%[^\n]%*c", statement);
	    send(sock, statement, DATA_LENGHT, 0);
	}
	else if (!strcmp(statement, "leave")){

    	// take group id
    	scanf("%s", statement);
	    send(sock, statement, DATA_LENGHT, 0);
	}
	else if (!strcmp(statement, "quit")){

	    printf("Ending connection...\n");
	    pthread_cancel(thread);
	    shutdown(sock,2);
	    return 0;
	}
}

return 0;
	
}

void* threading(void * client_socket){

	int clientSocket = *((int *)client_socket);
	while(1){
		char buffer[1024] = {0};
		int valread = read(clientSocket, buffer, 1024);
		buffer[valread] = '\0';

		if (valread < 0) {
			perror("read");
		}
		printf("%s\n", buffer);
	}
}