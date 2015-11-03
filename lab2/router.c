#include "ne.h"
#include "router.h"



int open_listenfd(int router_port);


int main(int argc, char ** argv) {

	unsigned int router_id;
	char * ne_host;
	int ne_port;
	int router_port;
	
	int routerfd;
	int sendfd;

	struct pkt_INIT_REQUEST init_req;
	struct pkt_INIT_RESPONSE init_resp;

	struct sockaddr_in ne_addr;

	char filename[15];
	FILE * logfile;




	
	
	

	if(argc != 5) {
		printf("usage: router <router id> <ne_host> <ne_port> <router_port>\n");
		return -1;
	}


	router_id = atoi(argv[1]);
	ne_host = argv[2];
	ne_port = atoi(argv[3]);
	router_port = atoi(argv[4]);

	strcpy(filename, "router");
	strcat(filename, argv[1]);
	strcat(filename, ".log");
	logfile = fopen(filename, "w");

	if (router_id < 0 || router_id > (MAX_ROUTERS - 1)) {
		printf("Router ID must be between 0 and %d\n", MAX_ROUTERS-1);
		return -1;
	}

	routerfd = open_listenfd(router_port);

	init_req.router_id = htonl(router_id);


	if((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("\nFailed to open socket\n");
		return -2;
	}

    bzero((char *)&ne_addr, sizeof(ne_addr));
	ne_addr.sin_family = AF_INET;
	inet_aton(ne_host, &ne_addr.sin_addr);
	ne_addr.sin_port = htons((unsigned short)ne_port);


    if (sendto(sendfd, (struct pkt_INIT_REQUEST *)&init_req, sizeof(init_req), 0, (struct sockaddr *)&ne_addr, sizeof(ne_addr)) < 0) {
    	printf("\nFailed to send data\n");
    	return -1;
    }
	
	if (recvfrom(sendfd, (struct pkt_INIT_RESPONSE *)&init_resp, sizeof(init_resp), 0, NULL, NULL) < 0) {
		printf("\nFailed to receive data\n");
		return -1;
	}
	
	printf("R%u received INIT_RESPONSE\n", router_id);

	ntoh_pkt_INIT_RESPONSE(&init_resp);

	InitRoutingTbl(&init_resp, router_id);

	PrintRoutes(logfile, router_id);


	fclose(logfile);


		

		


	
	return 0;
	


}


int open_listenfd(int router_port) {
	

	int listenfd, optval = 1;

	struct sockaddr_in serveraddr;

	
	if((listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -2;
	}
	

	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0) {
		return -3;
	}


	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)router_port);

	if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		close(listenfd);
		return -4;
	}



	return listenfd;



}





