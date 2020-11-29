#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080
#define GROUP_NUMBERS 50
#define CLIENT_NUMBERS 50
#define DATA_LENGHT 1024
pthread_t thread[50];

struct client{

	int port;
	int id;
	int socket;
	struct sockaddr_in address;
	char name[50];
	int last_group_index;
	int groups[GROUP_NUMBERS];
};
struct client clients[CLIENT_NUMBERS];

void* client_threading(void * ClientInfo);

int main(int argc, char const *argv[]) {

	// creates socket file descriptor
	int server_fd;
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htons(INADDR_ANY);
	address.sin_port = htons(atoi(argv[1])); // host to network
	const int addrlen = sizeof(address);

	// binding
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// listening on server socket with backlog size CLIENT_NUMBERS.
	if (listen(server_fd, CLIENT_NUMBERS) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Server is listening on %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));


	// making a thread for each client
	int clientID = 0;
	while(1){

		clients[clientID].id 	  = clientID;
		clients[clientID].socket  = accept(server_fd, (struct sockaddr *)&clients[clientID].address, (socklen_t*)&addrlen);
		// printf("Welcome client %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		pthread_create(&thread[clientID], NULL, client_threading, (void*) &clients[clientID]);

		clientID++;
	}
	return 0;
}

void* client_threading(void * ClientInfo){

	struct client * clientInfo = (struct client*) ClientInfo;
	int id = clientInfo->id;
	int socket = clientInfo->socket;

	clients[id].last_group_index = 0;

	char client_name[DATA_LENGHT];
	int name_read = recv(socket, client_name, DATA_LENGHT, 0);
	client_name[name_read] = '\0';
	// clients[id].name = client_name;
	strcpy(clients[id].name, client_name);
	char final_message[DATA_LENGHT];
	printf("%s has connected to server with id: %d\n", clients[id].name,id);

	
	while(1){
		// reads a buffer with maximum size 1024 (DATA_LENGHT) from socket.
		char buffer[DATA_LENGHT];
		int valread = recv(socket, buffer, DATA_LENGHT, 0);
		buffer[valread] = '\0';
		if (valread < 0) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		// printf("(s = %d) %s\n", valread, buffer);

		if (!strcmp(buffer, "quit")){

			printf("%s has left...\n", clients[id].name);
			pthread_cancel(thread[id]);
			shutdown(clients[id].socket,2);
		}
		else if (!strcmp(buffer, "join")){

			
			valread = recv(socket, buffer,DATA_LENGHT, 0);
			buffer[valread] = '\0';
			int exist = 0;
			for (int i = 0; i < clients[id].last_group_index+1; i++){
				if(clients[id].groups[i] == atoi(buffer))
					exist = 1;
			}
			if(exist == 0){
				clients[id].groups[clients[id].last_group_index+1] = atoi(buffer);
				clients[id].last_group_index++;
			}
			
		}
		else if (!strcmp(buffer, "send")){

			// read group id
			valread = recv(socket, buffer,DATA_LENGHT, 0);
			buffer[valread] = '\0';
			int group = atoi(buffer);

			// read message
			valread = recv(socket, buffer,DATA_LENGHT, 0);
			buffer[valread] = '\0';

			// concat message with the name of sender
			strcat(final_message, clients[id].name);
			strcat(final_message, " : ");
			strcat(final_message, buffer);
			strcat(final_message, "\n");

			// check if the sender is a member of the group
			int in_group = 0;
			for (int k = 0; k < clients[id].last_group_index+1; k++){
				if (clients[id].groups[k] == group){ // it means the client is a member of the group

					in_group = 1;
					// now search in all clients and send the message for those with the same group_id in message
					for (int i = 0; i < CLIENT_NUMBERS; i++){
						for (int j = 0; j < clients[i].last_group_index+1; j++){
							if (clients[i].groups[j] == group)
		 						send(clients[i].socket, final_message, DATA_LENGHT, 0);
						}
					}
				}
			}
			if (in_group == 0){
				printf("%s you're not a member in group with id: %d\n", clients[id].name, group);
				send(socket, "you're not a member in this group", DATA_LENGHT, 0);
			}
		}
		else if (!strcmp(buffer, "leave")){

			
			valread = recv(socket, buffer,DATA_LENGHT, 0);
			buffer[valread] = '\0';
			int exist = 0;
			int deleted_group_index;
			for (int i = 0; i < clients[id].last_group_index+1; i++)
			{
				if(clients[id].groups[i] == atoi(buffer)){
					exist = 1;
					deleted_group_index = i;
					printf("this group is deleted:%d\n", clients[id].groups[i]);
				}
			}
			if(exist == 1){
				for(int i = deleted_group_index; i<clients[id].last_group_index+1; i++)
					clients[id].groups[i] = clients[id].groups[i+1];
				clients[id].last_group_index--;
			}
			else{
				printf("no such a group with id: %d !!\n", atoi(buffer));
				send(socket, "no such a group", DATA_LENGHT, 0);
			}

		}
	}
}


