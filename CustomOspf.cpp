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
#include <algorithm>

/* to be removed */
#include <iostream>

CustomOspf::CustomOspf(RouteTable *routeTable, uint32_t rtr1, uint32_t rtr2) 
  : rtr1_(rtr1),
    rtr2_(rtr2) {
  routeTable_ = routeTable;
  getMyIpInfo();


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

  while(true) {
    sleep(1);
    
    /* cant reach network lets increase the strike */
    if (neighborStatus_[addr] == false) {
        strikeCount++;
    } else if (strikeCount < 3) { 
        strikeCount = 0;
    }
    
    /* Zero the buffer everytime */
    bzero((char *) buffer, sizeof(buffer));
    
    /* Strike = 3! Executing delete sequence */ 
    if (strikeCount == 3 && neighborStatus_[addr] == false) {
        std::cout << "My link is down to addr: " << addr << std::endl;
	/* Time to send delete message */

	/* Step 1: Get all entries with addr as next hop 
	*  and also delete them.
	*/
        auto deleteEntries = routeTable_->removeEntries(addr);


        if (addr == rtr1_) { 
          /* Set server address values */
          serverAddr.sin_addr.s_addr = rtr2_;
          serverAddrLength = sizeof(serverAddr);
        } else {
          /* Set server address values */
          serverAddr.sin_addr.s_addr = rtr1_;
          serverAddrLength = sizeof(serverAddr);
        }
	
	/* Step 2 : Fill buffer with all the entries */	
  	ospfType = (uint32_t) OspfMsgType::DELETE; 
 	bcopy(&ospfType, buffer, sizeof(uint32_t));
  	int i = sizeof(uint32_t);
  	uint32_t count = deleteEntries.size();
  	bcopy(&count, (buffer + i), sizeof(uint32_t));
 	i += sizeof(uint32_t);
  	for(auto entry : deleteEntries) {
    	  uint32_t networkIP = entry.getNwAddress();
    	  bcopy(&networkIP, (buffer + i), sizeof(uint32_t));
    	  i += sizeof(uint32_t);
	}
        std::cout << "Printing Route Table After Del: " << std::endl;
        routeTable_->printRouteTable();
        std::cout << std::endl;
	/* Delete message's buffer ready. */ 
        auto rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
        if (rc < 0) {  
          std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
       } 
       neighborStatus_[addr] = false;
       continue;
    }
    
    /* Line came back up! We are back in the game */
    if (neighborStatus_[addr] == true && strikeCount >= 3) {
       std::cout << "My link is up to addr: " << addr << std::endl;
       strikeCount = 0;
       serverAddr.sin_addr.s_addr = addr;
       serverAddrLength = sizeof(serverAddr);

       bzero((char *) buffer, sizeof(buffer));
       auto localEntries = routeTable_->searchAll(RoutePriority::LOCAL);
       ospfType = (uint32_t) OspfMsgType::ADD; 
       bcopy(&ospfType, buffer, sizeof(uint32_t));
       int i = sizeof(uint32_t);
       uint32_t count = localEntries.size();
       bcopy(&count, (buffer + i), sizeof(uint32_t));
       i += sizeof(uint32_t);
       for(auto entry : localEntries) {
         std::cout << "Sending Local: " << entry.getNwAddress() << " :: " << (uint32_t) entry.getPriority() << std::endl;
         uint32_t networkIP = entry.getNwAddress();
    	 bcopy(&networkIP, (buffer + i), sizeof(uint32_t));
    	 i += sizeof(uint32_t);
       }
       auto rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
       if (rc < 0) {  
         std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
       }
       
       /* Send the local entries to the other router */
       if (addr == rtr1_) {
         serverAddr.sin_addr.s_addr = rtr2_;
         serverAddrLength = sizeof(serverAddr);
       } else {
         serverAddr.sin_addr.s_addr = rtr1_;
         serverAddrLength = sizeof(serverAddr);
       }
       rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
       if (rc < 0) {  
         std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
       }
       
       /* Send the previously known cascaded entries */
       serverAddr.sin_addr.s_addr = addr;
       serverAddrLength = sizeof(serverAddr);

       bzero((char *) buffer, sizeof(buffer));
       auto directEntries = routeTable_->searchAll(RoutePriority::DIRECT);
       ospfType = (uint32_t) OspfMsgType::CASCADED_ADD; 
       bcopy(&ospfType, buffer, sizeof(uint32_t));
       i = sizeof(uint32_t);
       count = directEntries.size();
       bcopy(&count, (buffer + i), sizeof(uint32_t));
       i += sizeof(uint32_t);
       for(auto entry : directEntries) {
         /* Adhip: Do not delete this */
         if (entry.getNextHop() == addr) {
           count = count - 1;
           bcopy(&count, (buffer + sizeof(uint32_t)), sizeof(uint32_t));
           continue;
         } 
         uint32_t networkIP = entry.getNwAddress();
         std::cout << "Sending Direct: " << entry.getNwAddress() << " :: " << (uint32_t) entry.getPriority() << std::endl;
    	 bcopy(&networkIP, (buffer + i), sizeof(uint32_t));
    	 i += sizeof(uint32_t);
       }
       rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
       if (rc < 0) {  
         std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
       }

       neighborStatus_[addr] = false;
       continue;	
    }
    
    
    /* HELLO MESSAGE : Prepare the hello message */
    serverAddr.sin_addr.s_addr = addr;
    serverAddrLength = sizeof(serverAddr);
    ospfType = (uint32_t) OspfMsgType::HELLO;
    bcopy(&ospfType, buffer, sizeof(uint32_t));
    auto rc = sendto(socketfd, buffer, BUFLEN, 0,
                      (struct sockaddr *) &serverAddr, serverAddrLength);
    if (rc < 0) {  
      std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
    } 
    neighborStatus_[addr] = false;
  }
}

