/* This header file contains class which pings and maintains OSPF functionality to be always updated about the network topology */
#pragma once
#include <memory>
#include "MyIp.h"
#include "PacketEngine.h"

class NetworkHandler{
	public:
		NetworkHandler(MyIp * myIps,PacketEngine *packetEngine);
		void broadcastPing();
		//void generateOSPF();
		
	private:
	MyIp *_myIps;
	PacketEngine *_packetEngine;
};