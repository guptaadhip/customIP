#pragma once
#include <netinet/in.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

/* Ethernet addresses are 6 bytes */
static const int ETHERNET_ADDRESS_LEN	= 6;
/* Ethernet header is 14 bytes */
static const int ETHERNET_HEADER_LEN = 14;

static const int IP_HEADER_LEN = 20;
static const int ICMP_HEADER_LEN = 8;
static const int PING_PACKET_LEN = 64;

static const uint32_t INCREMENT_IP = 0x01000000;

/* IPs not to be added */
static const uint32_t LOOPBACK_IP = 0x0100007f; /* loopback */
static const uint32_t CONTROL_NW_IP = 0x0000A8C0; /* Deter control Nw IP */

enum class IcmpResponse : int {
	ECHO_REPLY = 0,
	DESTINATION_UNREACHABLE = 3,
	ECHO = 8,
	TIME_EXCEEDED = 11,
};

enum class RoutePriority : int {
	LOCAL = 0,
	ADDED = 5,
	CASCADED = 10,
};

/* Ethernet header */
struct ethernetHeader {
		u_char ethDst[ETHERNET_ADDRESS_LEN];
		u_char ethSrc[ETHERNET_ADDRESS_LEN];
		u_short ethType;
};

static const uint32_t ethTypeIp = 0x0800;
static const uint32_t ethTypeArp = 0x0806;
static const u_char icmpProto = 0x1;

/* IP header */
struct ipHeader {
  u_char ipVhl;
  u_char ipTos;
  u_short ipLen;
  u_short ipId;
  u_short ipOffset;
  static const uint32_t ipRf = 0x8000;		/* reserved fragment flag */
  static const uint32_t ipDf = 0x4000;		/* dont fragment flag */
  static const uint32_t ipMf = 0x2000;		/* more fragments flag */
  static const uint32_t ipOffmask = 0x1fff;	/* mask for fragmenting bits */
  u_char ipTtl;
  u_char ipProto;
  u_short ipChecksum;
  struct in_addr ipSrc,ipDst;
};

#define IP_HL(ip)		(((ip)->ipVhl) & 0x0f)
#define IP_V(ip)		(((ip)->ipVhl) >> 4)
#define IP_VHL(v, hl) (v << 4 | (0x0f & hl))

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

struct icmpHeader {
  u_char type;
  u_char code;
  u_short checksum;
  u_short identifier;
  u_short seqNum;
};

struct udpHeader {
	u_short	srcPort;		/* source port */
	u_short	dstPort;		/* destination port */
	u_short	len;			  /* udp length */
	u_short	checksum;		/* udp checksum */
};
