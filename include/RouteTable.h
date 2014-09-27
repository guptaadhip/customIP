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
  
  /* Search an entry in the Route Table 
   * Returns the interface
   */
  RouteEntry * search(uint32_t);
  
  /* Remove an entry in the Route Table */
  void remove(uint32_t);
  
 private:
  std::unordered_map<uint32_t, RouteEntry> routeTable_;
};