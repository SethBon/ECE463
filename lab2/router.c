#include "ne.h"
#include "router.h"



int open_listenfd(int router_port);


int main(int argc, char ** argv) {

	int router_id;
	char * ne_host;
	int ne_port;
	int router_port;
	
	int routerfd;

	struct pkt_INIT_REQUEST init_req;
	struct pkt_INIT_RESPONSE init_resp;

	struct sockaddr_in ne_addr;

	int ne_addrlen = sizeof(ne_addr);





	
	
	

	if(argc != 5) {
		printf("usage: router <router id> <ne_host> <ne_port> <router_port>\n");
	}

	router_id = atoi(argv[1]);
	ne_host = argv[2];
	ne_port = atoi(argv[3]);
	router_port = atoi(argv[4]);

	routerfd = open_listenfd(router_port);

	init_req.router_id = router_id;

	memset(&ne_addr, 0, sizeof ne_addr);

	ne_addr.sin_family = AF_INET;

    inet_pton(AF_INET, ne_host, &ne_addr.sin_addr); // Set the broadcast IP address

    ne_addr.sin_port = htons(router_port); // Set port 1900

    sendto(routerfd, &init_req, sizeof(init_req), 0, (struct sockaddr*)&ne_addr, sizeof ne_addr);
	
	recvfrom(routerfd, &init_resp, sizeof(init_resp), 0, (struct sockaddr*)&ne_addr, &ne_addrlen);
	



	InitRoutingTbl(&init_resp, router_id);

	





		

		


	
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





