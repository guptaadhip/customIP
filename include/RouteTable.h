#pragma once
#include "RouteEntry.h"
#include "net.h"
#include <unordered_map>
#include <cstdint>
#include <vector>

/* Ip Table Entry Object */
class RouteTable {
 public:
  RouteTable();
  
  /* Insert an entry in the Route Table */
  void insert(RouteEntry);

  /* add my Ip to the routeTable */
  void addMyRoutes(std::unordered_map<uint32_t, std::string> myIps);
  
	/* Search an entry in the Route Table with Highest Priority on basis of 
																															network address */
	RouteEntry searchHighestPriority(uint32_t);
	
  /* Search an entry in the Route Table on basis of network address */
  RouteEntry * search(uint32_t);
	
	/* Search all entries in the Route Table on basis of priority */
	std::vector<RouteEntry> searchAll(RoutePriority);
	
	/* Search all entries in the Route Table on basis of next hop */
	std::vector<RouteEntry> searchAll(uint32_t);
	
	/* Search all entries in the Route Table on basis of network 
																								address and priority */
	std::vector<RouteEntry> searchAll(uint32_t, RoutePriority);

	/* Search all entries in the Route Table on basis of network address */
	std::vector<RouteEntry> searchAllNWAddress(uint32_t);
	
  /* Remove all entry in the Route Table on basis of network address */
  void remove(uint32_t);

	/* Remove an entry in the Route Table on basis of next hop */
	std::vector<RouteEntry> removeEntries(uint32_t);
	
	/* Remove an entry in the Route Table on basis of network address & next hop */
	std::vector<RouteEntry> removeEntry(uint32_t,uint32_t);
	
  /* just for debugging purposes need to be removed */
  void printRouteTable();
  
 private:
	typedef std::unordered_multimap<uint32_t, RouteEntry> routeTableMap_;
  routeTableMap_ routeTable_;
  int kernelSocketFd_;
  /* Function to update kernel routing table */
  void addKernelRouteTable(RouteEntry);
	void removeKernelRouteTable(RouteEntry);
};