void CustomOspf::recvInfo() {
  int yes = 1;
  struct sockaddr_in clientAddr, serverAddr;
  char buffer[BUFLEN];
  socklen_t clientAddrLength;
  struct sockaddr_in neighborAddr;
  socklen_t neighborAddrLength;
  int sendSocket;


  /* Creating the Internet domain socket */
  sendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sendSocket < 0) {
    std::cout << "Custom OSPF: Error sendSocket creation failed" << std::endl;
    exit(1);
  }
  
 
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
        auto localRoute = routeTable_->searchAll((clientAddr.sin_addr.s_addr & 0x00FFFFFF), RoutePriority::LOCAL);
        RouteEntry route(networkAddr, clientAddr.sin_addr.s_addr, 0x00FFFFFF, 
                                      localRoute[0].getInterface(), priority);
        routeTable_->insert(route);     
      } else {
        /* Delete */
        routeTable_->removeEntry(networkAddr, clientAddr.sin_addr.s_addr);
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
    }
    if (ospfType == OspfMsgType::ADD) {
      std::cout << "Sending cascaded update" << std::endl;
      
      /* zeroing the address structures Good Practice*/
      bzero((char *) &neighborAddr, sizeof(neighborAddr));
      
      neighborAddr.sin_family = AF_INET;
      neighborAddr.sin_port = htons(OSPF_PORT);

      if (clientAddr.sin_addr.s_addr == rtr1_) { 
        /* Set server address values */
        neighborAddr.sin_addr.s_addr = rtr2_;
        neighborAddrLength = sizeof(serverAddr);
      } else {
        /* Set server address values */
        neighborAddr.sin_addr.s_addr = rtr1_;
        neighborAddrLength = sizeof(serverAddr);
      }
      auto rc = sendto(sendSocket, buffer, BUFLEN, 0,
                      (struct sockaddr *) &neighborAddr, neighborAddrLength);
      if (rc < 0) {  
        std::cout << "Custom OSPF: Error Sending Update Data" << std::endl;
      } 
    }
  }
}
