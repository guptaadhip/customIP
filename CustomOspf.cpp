#include "include/CustomOspf.h"
#include "include/RouteTable.h"
#include "include/RouteEntry.h"
#include "include/net.h"
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <functional>
#include <cstring>

/* to be removed */
#include <iostream>

CustomOspf::CustomOspf(RouteTable *routeTable) {
  routeTable_ = routeTable;
  getMyIpInfo();

  /* Creating the Internet domain socket */
  sendSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sendSocket_ < 0) {
    std::cout << "Custom OSPF: Error sendSocket creation failed" << std::endl;
    exit(1);
  }
}

/* lets do the OSPF */
void CustomOspf::start() {
  /* Receiver Thread */
  receiver_ = std::thread(std::bind(&CustomOspf::recvInfo,this));
  /* Initial Sender Thread */
  sender_ = std::thread(std::bind(&CustomOspf::sendInfo,this, rtr1));
  /* Initial Sender 2 Thread */
  sender2_ = std::thread(std::bind(&CustomOspf::sendInfo,this, rtr2));
}

void CustomOspf::getMyIpInfo() {
  struct ifaddrs *ifAddrStruct= NULL;
  struct ifaddrs *ifa = NULL;
  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family==AF_INET) { // check it is IP4
      uint32_t ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      if (ipAddr != LOOPBACK_IP && ((ipAddr & 0x0000ffff) != CONTROL_NW_IP)) {
        ipVector_.push_back(ipAddr);
      }
    }
  }
}

void CustomOspf::sendInfo(uint32_t addr) {
  /* This sleep is needed */
  sleep(5);
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
  uint32_t ospfType = (uint32_t) OspfMsgType::ADD; 
  bcopy(&ospfType, buffer, sizeof(uint32_t));
  int i = sizeof(uint32_t);
  uint32_t count = ipVector_.size();
  bcopy(&count, (buffer + i), sizeof(uint32_t));
  i = sizeof(uint32_t);
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

void CustomOspf::sendUpdate(char *buffer, uint32_t addr) {
  struct sockaddr_in serverAddr;
  struct hostent *server;
  socklen_t serverAddrLength;
  
  /* zeroing the address structures Good Practice*/
  bzero((char *) &serverAddr, sizeof(serverAddr));
  
  /* Set server address values */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = addr;
  serverAddr.sin_port = htons(OSPF_PORT);
  serverAddrLength = sizeof(serverAddr);

  auto rc = sendto(sendSocket_, buffer, sizeof(buffer), 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
  if (rc < 0) {  
    std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
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
    OspfMsgType ospfType;
    bcopy(buffer, &ospfType, sizeof(uint32_t));
    /* do the stuff for hello */
    if (ospfType == OspfMsgType::HELLO) {
      /* no need to go further */
      continue;
    }

    uint32_t count;
    bcopy((buffer + sizeof(uint32_t)), &count, sizeof(uint32_t));
    for (int idx = 1; idx <= count; idx++) {
      uint32_t networkAddr;
			RoutePriority priority;
      bcopy((buffer + (idx * sizeof(uint32_t))), &networkAddr, sizeof(uint32_t));
      std::cout << "Network Address: " << networkAddr << std::endl;
      if (ospfType == OspfMsgType::ADD || ospfType == OspfMsgType::CASCADED_ADD) {
				priority = RoutePriority::ADDED;
				if(ospfType == OspfMsgType::CASCADED_ADD) priority = RoutePriority::CASCADED;
				
        auto localRoute = routeTable_->search((clientAddr.sin_addr.s_addr & 0x00FFFFFF));
        RouteEntry route(networkAddr, clientAddr.sin_addr.s_addr, 0x00FFFFFF, 
                                                   localRoute->getInterface(),
																										priority);
        routeTable_->insert(route);
        if (ospfType == OspfMsgType::ADD) {
          uint32_t temp = (uint32_t) OspfMsgType::CASCADED_ADD;
          /* update the type */
          bcopy(&temp, buffer, sizeof(uint32_t));
          /* send on the interface from where the update was not received */
          if (clientAddr.sin_addr.s_addr == rtr1) {
            sendUpdate(buffer, rtr2);
          } else {
            sendUpdate(buffer, rtr1);
          }
        }
      } else {
        /* Delete */
        routeTable_->remove(networkAddr);
        if (ospfType == OspfMsgType::DELETE) {
          uint32_t temp = (uint32_t) OspfMsgType::CASCADED_DELETE;
          /* update the type */
          bcopy(&temp, buffer, sizeof(uint32_t));
          /* send on the interface from where the update was not received */
          if (clientAddr.sin_addr.s_addr == rtr1) {
            sendUpdate(buffer, rtr2);
          } else {
            sendUpdate(buffer, rtr1);
          }
        }
      }
    }
    routeTable_->printRouteTable();
  }
}

