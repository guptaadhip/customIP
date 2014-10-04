#include "include/PacketEngine.h"
#include "include/net.h"
#include "include/MyIp.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>

using namespace std;

PacketEngine::PacketEngine() {
	const int on = 1;
	if ((socketFd_ = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) {
		cout << "Socket creation failed!" << endl;
		exit(1);
	}

	/*
   *  IP_HDRINCL must be set on the socket so that
   *  the kernel does not attempt to automatically add
   *  a default ip header to the packet
   */
	if (setsockopt(socketFd_, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		cout << "Setting the socket option failed!" << endl;
    exit(1);
  }
}

void PacketEngine::setMyIps(MyIp myIps){
	this->myIps_ = myIps;
}

void PacketEngine::forwardPacket(const u_char *packet, uint32_t nextHopAddr) {
	struct ipHeader *ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
	struct sockaddr_in dest;
	u_short ipLen = ntohs(ip->ipLen);

	/* lets first check the TTL if 0 no point proceeding */
	ip->ipTtl = ip->ipTtl - 1; // decrease TTL
	if (ip->ipTtl == 0) {
		/* TBD: need to send what kind of a response should be sent */
		responsePacket(packet, IcmpResponse::TIME_EXCEEDED);
		return;
 	}

 	ip->ipChecksum = 0;
  ip->ipChecksum = checksum ((uint16_t *) ip, IP_HEADER_LEN);

	dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = nextHopAddr;

	/*
   *  now the packet is sent
   */
  auto rc = sendto(socketFd_, packet + ETHERNET_HEADER_LEN, ipLen, 0, 
  											 (struct sockaddr *)&dest, sizeof(struct sockaddr));
  if (rc < 0) {
  	cout << "Forwarding Packet Failed!" << endl;
    exit(1);
  }

}

void PacketEngine::responsePacket(const u_char *packet, IcmpResponse type) {
	struct ipHeader *ip = (struct ipHeader *) (packet + ETHERNET_HEADER_LEN);
 	struct ipHeader *tempIp;
 	struct sockaddr_in dest;
 	u_short ipLen = ntohs(ip->ipLen);
	u_char *tempPacket = (u_char *) malloc ((sizeof(u_char) * (ip->ipLen)));

	/* Start IP Header Creation */
	tempIp = (struct ipHeader *) malloc (IP_HEADER_LEN);
  tempIp->ipVhl = IP_VHL(0x4, 0x5);
  tempIp->ipTos = 0x0;
  tempIp->ipLen = ipLen;
  tempIp->ipId = htons(random());
  tempIp->ipOffset = 0x0;
  tempIp->ipTtl = 64;
  tempIp->ipProto = icmpProto;
	if(type == IcmpResponse::DESTINATION_UNREACHABLE){
		tempIp->ipSrc.s_addr = this->myIps_.getMyIpUsingNetworkAddress
                                                ((ip->ipSrc.s_addr & SUBNET));
	}else{
		tempIp->ipSrc.s_addr = ip->ipDst.s_addr;
	}
  
	tempIp->ipDst.s_addr = ip->ipSrc.s_addr;
  
	tempIp->ipChecksum = 0;
  tempIp->ipChecksum = checksum ((uint16_t *) tempIp, IP_HEADER_LEN);
	/* End IP Header Creation */

  /* copy IP Header to Place */
  int datalen = ipLen - IP_HEADER_LEN - ICMP_HEADER_LEN;
	memcpy(tempPacket, tempIp, IP_HEADER_LEN);
	memcpy((tempPacket + IP_HEADER_LEN), 
                        packet + ETHERNET_HEADER_LEN + IP_HEADER_LEN,
                                          datalen + ICMP_HEADER_LEN);
	/* Lets do the ICMP */
	struct icmpHeader *icmp;
	icmp = (struct icmpHeader *) (tempPacket + IP_HEADER_LEN);
	icmp->type = (uint8_t) type;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->checksum = checksum((uint16_t *) (tempPacket + IP_HEADER_LEN),
                                          ICMP_HEADER_LEN + datalen);

	/* Lets send the packet */
	
  memset(&dest, 0, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = tempIp->ipDst.s_addr;
  
  /*
   *  now the packet is sent
   */
  auto rc = sendto(socketFd_, tempPacket, ipLen, 0, (struct sockaddr *)&dest,
                   sizeof(struct sockaddr));
  if (rc < 0) {
  	cout << "Sending the ICMP Response failed!!" << endl;
    exit(1);
  }
}

void PacketEngine::sendPing(const uint32_t srcAddr, const uint32_t dstAddr) {
	u_char *pingPacket = (u_char *) malloc ((sizeof(u_char) * PING_PACKET_LEN));
 	struct ipHeader *tempIp;
 	struct sockaddr_in dest;
	
	/* Start IP Header Creation */
	tempIp = (struct ipHeader *) malloc (IP_HEADER_LEN);
  tempIp->ipVhl = IP_VHL(0x4, 0x5);
  tempIp->ipTos = 0x0;
  tempIp->ipLen = PING_PACKET_LEN;
  tempIp->ipId = htons(random());
  tempIp->ipOffset = 0x0;
  tempIp->ipTtl = 64;
  tempIp->ipProto = icmpProto;
  tempIp->ipSrc.s_addr = srcAddr;
  tempIp->ipDst.s_addr = dstAddr;
  tempIp->ipChecksum = 0;
  tempIp->ipChecksum = checksum ((uint16_t *) tempIp, IP_HEADER_LEN);
	/* End IP Header Creation */

  /* copy IP Header to Place */
  int datalen = PING_PACKET_LEN - IP_HEADER_LEN - ICMP_HEADER_LEN;
	memcpy(pingPacket, tempIp, IP_HEADER_LEN);

	/* Lets do the ICMP */
	struct icmpHeader *icmp;
	icmp = (struct icmpHeader *) (pingPacket + IP_HEADER_LEN);
	icmp->type = (uint8_t) IcmpResponse::ECHO;
	icmp->code = 0;
	icmp->checksum = 0;
	icmp->checksum = checksum((uint16_t *) (pingPacket + IP_HEADER_LEN),
                                          ICMP_HEADER_LEN + datalen);

	/* Lets send the packet */
	
  memset(&dest, 0, sizeof(struct sockaddr_in));
	dest.sin_family = AF_INET;
  dest.sin_addr.s_addr = dstAddr;
  
  /*
   *  now the packet is sent
   */
  auto rc = sendto(socketFd_, pingPacket, PING_PACKET_LEN, 0, 
  								(struct sockaddr *)&dest, sizeof(struct sockaddr));
  if (rc < 0) {
  	cout << "Sending the ICMP Send failed!" << endl;
    exit(1);
  }
}

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