#include "include/CustomIp.h"
#include "include/Sniffer.h"
#include "include/net.h"
#include "include/NetworkHandler.h"
#include "include/CustomOspf.h"

#include <sys/socket.h>
#include <iostream>
#include <functional>
#include <thread>

using namespace std;



void callbackHandler(u_char *args, const struct pcap_pkthdr* pkthdr,
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
      auto entry = routeTable.search(ip->ipDst.s_addr);
      if (entry == nullptr) {
        packetEngine.responsePacket(packet,
                                    IcmpResponse::DESTINATION_UNREACHABLE);
        return;
      /*} else {
        packetEngine.forwardPacket(packet, entry->getNextHop());*/
      }
    }
  }
}

int main() {
  routeTable.addMyRoutes(myIps.getMyIps());
  /* get local network */
  NetworkHandler networkHandler;
  /* get non-local networks */
  CustomOspf ospf; 
  ospf.start();
  Sniffer sniff("en1");
  sniff.registerCallback(callbackHandler);
  return 0;
}
