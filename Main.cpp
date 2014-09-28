#include "include/RouteTable.h"
#include "include/Sniffer.h"
#include "include/net.h"
#include "include/MyIp.h"
#include "include/PacketEngine.h"
#include "include/NetworkHandler.h"

#include <sys/socket.h>
#include <iostream>

using namespace std;


/* global Route table */
RouteTable routeTable;
/* Self Ip address */
MyIp myIps;
PacketEngine packetEngine;

static void callbackHandler(u_char *args, const struct pcap_pkthdr* pkthdr,
                                                        const u_char* packet) {
  u_int caplen = pkthdr->caplen; 
  u_int length = pkthdr->len;
  struct ethernetHeader *ethernet;
  struct ipHeader *ip;
  u_short ethernetType;
  uint32_t ipHeaderLen;
  ethernet = (struct ethernetHeader *) packet;
  ethernetType = ntohs(ethernet->ethType);
  if (ethernetType == ethTypeIp) {
    ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
    ipHeaderLen = IP_HL(ip) * 4;
    /* if the packets are destined to me */
    if (myIps.isDestinedToMe(ip) == true) {
      /* ICMP Packet Proto */
      if (ip->ipProto == icmpProto) {
        struct icmpHeader *icmp = (struct icmpHeader *) (packet + 
                                          ETHERNET_HEADER_LEN + ipHeaderLen);
        /* create the ICMP reply */
        packetEngine.responsePacket(packet, IcmpResponse::ECHO_REPLY);
        return;
      }
      
    } else { // Packet should be forwarded
      return;
      auto entry = routeTable.search(ip->ipDst.s_addr);
      if (entry == nullptr) {
        cout << "I Got called 2" << endl;
        packetEngine.responsePacket(packet, IcmpResponse::DESTINATION_UNREACHABLE);
        return;
      } else {
        cout << "I Got called 3" << endl;
        packetEngine.forwardPacket(packet, entry->getNextHop());
      }
		}
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
	NetworkHandler networkHandler(&myIps,&packetEngine);
  Sniffer sniff;
  sniff.registerCallback(callbackHandler);
  return 0;
}