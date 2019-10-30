/*
 ============================================================================
 Name        : client.c
 Author      : Clara Lorusso & Francesco Didio
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Socket Lato Client
 ============================================================================
 */
#if defined WIN32
#include <winsock.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PROTOPORT 27015 // Numero di porta di default
#define IP "127.0.0.1" //localhost

struct msgStruct{
	char op;
	int num1;
	int num2;
	float result;
	char* error;
}msg;

char conn[21];

void errorhandler(char *error_message) {
	printf("%s",error_message);
}
void clearwinsock() {
#if defined WIN32
	WSACleanup();
	system("pause");
#endif
}



int main(void) {
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2 ,2), &wsa_data);
	if (result != 0) {
		printf ("error at WSASturtup\n");
		return -1;
	}

	// Socket creation
	int c_socket;
	c_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (c_socket < 0) {
		errorhandler("socket creation failed.\n");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	// Build server address
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad));
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr(IP); // IP server "127.0.0.1"
	sad.sin_port = htons(PROTOPORT); // Server port 27015

	// Connection to server
	if (connect(c_socket, (struct sockaddr *)&sad, sizeof(sad))< 0)
	{
		errorhandler( "Failed to connect.\n" );
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}
	int msg_len = sizeof(conn);
	int bytes_rcvd;
	int total_bytes_rcvd = 0;
	printf("Received: "); // Setup to print the echoed string
	while (total_bytes_rcvd < msg_len) {
		if ((bytes_rcvd = recv(c_socket, conn, msg_len, 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		total_bytes_rcvd += bytes_rcvd; // Keep tally of total bytes
		printf ("%s", &conn);
	}
	// Sending data
	puts("\nInserisci comandi ");
	fflush(stdin);
	char* temp = malloc(8);
	gets(temp);
	char* tokenPtr = strtok(temp, " ");
	msg.op = *tokenPtr;
	tokenPtr = strtok(NULL, " ");
	msg.num1 = atoi(tokenPtr);
	tokenPtr = strtok(NULL, " ");
	msg.num2 =  atoi(tokenPtr);

	int msg_len2 = sizeof(msg);
	int vsend2 = send(c_socket, &msg, msg_len2, 0);
	if (vsend2 != msg_len2) {
		errorhandler("\nsend() sent a different number of bytes than expected");
		closesocket(c_socket);
		clearwinsock();
		return -1;
	}

	//  Receive data from the server
	int bytes_rcvd2;
	int total_bytes_rcvd2 = 0;
	printf("Received: "); // Setup to print the echoed string
	while (total_bytes_rcvd2 < msg_len2) {
		if ((bytes_rcvd2 = recv(c_socket, &msg, msg_len2, 0)) <= 0) {
			errorhandler("recv() failed or connection closed prematurely\n");
			closesocket(c_socket);
			clearwinsock();
			return -1;
		}
		total_bytes_rcvd2 += bytes_rcvd2; // Keep tally of total bytes
		if(msg.result == 0 && msg.error != NULL){
			printf("chiusura processo\n");
			closesocket(c_socket);
			clearwinsock();
		}
	}

	return 0;
}
