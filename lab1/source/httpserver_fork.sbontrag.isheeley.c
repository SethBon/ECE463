#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTENQ 512


int open_listenfd(int);

void sendFile(int connfd);

int main(int argc, char const *argv[]) {
	
	int listenfd, connfd, port, clientlen;
	
	struct sockaddr_in clientaddr;
	
	struct hostent * hp;
	
	char * haddrp;

	pid_t childpid;



	port = atoi(argv[1]);

	listenfd = open_listenfd(port);

	while(1) {
		clientlen = sizeof(clientaddr);
	
		connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);

		if( (childpid = fork()) == 0 ) {
			close(listenfd);
			sendFile(connfd);
			exit(0);

		}

		

		close(connfd);



	}

	return 0;
}


int open_listenfd(int port) {
	

	int listenfd, optval = 1;

	struct sockaddr_in serveraddr;

	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
		return -1;
	}


	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		return -1;
	}

	if(listen(listenfd, LISTENQ) < 0) {
		return -1;
	}

	return listenfd;



}


void sendFile(int connfd) {
	
	size_t n;
	char buf[512];

	char * httpCommand;
	char * filename;
	char * httpVersion;

	char * running;

	const char delimiter[] = " \n";

	int i;

	FILE * fh;


	n = read(connfd, buf, 512);

	running = buf;


	printf("%s\n\n", buf);

	httpCommand = strsep(&running, delimiter);
	filename = strsep(&running, delimiter);
	httpVersion = strsep(&running, delimiter);

	 

	i = 0;
	while(httpCommand[i] != '\0') {
		httpCommand[i] = toupper(httpCommand[i++]);
	}

	printf("\nhttpCommand = %s\n", httpCommand); 
	printf("\nfilename = %s\n", filename); 
	printf("\nhttpVersion = %s\n", httpVersion);

	int test = strcmp(httpCommand, "GET") == 0;
	printf("%d\n\n", test);


	if(strcmp(httpCommand, "GET") == 0) {
		
		printf("its a get!");


		if(access(filename, F_OK) != 0) {
			send(connfd, "HTTP/1.0 404 Not Found\r\n\r\n", 26, 0);
			return;
		}


		if(access(filename, R_OK) != 0) {
			send(connfd, "HTTP/1.0 403 Forbidden\r\n\r\n", 26, 0);
			return;
		}

		printf("before fopen\n");
		fh = fopen(filename, "r");
		if(fh == NULL){
			printf("file handle is null");
		}
		printf("before send\n");
		send(connfd, "HTTP/1.0 200 OK\r\n\r\n", 19, 0);
		printf("HTTP/1.0 200 OK\r\n\r\n");
		while(!feof(fh)) {
			bzero(buf, 512);
			fread(buf, 511, 1, fh);
			send(connfd, buf, 511, 0);
			//printf(buf);
		
		}





	}






	//write(connfd, buf, n);
	




}