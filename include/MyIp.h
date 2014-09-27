#pragma once
#include "net.h"
#include <unordered_map>
#include <string>

/* Ip Table Entry Object */
class MyIp {
 public:
  MyIp();
  bool isDestinedToMe(struct ipHeader *ip);
  std::string getInterface(struct ipHeader *ip);
  
 private:
  void init();
  std::unordered_map<uint32_t, std::string> myIp_;
};