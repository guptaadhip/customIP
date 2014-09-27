#include "include/PacketProducer.h"
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

using namespace std;

void PacketProducer::ForwardPacket(const u_char *packet){
	struct ipHeader *ipHead = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
	int destinationReachable = 1;
	
	//Check if destination is reachable
	if(routeTable.search(ipHead->ipDst) == nullptr){
		destinationReachable = 0;
	}
	
	if(destinationReachable == 1){
		if((ipHead->ipTtl -1) == 0){ // TBD - Packet outlived its time, send an ICMP message to source, call icmpHandler
		}
		
		ipHead->ipTtl = ipHead->ipTtl - 1; // decrease TTL
		ipHead->ipChecksum = IPChecksum((unsigned short *)ipHead, sizeof(struct ipHeader));	//Update Checksum;
	}else if(destinationReachable == 0){ // TBD - Destination Host unreachable, notify the Source , send ICMP message
	}
}

void PacketProducer::ResponsePacket(const u_char *packet){
	struct ipHeader *ipHead = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
	struct in_addr tempAddr;
	
	tempAddr = ipHead->ipDst;
	ipHead->ipDst = ipHead->ipSrc;
	ipHead->ipSrc = tempAddr;
	
		if((ipHead->ipTtl -1) == 0){ // TBD - Packet outlived its time, send an ICMP message to source, call icmpHandler
		}
		
		ipHead->ipTtl = 255; // decrease TTL
		ipHead->ipChecksum = IPChecksum((unsigned short *)ipHead, sizeof(struct ipHeader));	//Update Checksum;
		
		// TBD -  Create ICMP Header
}

unsigned short PacketProducer::IPChecksum(unsigned short *addr, int len){ // TBD - Produce wrong checksum
	int nleft = len;
  int sum = 0;
  unsigned short *w = addr;
  unsigned short answer = 0;
  
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  
  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}