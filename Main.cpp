#include "include/RouteTable.h"
#include <iostream>

using namespace std;

int main() {
  /* Testing the routing table */
  RouteTable routeTable;
  /* 192.168.0.2/32 nexthop: 192.168.0.1, interface 1 */
  RouteEntry entry(3232235778,3232235777, 4294967295,10);
  routeTable.insert(entry);
  auto found = routeTable.search(3232235778);
  if (found) {
    cout << "Interface: " << found->getInterface() << std::endl;
  }
  return 0;
}