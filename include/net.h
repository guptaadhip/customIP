#pragma once
#include <netinet/in.h>
#include <net/ethernet.h>
#include <arpa/inet.h>


/* Ethernet addresses are 6 bytes */
static const int ETHERNET_ADDRESS_LEN	= 6;
/* Ethernet header is 14 bytes */
static const int ETHERNET_HEADER_LEN = 14;

/* Ethernet header */
struct ethernetHeader {
		u_char ethDst[ETHERNET_ADDRESS_LEN];
		u_char ethSrc[ETHERNET_ADDRESS_LEN];
		u_short ethType;
};

static const uint32_t ethTypeIp = 0x0800;
static const uint32_t ethTypeArp = 0x0806;

/* IP header */
struct ipHeader {
  static const uint32_t IP_RF = 0x8000;		/* reserved fragment flag */
  static const uint32_t IP_DF = 0x4000;		/* dont fragment flag */
  static const uint32_t IP_MF = 0x2000;		/* more fragments flag */
  static const uint32_t IP_OFFMASK = 0x1fff;	/* mask for fragmenting bits */
  u_char ipVhl;
  u_char ipTos;
  u_short ipLen;
  u_short ipId;
  u_short ipOffset;
  u_char ipTtl;
  u_char ipProto;
  u_short ipChecksum;
  struct in_addr ipSrc,ipDst;
};

#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
struct tcpHeader {
  u_short srcPort;
	u_short dstPort;
  u_int seqNum;
	u_int ack;
	u_char offset;
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
		u_char th_flags;
  static const u_short FIN = 0x01;
  static const u_short SYN = 0x02;
  static const u_short RST = 0x04;
  static const u_short PUSH = 0x08;
  static const u_short ACK = 0x10;
  static const u_short URG = 0x20;
  static const u_short ECE = 0x40;
  static const u_short CWR = 0x80;
#define FLAGS (FIN|SYN|RST|ACK|URG|ECE|CWR)
  u_short window;
  u_short checksum;
	u_short urgPointer;
};