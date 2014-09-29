#include "include/RouteTable.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <iostream>

using namespace std;

RouteTable::RouteTable() {
  kernelSocketFd_ = socket(AF_INET, SOCK_DGRAM, 0);
	if (kernelSocketFd_ < 0) {
		cout << "Route Table: Error in opening kernel socket" << endl;
		exit(1);
	}
}

void RouteTable::insert(RouteEntry entry) {
	auto check = routeTable_.find(entry.getNwAddress());
	if(check != routeTable_.end()) {
  	routeTable_.insert(std::make_pair(entry.getNwAddress(), entry));
  	updateKernelRouteTable(entry);
	}
}

/* 
 * Returns -1 if no match is found 
 */
RouteEntry * RouteTable::search(uint32_t address) {
  std::unordered_map<uint32_t,RouteEntry>::iterator match =
                                                      routeTable_.find(address);
  
  if (match == routeTable_.end()) {
    return nullptr;
  } else {
    return &match->second;
  }
}

/* add my ip routes */
void RouteTable::addMyRoutes(std::unordered_map<uint32_t, std::string> ipList) {
	for (auto entry : ipList) {
		/* to get the network address from the IP we use the subnet mask */
		RouteEntry route((entry.first & 0x00FFFFFF), 0x00000000, 0x00FFFFFF, 
																																		entry.second);
		insert(route);
	}
}

void RouteTable::remove(uint32_t address) {
  routeTable_.erase(address);
}

/* just for debugging purposes need to be removed */
void RouteTable::printRouteTable() {
	for (auto entry : routeTable_) {
		auto temp = entry.second;
		cout << temp.getNwAddress() << " " << temp.getNextHop() 
				 << " " << temp.getSubnetMask() << " " << temp.getInterface()
				 << endl;
	}
}

/* Update the kernel routing table */
void updateKernelRouteTable(RouteEntry entry) {
	struct rtentry kRouteEntry;
	memset(&kRouteEntry, 0, sizeof(kRouteEntry));


  /* setting the next hop */
  ((struct sockaddr_in *) &kRouteEntry.rt_gateway)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_gateway)->sin_addr.s_addr
																								= entry.getNextHop();
  
  /* setting the network address */
  ((struct sockaddr_in *) &kRouteEntry.rt_dst)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_dst)->sin_addr.s_addr
																								= entry.getNwAddress();
  
  /* setting the subnet mask */
  ((struct sockaddr_in *) &kRouteEntry.rt_genmask)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_genmask)->sin_addr.s_addr
																								= entry.getSubnetMask();

  /* setting the interface */
  kRouteEntry.rt_dev = entry.getInterface().c_str;
  
  /* settin flags for Routing table */
  kRouteEntry.rt_flags = RTF_UP | RTF_GATEWAY;

  /* adding the entry to the routing table */
	if ((int err = ioctl(kernelSocketFd_, SIOCADDRT, &kRouteEntry)) < 0) {
		cout << "Route Table: Error in setting the kernel table" << endl;
	}
}