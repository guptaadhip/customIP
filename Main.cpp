#include "include/RouteTable.h"
#include "include/Sniffer.h"
#include "include/net.h"
#include <sys/socket.h>

#include <iostream>

using namespace std;


/* global Route table */
RouteTable routeTable;

static void callbackHandler(u_char *args, const struct pcap_pkthdr* pkthdr,
                                                        const u_char* packet) {
  
  u_int caplen = pkthdr->caplen; 
  u_int length = pkthdr->len;
  const struct ethernetHeader *ethernet;
  const struct ipHeader *ip;
  u_short ethernetType;
  ethernet = (struct ethernetHeader *) packet;
  ethernetType = ntohs(ethernet->ethType);
  if (ethernetType == ethTypeIp) {
    ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
    cout << "Source Host: " << inet_ntoa(ip->ipSrc)
         << " Destination Host: " << inet_ntoa(ip->ipDst)
         << std::endl;
  }
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