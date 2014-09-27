#pragma once
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "net.h"

/* Build Forward IP Packet */
class PacketProducer {
 public:
  void ForwardPacket(const u_char *packet);
	void ResponsePacket(const u_char *packet);
	
 private:
  unsigned short IPChecksum(unsigned short *addr, int len);
};