#include "include/RouteTable.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/route.h>
#include <iostream>
#include <cstring>
#include <string>

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
	if(check == routeTable_.cend()) {
  	routeTable_.insert(routeTableMap_::value_type(entry.getNwAddress(), entry));
  	updateKernelRouteTable(entry);
	}else{ //if the key  exists , check if the that entry exists
		auto elements = routeTable_.equal_range(entry.getNwAddress());
		bool flag = true;
		
		for (auto element = elements.first; element != elements.second; ++element) {	
			if(element->second.getNwAddress() == entry.getNwAddress() 
					&& element->second.getSubnetMask() == entry.getSubnetMask() 
					&& element->second.getInterface() == entry.getInterface() 
					&& element->second.getNextHop() == entry.getNextHop() ){
					flag = false;
					break;
			}
		}
		
		if(flag){
			routeTable_.insert(routeTableMap_::value_type(entry.getNwAddress(), entry));
			updateKernelRouteTable(entry);
		}
	}
}

/* Search an entry in the Route Table on basis of network address */
RouteEntry * RouteTable::search(uint32_t address) {
	auto elements = routeTable_.equal_range(address);
  if (elements.first == elements.second) {
    return nullptr;
  } else {
		return &elements.first->second;
  }
}

/* Search all entries in the Route Table on basis of priority */
std::list<RouteEntry> RouteTable::searchAll(RoutePriority priority) {
	routeTableMap_::iterator mapIterator;
	std::list<RouteEntry> returnList;
	
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();
                                                              ++mapIterator){
		if (mapIterator->second.getPriority() == priority){
      returnList.push_back(mapIterator->second);
    }
	}
	return returnList;
}

/* Search all entries in the Route Table on basis of next hop */
std::list<RouteEntry> RouteTable::searchAll(uint32_t nextHop) {
	routeTableMap_::iterator mapIterator;
	std::list<RouteEntry> returnList;
	
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();
                                                              ++mapIterator){
		if (mapIterator->second.getNextHop() == nextHop){
      returnList.push_back(mapIterator->second);
    }
	}
	return returnList;
}

/* Search all entries in the Route Table on basis of network address 
																															and priority */
std::list<RouteEntry> RouteTable::searchAll(uint32_t address,
	                                                   RoutePriority priority) {
	routeTableMap_::iterator mapIterator;
	std::list<RouteEntry> returnList;
	
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();
                                                              ++mapIterator){
		if (mapIterator->second.getNwAddress() == address
				&& mapIterator->second.getPriority() == priority){
      returnList.push_back(mapIterator->second);
    }
	}
	return returnList;
}

/* Search all entries in the Route Table on basis of network address */
std::list<RouteEntry> RouteTable::searchAllNWAddress(uint32_t address) {
	routeTableMap_::iterator mapIterator;
	std::list<RouteEntry> returnList;
	
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();
                                                              ++mapIterator){
		if (mapIterator->second.getNwAddress() == address){
      returnList.push_back(mapIterator->second);
    }
	}
	return returnList;
}


/* add my ip routes */
void RouteTable::addMyRoutes(std::unordered_map<uint32_t, std::string> ipList) {
	for (auto entry : ipList) {
		/* to get the network address from the IP we use the subnet mask */
		RouteEntry route((entry.first & 0x00FFFFFF), 0x00000000, 0x00FFFFFF,
                                                    entry.second,
																											RoutePriority::LOCAL);
		insert(route);
	}
}

/* Remove all entry in the Route Table on basis of network address */
void RouteTable::remove(uint32_t address) {
  routeTable_.erase(address);
}

/* Remove an entry in the Route Table on basis of next hop */
void RouteTable::removeEntry(uint32_t nextHop){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNextHop() == nextHop){
        routeTable_.erase( mapIterator++ );
    }else{
        ++mapIterator;
    }
	}
}
	
/* Remove an entry in the Route Table on basis of next hop and interface */
void RouteTable::removeEntry(uint32_t nextHop,std::string interface){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNextHop() == nextHop 
				&& interface.compare(mapIterator->second.getInterface()) == 0){
        routeTable_.erase( mapIterator++ );
    }else{
        ++mapIterator;
    }
	}
}

