#pragma once
#include "net.h"
#include <unordered_map>
#include <string>
#include <ifaddrs.h>

/* Ip Table Entry Object */
class MyIp {
 public:
  MyIp();
  bool isDestinedToMe(struct ipHeader *ip);
  std::string getInterface(struct ipHeader *ip);
  std::unordered_map<uint32_t, std::string> getMyIps() const;
  void printIfAddr();

 private:
  void init();
  std::unordered_map<uint32_t, std::string> myIp_;
  struct ifaddrs *ifAddrStruct_= NULL;
};