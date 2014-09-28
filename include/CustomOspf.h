#pragma once
#include <vector>

#define OSPF_PORT 5000   //The port on which to listen for incoming data
#define BUFLEN 100  //Max length of buffer

class CustomOspf {
 public:
  CustomOspf();
  void start();
  void recvInfo();
  void sendInfo();

 private:
  void getMyIpInfo();
  std::vector<uint32_t> ipVector_;
};