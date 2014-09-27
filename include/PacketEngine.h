#pragma once
#include "net.h"

/* Build Forward IP Packet */
class PacketEngine {
 public:
 	PacketEngine();
  void forwardPacket(const u_char *packet, uint32_t nextHopAddr);
	void responsePacket(const u_char *packet, IcmpResponse type);
	
 private:
  unsigned short checksum(unsigned short *addr, int len);
  int socketFd_;
};