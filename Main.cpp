#include "include/RouteTable.h"
#include "include/Sniffer.h"
#include "include/net.h"
#include "include/MyIp.h"
#include "include/PacketEngine.h"
#include "include/NetworkHandler.h"
#include "include/CustomOspf.h"

#include <sys/socket.h>
#include <iostream>
#include <functional>
#include <thread>

using namespace std;

/* global Route table */
RouteTable routeTable;
/* Self Ip address */
MyIp myIps;
PacketEngine packetEngine;

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

/* Thread Spawned to sniff on all interfaces */
void startSniffing(std::string device){
  Sniffer sniff(device);
  sniff.registerCallback(callbackHandler);
}

int main(int argv, char *args[]) {
  std::vector<std::thread> threads;
  uint32_t rtr1, rtr2;

  packetEngine.setMyIps(myIps);
  routeTable.addMyRoutes(myIps.getMyIps());
  /* get local network */
  NetworkHandler networkHandler(&myIps,&packetEngine);
	//routeTable.printRouteTable();
	
  /* OSPF needs to only start if the router has any neighbor routers */
  if (argv > 1) {
    /* get non-local networks */
    if (argv == 2) {
      rtr1 = (uint32_t) atoi(args[1]);
    } else if (argv == 3) {
      rtr1 = (uint32_t) atoi(args[1]);
      rtr2 = (uint32_t) atoi(args[2]);
    } else {
      cout << "Max number of Neighbors can be 2" << endl;
      exit(1);
    }
    CustomOspf ospf(&routeTable, rtr1, rtr2); 
    ospf.start();
    /* this needs to be here or else ospf will go out of scope and core dump */
    for(auto entry : myIps.getMyIps()){
      threads.push_back(std::thread(startSniffing,entry.second));
    }
  
    for (auto& joinThreads : threads) joinThreads.join();
  }

  //Sniffer sniff("eth0");
  //sniff.registerCallback(callbackHandler);
	
  for(auto entry : myIps.getMyIps()){
    threads.push_back(std::thread(startSniffing,entry.second));
  }
  
  for (auto& joinThreads : threads) joinThreads.join();
  return 0;
}
