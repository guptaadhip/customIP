#pragma once
#include "RouteEntry.h"
#include <unordered_map>
#include <cstdint>

/* Ip Table Entry Object */
class RouteTable {
 public:
  RouteTable();
  
  /* Insert an entry in the Route Table */
  void insert(RouteEntry);

  /* add my Ip to the routeTable */
  void addMyRoutes(std::unordered_map<uint32_t, std::string> myIps);
  
  /* Search an entry in the Route Table 
   * Returns the interface
   */
  RouteEntry * search(uint32_t);
  
  /* Remove all entry in the Route Table on basis of network address */
  void remove(uint32_t);

	/* Remove an entry in the Route Table on basis of next hop */
	void removeEntry(uint32_t);
	
	/* Remove an entry in the Route Table on basis of next hop and interface */
	void removeEntry(uint32_t,std::string);
	
	/* Remove an entry in the Route Table on basis of network address, next hop and interface */
	void removeEntry(uint32_t,uint32_t,std::string);
	
	
	
  /* just for debugging purposes need to be removed */
  void printRouteTable();
  
 private:
	typedef std::unordered_multimap<uint32_t, RouteEntry> routeTableMap_;
  routeTableMap_ routeTable_;
  int kernelSocketFd_;
  /* Function to update kernel routing table */
  void updateKernelRouteTable(RouteEntry);
};