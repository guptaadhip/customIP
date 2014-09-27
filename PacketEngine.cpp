#include "include/PacketEngine.h"
#include "include/net.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void PacketEngine::forwardPacket(const u_char *packet, uint32_t nextHopAddr) {
	struct ipHeader *ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);

	/* lets first check the TTL if 0 no point proceeding */
	ip->ipTtl = ip->ipTtl - 1; // decrease TTL
	if (ip->ipTtl == 0) {
		/* TBD: need to send what kind of a response should be sent */
		responsePacket(packet, IcmpResponse::TIME_EXCEEDED);
		return;
 	}

	/* TBD: reach here means that the packet needs to be forwarded */

}

void PacketEngine::responsePacket(const u_char *packet, enum IcmpResponse) {
	struct ipHeader *ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
 	struct in_addr tempAddr;

	ip->ipTtl = ip->ipTtl - 1; // decrease TTL
	if (ip->ipTtl == 0) {
		/* TBD: need to send what kind of a response should be sent */
		responsePacket(packet, IcmpResponse::TIME_EXCEEDED);
		return;
	}
	
	tempAddr = ip->ipDst;
	ip->ipDst = ip->ipSrc;
	ip->ipSrc = tempAddr;
	/* TBD: Entire IP Packet Logic */
	
	ip->ipTtl = 255; 
	ip->ipChecksum = checksum((unsigned short *)ip, sizeof(struct ipHeader));		

	// TBD -  Create ICMP Header
}

// TBD - Produce wrong checksum
unsigned short PacketEngine::checksum(unsigned short *addr, int len){ 
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