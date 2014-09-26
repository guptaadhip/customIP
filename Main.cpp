#include "include/RouteTable.h"
#include "include/Sniffer.h"
#include <net/ethernet.h>
#include <iostream>

using namespace std;

/* global Route table */
RouteTable routeTable;

static void callbackHandler(u_char *args, const struct pcap_pkthdr* pkthdr,
                                                        const u_char* packet) {
  
  u_int caplen = pkthdr->caplen; 
  u_int length = pkthdr->len;
  struct ether_header *eptr;
  u_short ether_type;
  eptr = (struct ether_header *) packet;
  ether_type = ntohs(eptr->ether_type);
  cout << "Source Host: " << ether_ntoa((struct ether_addr*)eptr->ether_shost)
  << " Destination Host: " << ether_ntoa((struct ether_addr*)eptr->ether_dhost)
  << std::endl;
}

int main() {
  /* Testing the routing table */
  //RouteTable routeTable;
  /* 192.168.0.2/32 nexthop: 192.168.0.1, interface 1 */
  /*RouteEntry entry(3232235778,3232235777, 4294967295, 10);
  routeTable.insert(entry);
  auto found = routeTable.search(3232235778);
  if (found) {
    cout << "Interface: " << found->getInterface() << std::endl;
  }*/
  Sniffer sniff;
  sniff.registerCallback(callbackHandler);
  return 0;
}