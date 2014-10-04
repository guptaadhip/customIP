#pragma once
#include <vector>
#include <cstdint>
#include <thread>
#include <unordered_map>

class RouteTable;

#define OSPF_PORT 50000   //The port on which to listen for incoming data
#define BUFLEN 1024  //Max length of buffer



static std::unordered_map<uint32_t, bool> neighborStatus_;

enum class OspfMsgType : uint32_t {
  HELLO = 2,
  ADD = 4,
  DELETE = 8,
  CASCADED_ADD = 16,
  CASCADED_DELETE = 20,
  ADD_LINKUP = 24,
};

class CustomOspf {
 public:
  CustomOspf(RouteTable *routeTable, uint32_t, uint32_t);
  void start();
  void recvInfo();
  void sendInfo(uint32_t addr);
  void sendUpdate(char *buffer, uint32_t addr);

 private:
  uint32_t rtr1_; 
  uint32_t rtr2_; 

  void getMyIpInfo();
  bool getNeighborStatus(uint32_t addr);
  std::vector<uint32_t> ipVector_;
  RouteTable *routeTable_;
  std::thread receiver_;
  std::thread sender_;
  std::thread sender2_;
};
