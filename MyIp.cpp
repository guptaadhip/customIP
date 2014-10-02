#include "include/MyIp.h"
#include <sys/types.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

/* to be removed */
#include <iostream>

using namespace std;

MyIp::MyIp() {
  /* Find all of my ips */
  init();
}

/* find my ips and set it in the list */
void MyIp::init() {
  struct ifaddrs * ifa=NULL;
  getifaddrs(&ifAddrStruct_);

  for (ifa = ifAddrStruct_; ifa != NULL; ifa = ifa->ifa_next) {
  	if (ifa->ifa_addr->sa_family==AF_INET) { // check it is IP4
  		uint32_t ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
  		//if (ipAddr != LOOPBACK_IP && ((ipAddr & 0x0000ffff) != CONTROL_NW_IP)) {
  		//if (ipAddr != LOOPBACK_IP) { // This is for the local systems only
  			std::string interface = string(ifa->ifa_name);
  			myIp_.insert(std::make_pair(ipAddr, interface));
  		//}
    }
  }
}

bool MyIp::isDestinedToMe(struct ipHeader *ip) {
	std::unordered_map<uint32_t, std::string>::iterator match =
                                                  myIp_.find(ip->ipDst.s_addr);
	if (match != myIp_.end()) {
		return true;
	}
	return false;
}

/* Returns -1 when address not found */
std::string MyIp::getInterface(struct ipHeader *ip) {
	std::unordered_map<uint32_t, std::string>::iterator match =
                                                  myIp_.find(ip->ipSrc.s_addr);
	if (match != myIp_.end()) {
		return match->second;
	}
	return nullptr;
}

/* Return my ip addresses */
std::unordered_map<uint32_t, std::string> MyIp::getMyIps() const {
	return myIp_;
}

/* Return my ip address based on interface */
uint32_t MyIp::getMyIp(std::string interface){
	for(auto element : myIp_){
		if(interface.compare(element.second) == 0){
			return element.first;
		}
	}
	return 0;
}

/* Return my interface based on ip address*/
std::string MyIp::getMyIp(uint32_t address){
	for(auto element : myIp_){
		if(element.first == address){
			return element.second;
		}
	}
	return "";
}

/* Return my ip address based on interface */
uint32_t MyIp::getMyIpUsingNetworkAddress(uint32_t networkAddress){
	for(auto element : myIp_){
		if((element.first & SUBNET) == networkAddress){
			return element.first;
		}
	}
	return 0;
}

/* to be remove just for debugging reasons */
void MyIp::printIfAddr() {
	struct ifaddrs * ifa=NULL;
	for (ifa = ifAddrStruct_; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
			struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
			char *addr = inet_ntoa(sa->sin_addr);
			cout << "ifa_name: " << ifa->ifa_name << "\t::\t ifa_flags: " 
			<< ifa->ifa_flags << "\t::\t ifa_addr: " << addr << " " 
			<< ntohl(sa->sin_addr.s_addr) << endl;
		}
	}
}
