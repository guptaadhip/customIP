#include "include/RouteTable.h"

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

void RouteTable::remove(uint32_t address) {
  routeTable_.erase(address);
}