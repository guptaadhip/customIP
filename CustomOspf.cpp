#include "include/CustomOspf.h"
#include "include/RouteTable.h"
#include "include/net.h"
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ifaddrs.h>
#include <functional>
#include <cstring>
#include "include/RouteEntry.h"

/* to be removed */
#include <iostream>

CustomOspf::CustomOspf(RouteTable *routeTable) {
  routeTable_ = routeTable;
  getMyIpInfo();
}

/* lets do the OSPF */
void CustomOspf::start() {
  /* Receiver Thread */
  std::thread receiver (std::bind(&CustomOspf::recvInfo,this));
  /* sender runs after 5 seconds to make sure all others are up */
  sleep(5);
  /* Sender Thread */
  std::thread sender (std::bind(&CustomOspf::sendInfo,this, rtr1));
  /* Sender 2 Thread */
  std::thread sender2 (std::bind(&CustomOspf::sendInfo,this, rtr2));
  /* wait for the receiver to finish */
  receiver.join();
}

void CustomOspf::getMyIpInfo() {
  struct ifaddrs *ifAddrStruct= NULL;
  struct ifaddrs *ifa = NULL;
  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family==AF_INET) { // check it is IP4
      uint32_t ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      //if (ipAddr != LOOPBACK_IP && ((ipAddr & 0x0000ffff) != CONTROL_NW_IP)) {
      if (ipAddr != LOOPBACK_IP) {
        ipVector_.push_back(ipAddr);
      }
    }
  }
}

void CustomOspf::sendInfo(uint32_t addr) {
  int socketFd;
  struct sockaddr_in serverAddr;
  struct hostent *server;
  socklen_t serverAddrLength;
  char buffer[BUFLEN];

  /* zeroing the address structures Good Practice*/
  bzero((char *) &serverAddr, sizeof(serverAddr));
  
  /* Creating the Internet domain socket */
  socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socketFd < 0) {
    std::cout << "Custom OSPF: Error socket creation failed" << std::endl;
  }

  /* Set server address values */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = addr;
  serverAddr.sin_port = htons(OSPF_PORT);

  serverAddrLength = sizeof(serverAddr);

  /* get the data to send from user */
  bzero((char *) buffer, sizeof(buffer));

  /* fill the buffer with the values */
  uint32_t count = ipVector_.size();
  bcopy(&count, buffer, sizeof(uint32_t));
  int i = sizeof(uint32_t);
  for(auto ipAddr : ipVector_) {
    uint32_t networkIP = ipAddr & 0x00ffffff;
    bcopy(&networkIP, (buffer + i), sizeof(uint32_t));
    i += sizeof(uint32_t);
  }
  /* fill done lets send */
 
  auto rc = sendto(socketFd, buffer, sizeof(buffer), 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
  if (rc < 0) {  
    std::cout << "Custom OSPF: Error Sending Data" << std::endl;
  }
  
}

void CustomOspf::recvInfo() {
  struct sockaddr_in clientAddr, serverAddr;
  char buffer[BUFLEN];
  socklen_t clientAddrLength;
  
  /* Creating the Internet domain socket */
  auto socketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socketFd < 0) {
    std::cout << "Custom OSPF: Error socket creation failed" << std::endl;
    exit(0);
  }

  /* Set server address values */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(OSPF_PORT);
  if (bind(socketFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    std::cout << "Custom OSPF: Error binding to socket" << std::endl;
    exit(0);
  }

  clientAddrLength = sizeof(clientAddr);
  while (true) {
    /* receive any UDP packets on the port */
    auto rc = recvfrom(socketFd, buffer, 1024, 0, (struct sockaddr *)&clientAddr, 
                                                            &clientAddrLength);
    if (rc < 0) {
      std::cout << "Custom OSPF: Error Receiving Data" << std::endl;
      close(socketFd);
      exit(0);
    }
    uint32_t count;
    bcopy(buffer, &count, sizeof(uint32_t));
    for (int idx = 1; idx <= count; idx++) {
      uint32_t networkAddr;
      bcopy((buffer + (idx * sizeof(uint32_t))), &networkAddr, sizeof(uint32_t));
      /* print to be removed */
      std::cout << "Network Address: " << networkAddr << std::endl;
      RouteEntry route(networkAddr, clientAddr.sin_addr.s_addr, 0x00FFFFFF, "en0");
      routeTable_->insert(route);
    }
    routeTable_->printRouteTable();
  }
}

