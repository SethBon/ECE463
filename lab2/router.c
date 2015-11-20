#include "ne.h"
#include "router.h"
#include <sys/timerfd.h>

#define MAX(a,b) ((a) > (b) ? a : b)


int get_nbr_ind(int nbr_id, struct pkt_INIT_RESPONSE *init_resp);

int main(int argc, char ** argv) {

	unsigned int router_id;
	char * ne_host;
	int ne_port;
	int router_port;
	
	int sendfd;
	int maxfd = 0;

	struct pkt_INIT_REQUEST init_req;
	struct pkt_INIT_RESPONSE init_resp;

	struct sockaddr_in ne_addr;
	struct sockaddr_in router_addr;

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


	init_req.router_id = htonl(router_id);


	if((sendfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	 	printf("\nFailed to open socket\n");
		return -2;
	}


	bzero((char *)&router_addr, sizeof(router_addr));
	router_addr.sin_family = AF_INET;
	router_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	router_addr.sin_port = htons((unsigned short)router_port);



    bzero((char *)&ne_addr, sizeof(ne_addr));
	ne_addr.sin_family = AF_INET;
	inet_aton(ne_host, &ne_addr.sin_addr);
	ne_addr.sin_port = htons((unsigned short)ne_port);


	if(bind(sendfd, (struct sockaddr *)&router_addr, sizeof(router_addr)) < 0) {
		close(sendfd);
	 	printf("\nFailed to bind socket\n");
	 	return -4;
	}


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



	struct pkt_RT_UPDATE UpdatePacketToSend;
	struct pkt_RT_UPDATE RecvdUpdatePacket;

	fd_set rfds;
	int update_timer_fd;
	int converge_timer_fd;
	int nbr_timeout_fd[init_resp.no_nbr];
	int runtime_timer_fd;


	struct itimerspec update_timer;
	struct itimerspec converge_timer;
	struct itimerspec timeout_timer[init_resp.no_nbr];
	struct itimerspec runtime_timer;

	update_timer.it_value.tv_sec = UPDATE_INTERVAL;
	update_timer.it_value.tv_nsec = 0;
	update_timer.it_interval.tv_sec = 0;
	update_timer.it_interval.tv_nsec = 0;

	converge_timer.it_value.tv_sec = CONVERGE_TIMEOUT;
	converge_timer.it_value.tv_nsec = 0;
	converge_timer.it_interval.tv_sec = 0;
	converge_timer.it_interval.tv_nsec = 0;

	runtime_timer.it_value.tv_sec = 0;
	runtime_timer.it_value.tv_nsec = 250000000;
	runtime_timer.it_interval.tv_sec = 0;
	runtime_timer.it_interval.tv_nsec = 0;
	
	int i;
	int retval;
	int tableChanged;
	int nbr_ind;
	int converged;
	int nbr_timeout_status[init_resp.no_nbr];
	int runtime_sec;
	int runtime_qtr_sec;

	update_timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	timerfd_settime(update_timer_fd, 0, &update_timer, NULL);

	converge_timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	timerfd_settime(converge_timer_fd, 0, &converge_timer, NULL);

	runtime_timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	timerfd_settime(runtime_timer_fd, 0, &runtime_timer, NULL);

	maxfd = MAX(update_timer_fd, converge_timer_fd);
	maxfd = MAX(maxfd, runtime_timer_fd);

	for (i = 0; i < init_resp.no_nbr; i++) {
		timeout_timer[i].it_value.tv_sec = FAILURE_DETECTION;
		timeout_timer[i].it_value.tv_nsec = 0;
		timeout_timer[i].it_interval.tv_sec = 0;
		timeout_timer[i].it_interval.tv_nsec = 0;
		nbr_timeout_fd[i] = timerfd_create(CLOCK_REALTIME, 0);
		timerfd_settime(nbr_timeout_fd[i], 0, &timeout_timer[i], NULL);
		maxfd = MAX(maxfd, nbr_timeout_fd[i]);
		nbr_timeout_status[i] = 0;
	}


	while(1) {
		FD_ZERO(&rfds);
		FD_SET(sendfd, &rfds);
		FD_SET(update_timer_fd, &rfds);
		FD_SET(converge_timer_fd, &rfds);
		FD_SET(runtime_timer_fd, &rfds);
		for (i = 0; i < init_resp.no_nbr; i++) {
			FD_SET(nbr_timeout_fd[i], &rfds);
		}

		retval = select(maxfd+1, &rfds, NULL, NULL, NULL);


		// Receive Update from Neighbor
		if(FD_ISSET(sendfd, &rfds)) {
			
			if (recvfrom(sendfd, (struct pkt_RT_UPDATE *)&RecvdUpdatePacket, sizeof(RecvdUpdatePacket), 0, NULL, NULL) < 0) {
				printf("\nFailed to receive data\n");
				return -1;
			}

			ntoh_pkt_RT_UPDATE(&RecvdUpdatePacket);

			nbr_ind = get_nbr_ind(RecvdUpdatePacket.sender_id, &init_resp);
			printf("Receive RT_UPDATE from R%d with cost %d containing %d routes\n", RecvdUpdatePacket.sender_id, init_resp.nbrcost[nbr_ind].cost, RecvdUpdatePacket.no_routes);
			tableChanged = UpdateRoutes(&RecvdUpdatePacket, init_resp.nbrcost[nbr_ind].cost, router_id);

			if (tableChanged) {
				PrintRoutes(logfile, router_id);
				converge_timer.it_value.tv_sec = CONVERGE_TIMEOUT;
				converge_timer.it_value.tv_nsec = 0;
				timerfd_settime(converge_timer_fd, 0, &converge_timer, NULL);
				converged = 0;
			}
			

			timeout_timer[nbr_ind].it_value.tv_sec = FAILURE_DETECTION;
			timeout_timer[nbr_ind].it_value.tv_nsec = 0;
			timerfd_settime(nbr_timeout_fd[nbr_ind], 0, &timeout_timer[nbr_ind], NULL);
			nbr_timeout_status[nbr_ind] = 0;

			fflush(logfile);
		}

		// Send update to neighbors
		if(FD_ISSET(update_timer_fd, &rfds)) {

			bzero((char *)&UpdatePacketToSend, sizeof(UpdatePacketToSend));
			ConvertTabletoPkt(&UpdatePacketToSend, router_id);

			for (i = 0; i < init_resp.no_nbr; i++) {
				
				UpdatePacketToSend.dest_id = init_resp.nbrcost[i].nbr;
				hton_pkt_RT_UPDATE(&UpdatePacketToSend);
				if (sendto(sendfd, (struct pkt_RT_UPDATE *)&UpdatePacketToSend, sizeof(UpdatePacketToSend), 0, (struct sockaddr *)&ne_addr, sizeof(ne_addr)) < 0) {
    				printf("\nFailed to send data\n");
    				return -1;
    			}
    			ntoh_pkt_RT_UPDATE(&UpdatePacketToSend);
			}

			update_timer.it_value.tv_sec = UPDATE_INTERVAL;
			update_timer.it_value.tv_nsec = 0;
			timerfd_settime(update_timer_fd, 0, &update_timer, NULL);

			if (converged) {
				printf("%d:Routing Table Converged\n", runtime_sec);
			}

			for (i = 0; i < init_resp.no_nbr; i++) {
				if(nbr_timeout_status[i]) {
					printf("Neighbor R%d is dead or link to it is down\n", init_resp.nbrcost[i].nbr);
				}
			}
			fflush(logfile);
		}

		// Routing table converged
		if(FD_ISSET(converge_timer_fd, &rfds)) {
			if (converged == 0) {
				fprintf(logfile, "%d:Converged\n", runtime_sec);
			}
			converge_timer.it_value.tv_sec = CONVERGE_TIMEOUT;
			converge_timer.it_value.tv_nsec = 0;
			timerfd_settime(converge_timer_fd, 0, &converge_timer, NULL);
			converged = 1;
			fflush(logfile);
		}


		if(FD_ISSET(runtime_timer_fd, &rfds)) {
			runtime_qtr_sec = (runtime_qtr_sec + 1) % 4;
			if (runtime_qtr_sec == 3) {
				runtime_sec += 1;
			}

			runtime_timer.it_value.tv_sec = 0;
			runtime_timer.it_value.tv_nsec = 250000000;
			timerfd_settime(runtime_timer_fd, 0, &runtime_timer, NULL);

		}

		// Neighbor timeout
		for (i = 0; i < init_resp.no_nbr; i++) {
			if(FD_ISSET(nbr_timeout_fd[i], &rfds)) {
				if (nbr_timeout_status[i] == 0) {
					UninstallRoutesOnNbrDeath(init_resp.nbrcost[i].nbr);
					PrintRoutes(logfile, router_id);

					// Reset converged timer
					converge_timer.it_value.tv_sec = CONVERGE_TIMEOUT;
					converge_timer.it_value.tv_nsec = 0;
					timerfd_settime(converge_timer_fd, 0, &converge_timer, NULL);
					converged = 0;
					
					fflush(logfile);
				}
				nbr_timeout_status[i] = 1;
			}
		}	
		



		
	}





	fclose(logfile);
	
	return 0;
	


}


int get_nbr_ind(int nbr_id, struct pkt_INIT_RESPONSE *init_resp) {
	int i;

	for (i = 0; i < init_resp->no_nbr; i++) {
		if(init_resp->nbrcost[i].nbr == nbr_id) {
			return i;
		}
	}
	return -1;
}


