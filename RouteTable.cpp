#include "include/RouteTable.h"
#include <iostream>

using namespace std;

RouteTable::RouteTable() {
  
}

void RouteTable::insert(RouteEntry entry) {
  routeTable_.insert(std::make_pair(entry.getNwAddress(), entry));
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