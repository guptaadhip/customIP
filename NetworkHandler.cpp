#include "include/NetworkHandler.h"
#include "include/net.h"

using namespace std;

static const uint32_t MAX_PING = 254; // Maximum number of neighbours to ping

NetworkHandler::NetworkHandler(MyIp *myIps,PacketEngine *packetEngine) {
	_myIps = myIps;
	_packetEngine = packetEngine;
	broadcastPing();
}


void NetworkHandler::broadcastPing(){
	std::unordered_map<uint32_t, std::string> myIps = _myIps->getMyIps();
	uint32_t ip,networkIP,currentDestinationIP;
	uint32_t broadcastCounter;
	
	for (auto myIp : myIps){
		ip = myIp.first;
		networkIP = ip & 0x00ffffff;
		currentDestinationIP = networkIP;
		
		for(broadcastCounter = 0 ; broadcastCounter < MAX_PING ; broadcastCounter++){
			currentDestinationIP += INCREMENT_IP;
			if(ip != currentDestinationIP){
				_packetEngine->sendPing(ip,currentDestinationIP);
			}
		}
	}
	
}