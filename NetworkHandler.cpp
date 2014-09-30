#include "include/CustomIp.h"
#include "include/NetworkHandler.h"
#include "include/net.h"

/* to be removed */
#include <iostream>

using namespace std;

static const uint32_t MAX_PING = 254; // Maximum number of neighbours to ping

NetworkHandler::NetworkHandler() {
	broadcastPing();
}


void NetworkHandler::broadcastPing(){
	std::unordered_map<uint32_t, std::string> Ips = myIps.getMyIps();
	uint32_t ip,networkIP,currentDestinationIP;
	uint32_t broadcastCounter;
	
	for (auto myIp : Ips){
		ip = myIp.first;
		networkIP = ip & 0x00ffffff;
		currentDestinationIP = networkIP;
		
		for(broadcastCounter = 0 ; broadcastCounter < MAX_PING ; broadcastCounter++){
			currentDestinationIP += INCREMENT_IP;
			if(ip != currentDestinationIP){
				packetEngine.sendPing(ip,currentDestinationIP);
			}
		}
	}
}
