#include <stdio.h>
#include <stdlib.h>
#include "client.h"
#include <netdb.h>


int main(int argc, char ** argv) {
	
	if(argc != 3) {
		printf("wrong number of input parameters\n");
		return -1;
	}

	int clientfd;
	int port;
	char * host;
	char * buf[MAXLINE];

	host = argv[1];

	port = atoi(argv[2]);

	clientfd = openClientFd(host, port);




}





int openClientFd(char * host, int port) {
	

	int clientfd;

	struct hostent * hp;

	hp = gethostbyname(host);

	if(hp) {
		puts(hp->h_name);
	}
	else {
		puts("error");
	}



}