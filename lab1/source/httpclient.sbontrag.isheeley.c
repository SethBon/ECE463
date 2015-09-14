#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 512

int openClientFd(char *, int);


int main(int argc, char ** argv) {
	
	if(argc != 4) {
		printf("wrong number of input parameters\n");
		return -1;
	}

	int clientfd;
	int port;
	char * host;
	char buf[MAXLINE];
	char * filepath;
	char httpGet[100] = "GET ";
	int bytesRead;

	host = argv[1];
	port = atoi(argv[2]);
	filepath = argv[3];

	clientfd = openClientFd(host, port);

	strcat(httpGet, filepath);
	strcat(httpGet, " HTTP/1.0\r\n\r\n");
 
	send(clientfd, httpGet, strlen(httpGet), 0);
	
	do {
		bzero(buf, MAXLINE);
		
		bytesRead = recv(clientfd, buf, MAXLINE-1, 0);
		if (bytesRead > 0) {
			printf("%s", buf);
		}
	} while(bytesRead > 0);
	
	close(clientfd);
	exit(0);
}


int openClientFd(char * host, int port) {
	
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;
	
	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}
	/* Fill in the server's IP address and port */ 
	if ((hp = gethostbyname(host)) == NULL) {
		return -2;
	}

	bzero((char *) &serveraddr, sizeof(serveraddr)); 
	serveraddr.sin_family = AF_INET;
	bcopy((char *)hp->h_addr,(char *)&serveraddr.sin_addr.s_addr, hp->h_length); 
	serveraddr.sin_port = htons(port);

	/* Establish a connection with the server */
	if (connect(clientfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0) {
		return -1; 
	}

	return clientfd;
}










