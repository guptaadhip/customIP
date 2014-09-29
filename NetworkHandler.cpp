#include "include/NetworkHandler.h"
#include "include/net.h"

/* to be removed */
#include <iostream>

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
	
	//struct sockaddr_in dest;
	
	for (auto myIp : myIps){
		ip = myIp.first;
		networkIP = ip & 0x00ffffff;
		currentDestinationIP = networkIP;
		
		/*dest.sin_family = AF_INET;
		dest.sin_addr.s_addr = ip;
		cout << inet_ntoa(dest.sin_addr) << " " << ntohl(ip);
		dest.sin_addr.s_addr = networkIP;
		cout << " : " << inet_ntoa(dest.sin_addr) << " " << ntohl(networkIP) << endl;
		dest.sin_addr.s_addr = currentDestinationIP;
		cout << " : " << inet_ntoa(dest.sin_addr) << " " << ntohl(currentDestinationIP) << endl;*/
		
		for(broadcastCounter = 0 ; broadcastCounter < MAX_PING ; broadcastCounter++){
			currentDestinationIP += INCREMENT_IP;
			if(ip != currentDestinationIP){
				_packetEngine->sendPing(ip,currentDestinationIP);
				//dest.sin_addr.s_addr = currentDestinationIP;
				//cout << " : " << inet_ntoa(dest.sin_addr) << " " << ntohl(currentDestinationIP) << endl;
			}
		}
	}
	
}