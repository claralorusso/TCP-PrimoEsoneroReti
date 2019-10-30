/*
 ============================================================================
 Name        : server.c
 Author      : Clara Lorusso & Francesco Didio
 Version     : 1.0
 Copyright   : Your copyright notice
 Description : Socket Lato Server
 ============================================================================
 */

#if defined WIN32
#include <winsock.h>
#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define closesocket close
#endif
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "function.h"
#define PROTOPORT 27015
#define QLEN 6 // Numero massimo di client
#define CONN "connessione avvenuta"

struct msgStruct{
	char op;
	int num1;
	int num2;
	float result;
	char* error;
}msg;


void errorhandler(char *errorMessage) {
	printf ("%s", errorMessage);
}

void clearwinsock() {
#if defined WIN32
	WSACleanup();
	system("pause");
#endif
}

int main(int argc, char *argv[]) {
	int port;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	else
		port = PROTOPORT;
	if (port < 0) {
		printf("bad port number %s \n", argv[1]);
		return 0;
	}
#if defined WIN32
	// Inizializzazione winsock
	WSADATA wsa_data;
	int result = WSAStartup(MAKEWORD(2,2), &wsa_data);
	if (result != 0) {
		printf("Error at WSAStartup()\n");
		return 0;
	}
#endif
	// Creazione della socket
	int my_socket;
	my_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (my_socket < 0) {
		errorhandler("socket creation failed.\n");
		clearwinsock();
		return -1;
	}
	// Costruzione indirizzo
	struct sockaddr_in sad;
	memset(&sad, 0, sizeof(sad)); // ensures that extra bytes contain 0
	sad.sin_family = AF_INET;
	sad.sin_addr.s_addr = inet_addr("127.0.0.1");
	sad.sin_port = htons(port);
	if (bind(my_socket, (struct sockaddr*) &sad, sizeof(sad)) < 0) {
		errorhandler("bind() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	// socket in ascolto
	if (listen (my_socket, QLEN) < 0) {
		errorhandler("listen() failed.\n");
		closesocket(my_socket);
		clearwinsock();
		return -1;
	}
	// Accettazione di una nuova connessione
	struct sockaddr_in cad;
	int client_socket;
	int client_len;
	printf("In attesa di connessione di un Client...");
	while (1) {
		client_len = sizeof(cad);
		if ((client_socket = accept(my_socket, (struct sockaddr*)&cad, &client_len)) < 0) {
			errorhandler("accept() failed.\n");

			// Chiusura connessione
			closesocket(my_socket);
			clearwinsock();
			return 0;
		}
		printf("\nConnection established with %s:%d\n", inet_ntoa(cad.sin_addr), ntohs(cad.sin_port));
		int vsend = send(client_socket, CONN, sizeof(CONN), 0);
		if (vsend != sizeof(CONN)) {
			errorhandler("send() sent a different number of bytes than expected\n");
		}
		//  Ricezione dei dati
		int bytes_rcvd;
		int total_bytes_rcvd = 0;
		printf("\nMessaggio ricevuto: ");
		while (total_bytes_rcvd < sizeof(msg)) {
			if ((bytes_rcvd = recv(client_socket, &msg, sizeof(msg), 0)) <= 0) {
				errorhandler("\nrecv() failed or connection closed prematurely");
				break;
			}
			total_bytes_rcvd += bytes_rcvd;
			printf("%c %d %d", msg.op, msg.num1, msg.num2);

			// Invio dati
			switch(msg.op){
			case 'A':
			case 'a':
				msg.result=add(msg.num1, msg.num2);
				break;

			case 'M':
			case 'm':
				msg.result=mult(msg.num1, msg.num2);
				break;

			case 'S':
			case 's':
				msg.result=sub(msg.num1, msg.num2);
				break;

			case 'D':
			case 'd':
				msg.result=divi(msg.num1, msg.num2);
				break;

			default:
				msg.result=0;
				msg.error="TERMINE PROCESSO CLIENT";
				break;
			}

			int vsend2 = send(client_socket, &msg, sizeof(msg), 0);
			if (vsend2 != sizeof(msg)) {
				errorhandler("send() sent a different number of bytes than expected\n");
			}
		}
	}
	closesocket(client_socket);
	clearwinsock();
	return -1;
}
