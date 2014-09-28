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
        packetEngine.responsePacket(packet, IcmpResponse::DESTINATION_UNREACHABLE);
        return;
      } else {
        packetEngine.forwardPacket(packet, entry->getNextHop());
      }
		}
  }
}

int main() {
	NetworkHandler networkHandler(&myIps,&packetEngine);
  routeTable.addMyRoutes(myIps.getMyIps());
  Sniffer sniff;
  sniff.registerCallback(callbackHandler);
  return 0;
}