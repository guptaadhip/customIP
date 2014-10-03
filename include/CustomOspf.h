#pragma once
#include <vector>
#include <cstdint>
#include <thread>
#include <unordered_map>

class RouteTable;

#define OSPF_PORT 5000   //The port on which to listen for incoming data
#define BUFLEN 1024  //Max length of buffer

static const uint32_t rtr1 = 0x0401A8C0; /* Anuj: 192.168.1.8 */
static const uint32_t rtr2 = 0x1401A8C0; /* Adhips VM: 192.168.1.20 */

static std::unordered_map<uint32_t, bool> neighborStatus_;

enum class OspfMsgType : uint32_t {
  HELLO = 2,
  ADD = 4,
  DELETE = 8,
  CASCADED_ADD = 16,
  CASCADED_DELETE = 20,
};

class CustomOspf {
 public:
  CustomOspf(RouteTable *routeTable);
  void start();
  void recvInfo();
  void sendInfo(uint32_t addr);
  void sendUpdate(char *buffer, uint32_t addr);

 private:
  void getMyIpInfo();
  bool getNeighborStatus(uint32_t addr);
  std::vector<uint32_t> ipVector_;
  RouteTable *routeTable_;
  std::thread receiver_;
  std::thread sender_;
  std::thread sender2_;
  int sendSocket_;
};
