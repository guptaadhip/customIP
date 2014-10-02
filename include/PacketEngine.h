#pragma once
#include "net.h"
#include "MyIp.h"

/* Build Forward IP Packet */
class PacketEngine {
 public:
 	PacketEngine();
  void forwardPacket(const u_char *packet, uint32_t nextHopAddr);
	void responsePacket(const u_char *packet, IcmpResponse type);
	void sendPing(const uint32_t srcAddr, const uint32_t dstAddr);
	void setMyIps(MyIp);
	
 private:
  unsigned short checksum(unsigned short *addr, int len);
  int socketFd_;
	MyIp myIps_;
};