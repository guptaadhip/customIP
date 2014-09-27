#include "include/MyIp.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

using namespace std;

MyIp::MyIp() {
  /* Find all of my ips */
  init();
}

/* find my ips and set it in the list */
void MyIp::init() {
	struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
  	if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
  		uint32_t ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
  		std::string interface = string(ifa->ifa_name);
  		myIp_.insert(std::make_pair(ipAddr, interface));
    }
  }
}

bool MyIp::isDestinedToMe(struct ipHeader *ip) {
	std::unordered_map<uint32_t, std::string>::iterator match = myIp_.find(ip->ipDst.s_addr);
	if (match != myIp_.end()) {
		return true;
	}
	return false;
}

/* Returns -1 when address not found */
std::string MyIp::getInterface(struct ipHeader *ip) {
	std::unordered_map<uint32_t, std::string>::iterator match = myIp_.find(ip->ipSrc.s_addr);
	if (match != myIp_.end()) {
		return match->second;
	}
	return nullptr;
}