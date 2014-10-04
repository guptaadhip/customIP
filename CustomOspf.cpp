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

CustomOspf::CustomOspf(RouteTable *routeTable, uint32_t rtr1, uint32_t rtr2) 
  : rtr1_(rtr1),
    rtr2_(rtr2) {
  routeTable_ = routeTable;
  getMyIpInfo();

  /* Creating the Internet domain socket */
  sendSocket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sendSocket_ < 0) {
    std::cout << "Custom OSPF: Error sendSocket creation failed" << std::endl;
    exit(1);
  }

  /* Initialzing neighbour status */
  neighborStatus_[rtr1_] = false;
  neighborStatus_[rtr2_] = false; 
}

/* lets do the OSPF */
void CustomOspf::start() {
  /* Receiver Thread */
  receiver_ = std::thread(std::bind(&CustomOspf::recvInfo,this));
  /* Initial Sender Thread */
  sender_ = std::thread(std::bind(&CustomOspf::sendInfo,this, rtr1_));
  /* if rtr 2 is 0 then only one neighbor was passed */
  if (rtr2_ != 0) {
    /* Initial Sender 2 Thread */
    sender2_ = std::thread(std::bind(&CustomOspf::sendInfo,this, rtr2_));
  }
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
  struct sockaddr_in serverAddr;
  socklen_t serverAddrLength;
  int strikeCount = 0;
  char buffer[BUFLEN];
  uint32_t ospfType;
  int routePriority;
  int socketfd;

  /* Creating the Internet domain socket */
  socketfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (socketfd < 0) {
    std::cout << "Custom OSPF: Error sendInfo Socket creation failed" << std::endl;
    exit(1);
  }

  /* zeroing the address structures Good Practice*/
  bzero((char *) &serverAddr, sizeof(serverAddr));
  /* Set server address values */
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = addr;
  serverAddr.sin_port = htons(OSPF_PORT);
  serverAddrLength = sizeof(serverAddr);

  /* get the data to send from user */
  bzero((char *) buffer, sizeof(buffer));
  /* INITIAL MSG -> fill the buffer with the values. */
  ospfType = (uint32_t) OspfMsgType::ADD; 
  bcopy(&ospfType, buffer, sizeof(uint32_t));
  int i = sizeof(uint32_t);
  uint32_t count = ipVector_.size();
  bcopy(&count, (buffer + i), sizeof(uint32_t));
  i += sizeof(uint32_t);
  for(auto ipAddr : ipVector_) {
    uint32_t networkIP = ipAddr & 0x00ffffff;
    bcopy(&networkIP, (buffer + i), sizeof(uint32_t));
    i += sizeof(uint32_t);
  }
  /* Initial message ready -> fill done lets send */

  auto rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
  if (rc < 0) {  
    std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
  } 

  return;
  while(true) {
    sleep(1);
    
    if(neighborStatus_[addr] == false)
      strikeCount++;    // false status, so increase strikeCount
    else if (strikeCount < 3)
      strikeCount = 0;

    if(neighborStatus_[addr] == true && strikeCount > 3) {
      /* Link came back up after going down.
      *  ADD MESSAGE : Prepare the link back up message. */
      ospfType = (uint32_t) OspfMsgType::ADD; 
      bcopy(&ospfType, buffer, sizeof(uint32_t));
      int idx = (sizeof(uint32_t) * 2);
      uint32_t addCount = 0;

      /* Sending non-cascaded entries */
      
      /* adding the "local" entries */
      uint32_t localNwAddr;
      routePriority = (int) RoutePriority::LOCAL;
      auto localEntries = routeTable_->searchAll(routePriority);
      for(auto localEntry : localEntries) {
        localNwAddr = localEntry.getNwAddress();
        bcopy(&localNwAddr, (buffer + idx), sizeof(uint32_t)); 
        addCount++;
        idx += sizeof(uint32_t);
      }

      /* adding the "added" entries */
      uint32_t addedNwAddr;
      routePriority = (int) RoutePriority::DIRECT;
      auto addedEntries = routeTable_->searchAll(routePriority);
      for(auto addedEntry : addedEntries) {
        addedNwAddr = addedEntry.getNwAddress();
        bcopy(&addedNwAddr, (buffer + idx), sizeof(uint32_t)); 
        addCount++;
        idx += sizeof(uint32_t);
      }
      strikeCount = 0;

      /* Calculated the count. Now putting it in buffer */
      bcopy(&addCount, (buffer + sizeof(uint32_t)), sizeof(uint32_t));
      sendUpdate(buffer, addr);

    } else if(strikeCount == 3) {
      /* DELETE MESSAGE : Prepare the link down message */
      ospfType = (uint32_t) OspfMsgType::DELETE;
      bcopy(&ospfType, buffer, sizeof(uint32_t));
      int idx = sizeof(uint32_t);
      uint32_t deleteCount = 0;

      /* Deleting entries with addr as nextHop */
      auto nextHopInvalidEntries = routeTable_->searchAll(addr);
      uint32_t nwAddr;
      for(auto entry : nextHopInvalidEntries) {
        nwAddr = entry.getNwAddress();
        bcopy(&nwAddr, (buffer + idx), sizeof(uint32_t)); 
        deleteCount++;
        idx += sizeof(uint32_t);
      }
      /* Deleting all entries with addr as next hop. */
      routeTable_->removeEntry(addr);

      /* Deleting entry with addr as nwAddress and 0.0.0.0 as next Hop */
      auto addrDirectInvalidEntries = routeTable_->searchAllNWAddress(addr);
      uint32_t directAddr;
      for(auto directEntry : addrDirectInvalidEntries) {
        if(directEntry.getNextHop() == 0x00000000) {
          directAddr = directEntry.getNwAddress();
          bcopy(&directAddr, (buffer + idx), sizeof(uint32_t));
          deleteCount++;
          routeTable_->removeEntry(directEntry.getNwAddress(), directEntry.getNextHop());
          break;
        }
      }

      /* Calculated the count. Now putting it in buffer */
      bcopy(&deleteCount, (buffer + sizeof(uint32_t)), sizeof(uint32_t));

      /* now sending the delete message */
      if(addr == rtr1_)
        sendUpdate(buffer, rtr2_);
      else
        sendUpdate(buffer, rtr1_);
    } else {
      /* HELLO MESSAGE : Prepare the hello message */
      ospfType = (uint32_t) OspfMsgType::HELLO;
      bcopy(&ospfType, buffer, sizeof(uint32_t));
      sendUpdate(buffer, addr);
    }
    neighborStatus_[addr] = false;
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

  auto rc = sendto(sendSocket_, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
  if (rc < 0) {  
    std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
  } 
}

void CustomOspf::recvInfo() {
  int yes = 1;
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
  
  if ( setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1 ) {
    std::cout << "Custom OSPF: Error setting socket option" << std::endl;
    exit(1);
  }

  if (bind(socketFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
    std::cout << "Custom OSPF: Error binding to socket" << std::endl;
    exit(1);
  }

  clientAddrLength = sizeof(clientAddr);
  while (true) {
    /* receive any UDP packets on the port */
    auto rc = recvfrom(socketFd, buffer, BUFLEN, 0, (struct sockaddr *)&clientAddr, 
                                                            &clientAddrLength);
    if (rc < 0) {
      std::cout << "Custom OSPF: Error Receiving Data" << std::endl;
      close(socketFd);
      exit(0);
    }

    /* I just received a message -> So update the status to true */
    neighborStatus_[clientAddr.sin_addr.s_addr] = true;
    /* TBD: Remove */
    std::cout << "Got a message from: " << clientAddr.sin_addr.s_addr << std::endl;
    OspfMsgType ospfType;
    bcopy(buffer, &ospfType, sizeof(uint32_t));
    /* do the stuff for hello */
    if (ospfType == OspfMsgType::HELLO) {
      /* no need to go further */
      continue;
    }

    uint32_t count;
    bcopy((buffer + sizeof(uint32_t)), &count, sizeof(uint32_t));
    uint32_t bufPtr = sizeof(uint32_t) + sizeof(uint32_t);
    for (int idx = 1; idx <= count; idx++) {
      uint32_t networkAddr;
      RoutePriority priority;
      bcopy((buffer + bufPtr), &networkAddr, sizeof(uint32_t));
      std::cout << "Network Address: " << networkAddr << std::endl;
      if (ospfType == OspfMsgType::ADD || ospfType == OspfMsgType::CASCADED_ADD) {
				if(ospfType == OspfMsgType::CASCADED_ADD) {
          priority = RoutePriority::CASCADED;
        } else {
          priority = RoutePriority::DIRECT;
        }
				
        auto localRoute = routeTable_->search((clientAddr.sin_addr.s_addr & 0x00FFFFFF));
        RouteEntry route(networkAddr, clientAddr.sin_addr.s_addr, 0x00FFFFFF, 
                                      localRoute[0].getInterface(), priority);
        routeTable_->insert(route);     
      } else {
				if(ospfType == OspfMsgType::CASCADED_DELETE) {
          priority = RoutePriority::CASCADED;
        } else {
          priority = RoutePriority::DIRECT;
        }
        /* Delete */
        routeTable_->removeEntry(networkAddr, clientAddr.sin_addr.s_addr, priority);
      }
      bufPtr = bufPtr + sizeof(uint32_t);
    }
    std::cout << "Printing Route Table: " << std::endl;
    routeTable_->printRouteTable();
    std::cout << std::endl;
    
    if (ospfType == OspfMsgType::ADD) {
      std::cout << "Sending cascaded Add" << std::endl;
      uint32_t temp = (uint32_t) OspfMsgType::CASCADED_ADD;
      bcopy(&temp, buffer, sizeof(uint32_t));
    } else if (ospfType == OspfMsgType::DELETE) {
      std::cout << "Sending cascaded Delete" << std::endl;
      uint32_t temp = (uint32_t) OspfMsgType::CASCADED_DELETE;
      bcopy(&temp, buffer, sizeof(uint32_t));
    }
    if (ospfType == OspfMsgType::ADD || ospfType == OspfMsgType::DELETE) {
      std::cout << "Sending cascaded update" << std::endl;
      if (clientAddr.sin_addr.s_addr == rtr1_) {
        sendUpdate(buffer, rtr2_);
      } else {
        sendUpdate(buffer, rtr1_);
      }
    }
  }
}
