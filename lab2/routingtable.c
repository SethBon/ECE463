#include "ne.h"
#include "router.h"

//Function that checks to see if an entry in the routing table needs updated
int needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID);


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

  			if(myEntry->dest_id == updateEntry.dest_id) {


  				if(needsUpdate(myEntry, &updateEntry, costToNbr, RecvdUpdatePacket->sender_id, myID)) {
  					myEntry->next_hop = RecvdUpdatePacket->sender_id;
  		
  					if (myEntry->cost == INFINITY && costToNbr + updateEntry.cost >= INFINITY) {
  						myEntry->cost = INFINITY;
  						tableChanged = 0;
  					}
  					else if (costToNbr + updateEntry.cost >= INFINITY) {
  						myEntry->cost = INFINITY;
  						tableChanged = 1;
  					}
  					else {
  						myEntry->cost = costToNbr + updateEntry.cost;
  						tableChanged = 1;		
  					}
  				}
  				break;
  			}

  		}
  		// Entry not found in routing table, add to table
  		if( j == NumRoutes ) {
  			routingTable[NumRoutes].dest_id = updateEntry.dest_id;
			routingTable[NumRoutes].next_hop = RecvdUpdatePacket->sender_id;
			routingTable[NumRoutes].cost = updateEntry.cost + costToNbr;
			NumRoutes++;
			tableChanged = 1;

  		}
  	}

  	return tableChanged;
}



int needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID) {

	unsigned int newCost = costToUpdater + updateEntry->cost;

	if(myEntry->next_hop == updater_id) {
		if(myEntry->cost == newCost) {		
			return 0;
		}
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
	fprintf(Logfile, "\nRouting Table:\n");

	int i;
	int j;

	for (j = 0; j < MAX_ROUTERS; j++) {
		for (i = 0; i < NumRoutes; i++) {
			if (routingTable[i].dest_id == j) {
				fprintf(Logfile, "%d -> %d: %d, %d\n", myID, routingTable[i].dest_id, routingTable[i].next_hop, routingTable[i].cost);
			}
		}
	}
}



void UninstallRoutesOnNbrDeath(int DeadNbr) {
	int i;

	for ( i = 0; i < NumRoutes; i++ ) {
		if ( routingTable[i].next_hop == DeadNbr ) {
			routingTable[i].cost = INFINITY;
		}
	}
}