/* Remove an entry in the Route Table on basis of network address, next hop and interface */
void RouteTable::removeEntry(uint32_t address,uint32_t nextHop,
																											std::string interface){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNwAddress() == address  
				&& mapIterator->second.getNextHop() == nextHop
				&& interface.compare(mapIterator->second.getInterface()) == 0){
        routeTable_.erase( mapIterator++ );
    }else{
        ++mapIterator;
    }
	}
}

/* Remove an entry in the Route Table on basis of priority */
void RouteTable::removeEntry(RoutePriority priority){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getPriority() == priority){
				routeTable_.erase( mapIterator++ );
		}else{
				++mapIterator;
		}
	}
}

/* Remove an entry in the Route Table on basis of network address & priority */
void RouteTable::removeEntry(uint32_t address, RoutePriority priority){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNwAddress() == address  
				&& mapIterator->second.getPriority() == priority){
				routeTable_.erase( mapIterator++ );
		}else{
				++mapIterator;
		}
	}
}

/* Remove an entry in the Route Table on basis of network address, next hop,
																												interface, priority */
void RouteTable::removeEntry(uint32_t address,uint32_t nextHop,
																								 std::string interface,
																									RoutePriority priority){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNwAddress() == address  
				&& mapIterator->second.getNextHop() == nextHop
				&& mapIterator->second.getPriority() == priority
				&& interface.compare(mapIterator->second.getInterface()) == 0){
				routeTable_.erase( mapIterator++ );
		}else{
				++mapIterator;
		}
	}
}

/* Remove an entry in the Route Table on basis of network address & next hop */
void RouteTable::removeEntry(uint32_t address,uint32_t nextHop){
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();){
		if (mapIterator->second.getNwAddress() == address  
				&& mapIterator->second.getNextHop() == nextHop){
				routeTable_.erase( mapIterator++ );
		}else{
				++mapIterator;
		}
	}
}

/* just for debugging purposes need to be removed */
void RouteTable::printRouteTable() {
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	
	routeTableMap_::iterator mapIterator;
	for (mapIterator = routeTable_.begin(); mapIterator!= routeTable_.end();
																											++mapIterator){
		auto temp = mapIterator->second;
		dest.sin_addr.s_addr = temp.getNwAddress();
		cout << inet_ntoa(dest.sin_addr) << "\t" << temp.getNwAddress() << "\t";
		dest.sin_addr.s_addr = temp.getNextHop();
		cout << inet_ntoa(dest.sin_addr) << "\t" << temp.getNextHop() << "\t";
		dest.sin_addr.s_addr = temp.getSubnetMask();
		cout << inet_ntoa(dest.sin_addr) << "\t" << temp.getSubnetMask() << "\t";
		cout << "\t" << temp.getInterface() << endl;
	}
}

/* Update the kernel routing table */
void RouteTable::updateKernelRouteTable(RouteEntry entry) {
	struct rtentry kRouteEntry;
	
	bzero(&kRouteEntry, sizeof(kRouteEntry));
	
  /* setting the next hop */
  ((struct sockaddr_in *) &kRouteEntry.rt_gateway)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_gateway)->sin_addr.s_addr = entry.getNextHop();
  
  /* setting the network address */
  ((struct sockaddr_in *) &kRouteEntry.rt_dst)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_dst)->sin_addr.s_addr = entry.getNwAddress();
  
  /* setting the subnet mask */
  ((struct sockaddr_in *) &kRouteEntry.rt_genmask)->sin_family = AF_INET;
	((struct sockaddr_in *) &kRouteEntry.rt_genmask)->sin_addr.s_addr = entry.getSubnetMask();

  /* setting the interface */
  kRouteEntry.rt_dev = (char *) entry.getInterface().c_str();
  
  /* settin flags for Routing table */
  if (entry.getNextHop() == 0x0) {
    kRouteEntry.rt_flags = RTF_UP;
  } else {
    kRouteEntry.rt_flags = RTF_UP | RTF_GATEWAY;
  }

  /* adding the entry to the routing table */
	if (ioctl(kernelSocketFd_	, SIOCADDRT, &kRouteEntry) < 0) {
		cout << "Route Table: Error in setting the kernel table" << endl;
	}
}