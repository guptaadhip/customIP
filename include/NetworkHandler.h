/* This header file contains class which pings and maintains OSPF functionality to be always updated about the network topology */
#pragma once
#include <memory>

class NetworkHandler{
	public:
		NetworkHandler();
		void broadcastPing();
};
