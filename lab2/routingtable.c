#include "ne.h"
#include "router.h"

//Function that checks to see if an entry in the routing table needs updated
int needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID);



void PrintRoutes_DEBUG (int myID);


//This is the routing table
struct route_entry routingTable[MAX_ROUTERS];

//Number of routers in the routing table
int NumRoutes;



void InitRoutingTbl (struct pkt_INIT_RESPONSE *InitResponse, int myID) {

	int i;
	
	for (i = 0; i < InitResponse->no_nbr; i++) {
		routingTable[i].dest_id = InitResponse->nbrcost[i].nbr;
		routingTable[i].next_hop = InitResponse->nbrcost[i].nbr;
		routingTable[i].cost = InitResponse->nbrcost[i].cost;
	}


	routingTable[i].dest_id = myID;
	routingTable[i].next_hop = myID;
	routingTable[i].cost = 0;

	NumRoutes = i + 1;

}


int UpdateRoutes(struct pkt_RT_UPDATE *RecvdUpdatePacket, int costToNbr, int myID) {

	struct route_entry * myEntry;
	struct route_entry updateEntry;

  	int i;
  	int j;
  	int tableChanged = 0;
  
  	for (i = 0; i < RecvdUpdatePacket->no_routes; i++) {
  		updateEntry = RecvdUpdatePacket->route[i];

  		
  		for(j = 0; j < NumRoutes; j++) {
  			myEntry = &routingTable[j];

  			/*
  			printf("\n\nUpdate Entry:\n");
			printf("%d -> %d: %d, %d", RecvdUpdatePacket->sender_id, updateEntry.dest_id, updateEntry.next_hop, updateEntry.cost);
			printf("\n\nMy Entry:\n");
			printf("%d -> %d: %d, %d", myID, myEntry->dest_id, myEntry->next_hop, myEntry->cost);
			*/

  			if(myEntry->dest_id == updateEntry.dest_id) {

  				
				
		

			


  				if(needsUpdate(myEntry, &updateEntry, costToNbr, RecvdUpdatePacket->sender_id, myID)) {
  					myEntry->next_hop = RecvdUpdatePacket->sender_id;
  					myEntry->cost = costToNbr + updateEntry.cost;
  					tableChanged = 1;
  					
  				}
  			
  				break;

  			}

  		}

  		if( j == NumRoutes ) {
  			//printf("\n\nADDING\n\n");
  			routingTable[NumRoutes].dest_id = updateEntry.dest_id;
			routingTable[NumRoutes].next_hop = updateEntry.next_hop;
			routingTable[NumRoutes].cost = updateEntry.cost + costToNbr;
			NumRoutes++;
			tableChanged = 1;

  		}



  	}

  	//PrintRoutes_DEBUG(myID);

  	return tableChanged;

}



int needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID) {

	unsigned int newCost = costToUpdater + updateEntry->cost;

	if(myEntry->next_hop == updater_id) {
		return 1;
	}

	else if(newCost < myEntry->cost && updateEntry->next_hop != myID) {
		return 1;
	}
	else {
		return 0;
	}


}


void ConvertTabletoPkt(struct pkt_RT_UPDATE *UpdatePacketToSend, int myID) {


	UpdatePacketToSend->sender_id = myID;
	UpdatePacketToSend->no_routes = NumRoutes;


	int i;
	for ( i = 0; i < NumRoutes; i++ ) {
		UpdatePacketToSend->route[i] = routingTable[i];
	}

}


void PrintRoutes (FILE* Logfile, int myID) {
	fprintf(Logfile, "Routing Table:\n");

	int i;
	for (i = 0; i < NumRoutes; i++) {
		fprintf(Logfile, "%d -> %d: %d, %d\n", myID, routingTable[i].dest_id, routingTable[i].next_hop, routingTable[i].cost);
	}
	fprintf(Logfile, "\n\n");
}


void PrintRoutes_DEBUG (int myID) {
	printf("Routing Table:\n");

	int i;
	for (i = 0; i < NumRoutes; i++) {
		printf("%d -> %d: %d, %d\n", myID, routingTable[i].dest_id, routingTable[i].next_hop, routingTable[i].cost);
	}
	printf("\n\n");
}



void UninstallRoutesOnNbrDeath(int DeadNbr) {
	int i;

	for ( i = 0; i < NumRoutes; i++ ) {
		if ( routingTable[i].next_hop == DeadNbr ) {
			routingTable[i].cost = INFINITY;
		}
	}


}