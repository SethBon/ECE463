#include "ne.h"

//This is the routing table
struct route_entry routingTable[MAX_ROUTERS];

//Number of routers in the routing table
int NumRoutes;

//Function that checks to see if an entry in the routing table needs updated
bool needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID);


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
  	unsigned int costToUpdater = routingTable[getEntryIdxById(RecvdUpdatePacket->sender_id)].cost;

  	for (i = 0; i < RecvdUpdatePacket->no_routes; i++) {
  		updateEntry = RecvdUpdatePacket->route[i];


  		for(j = 0; j < NumRoutes; j++) {
  			myEntry = &routingTable[j];

  			if(myEntry.dest_id == updateEntry.dest_id) {

  				if(needsUpdate(myEntry, &updateEntry, costToUpdater, RecvdUpdatePacket->sender_id, myID)) {
  					myEntry->next_hop = RecvdUpdatePacket->sender_id;
  					myEntry->cost = costToUpdater + updateEntry->cost;
  					
  				}
  				break;

  			}






  		}

  		if(j == NumRoutes) {
  			//add element
  		}



  	}

}

int getEntryIdxById(node_id) {
	int i; 
	for(i = 0; i < NumRoutes; i++) {
		if(routingTable[i].dest_id == node_id) {
			return i;
		}

	}

	return -1;

}



bool needsUpdate(struct route_entry * myEntry, struct route_entry * updateEntry, unsigned int costToUpdater, unsigned int updater_id, unsigned int myID) {

	unsigned int newCost = costToUpdater + updateEntry->cost;

	if(myEntry->next_hop == updater_id) {
		return TRUE;
	}

	else if(newCost < myEntry->cost && updateEntry->next_hop != myID) {
		return TRUE;
	}
	else {
		return FALSE;
	}


}
