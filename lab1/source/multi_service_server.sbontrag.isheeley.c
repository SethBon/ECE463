#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


#define LISTENQ 512
#define UDP 1
#define TCP 0
#define MAX(a,b) ((a) > (b) ? a : b)


int open_listenfd(int, int);

void sendFile(int connfd);

void handlePing(int);




int main(int argc, char const *argv[]) {
	
	int http_listenfd, ping_listenfd, connfd, http_port, ping_port, clientlen;
	
	struct sockaddr_in clientaddr;
	
	struct hostent * hp;
	
	char * haddrp;

	pid_t childpid;

	fd_set rfds;

	int retval;

	int maxfd;

	



	http_port = atoi(argv[1]);
	ping_port = atoi(argv[2]);

	http_listenfd = open_listenfd(http_port, 0);
	ping_listenfd = open_listenfd(ping_port, 1);
	//printf("\nping_listenfd = %d\n", ping_listenfd);


	while(1) {
		FD_ZERO(&rfds);
		FD_SET(http_listenfd, &rfds);
		FD_SET(ping_listenfd, &rfds);

		maxfd = MAX(http_listenfd, ping_listenfd) + 1;

		retval = select(maxfd, &rfds, NULL,	NULL, NULL);

		if(FD_ISSET(http_listenfd, &rfds)) {
			//printf("\nHTTP FD IS SET\n");
			clientlen = sizeof(clientaddr);
	
			connfd = accept(http_listenfd, (struct sockaddr*)&clientaddr, &clientlen);

			if( (childpid = fork()) == 0 ) {
				close(http_listenfd);
				sendFile(connfd);
				exit(0);

			}
		}


		if(FD_ISSET(ping_listenfd, &rfds)) {
			printf("\nPING FD IS SET\n");
			handlePing(ping_listenfd);


		}


		

		

		close(connfd);



	}

	return 0;
}


void handlePing(int sockfd) {

	size_t len = 68;
	char buf[68] = {0};
	struct sockaddr src_addr;
	socklen_t addrlen;
	uint32_t seq;

	//printf("\nHandling Ping: %d\n", sizeof(seq));

	recvfrom(sockfd, buf, len, 0, &src_addr, &addrlen);

	

	
	seq = (*((uint32_t *) buf));
	seq = ntohl(seq);
	seq++;
	(*((uint32_t *) buf)) = htonl(seq);

	//ntohl(seq);

	//printf("\nseq: %u\n", seq);

	//(*((uint32_t *) buf))++;

	//printf("\nseq: %u\n", (*((uint32_t *) buf)));
	
	//htonl(*((uint32_t *) buf));


	

	sendto(sockfd, buf, len, 0, &src_addr, addrlen);






}


int open_listenfd(int port, int protocol) {
	

	int listenfd, optval = 1;

	struct sockaddr_in serveraddr;

	if(protocol == TCP){
		//printf("\nTCP\n");
		if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			return -1;
		}
	}

	//UDP
	else {
		if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			return -2;
		}
	}

	

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
		return -3;
	}


	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		close(listenfd);
		return -4;
	}

	if(protocol == TCP){
		if(listen(listenfd, LISTENQ) < 0) {
			return -5;
		}
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


	//printf("%s\n", buf);

	httpCommand = strsep(&running, delimiter);
	filename = strsep(&running, delimiter);
	httpVersion = strsep(&running, delimiter);

	 

	i = 0;
	while(httpCommand[i] != '\0') {
		httpCommand[i] = toupper(httpCommand[i++]);
	}

	//printf("\nhttpCommand = %s\n", httpCommand); 
	//printf("\nfilename = %s\n", filename); 
	//printf("\nhttpVersion = %s\n", httpVersion);



	if(strcmp(httpCommand, "GET") == 0) {
		
		//printf("its a get!");


		if(access(filename, F_OK) != 0) {
			send(connfd, "HTTP/1.0 404 Not Found\r\n\r\n", 26, 0);
			return;
		}


		if(access(filename, R_OK) != 0) {
			send(connfd, "HTTP/1.0 403 Forbidden\r\n\r\n", 26, 0);
			return;
		}

		//printf("before fopen\n");
		fh = fopen(filename, "r");
		if(fh == NULL){
			return;
		}
		//printf("before send\n");
		send(connfd, "HTTP/1.0 200 OK\r\n\r\n", 19, 0);
		//printf("HTTP/1.0 200 OK\r\n\r\n");
		while(!feof(fh)) {
			bzero(buf, 512);
			fread(buf, 511, 1, fh);
			send(connfd, buf, 511, 0);
			//printf(buf);
		
		}

		fclose(fh);





	}






	//write(connfd, buf, n);
	




}