#pragma once
#include "net.h"

/* Build Forward IP Packet */
class PacketEngine {
 public:
  void forwardPacket(const u_char *packet, uint32_t nextHopAddr);
	void responsePacket(const u_char *packet, enum IcmpResponse);
	
 private:
  unsigned short checksum(unsigned short *addr, int len);
};