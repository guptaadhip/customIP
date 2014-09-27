#include "include/RouteTable.h"
#include "include/Sniffer.h"
#include "include/net.h"
#include "include/MyIp.h"
#include "include/PacketProducer.h"

#include <sys/socket.h>
#include <iostream>

using namespace std;


/* global Route table */
RouteTable routeTable;
/* Self Ip address */
MyIp myIps;
PacketProducer packetEngine;

/* This is the ICMP Handler if the dst address is of self*/
void icmpHandler(struct icmpHeader *icmp) {
  int icmpType = (int) icmp->type;
  switch (icmpType) {
    case 8:
      /* Set type to 8 and respond back */
      icmp->type = 0;
      break;
    case 0:
      cout << "Got the value" << endl;
      break;
  }
}

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
        icmpHandler(icmp);
        cout << "Made Request Need to send" << endl;
      }
      /*cout << "Source Host: " << inet_ntoa(ip->ipSrc)
           << " Destination Host: " << inet_ntoa(ip->ipDst)
           << " Type: "  << (int) icmp->type
           << " Code: "  << (int) icmp->code
           << " Identifier: "  << ntohs(icmp->identifier)
           << " seqNum: "  << ntohs(icmp->seqNum)
           << " Ip Header len: " << ipHeaderLen
           << std::endl;*/
    }else { // Packet should be forwarded
			packetEngine.ForwardPacket(packet);
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
  Sniffer sniff;
  sniff.registerCallback(callbackHandler);
  return 0;
}