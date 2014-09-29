#pragma once
#include <vector>
#include <cstdint>

class RouteTable;

#define OSPF_PORT 5000   //The port on which to listen for incoming data
#define BUFLEN 100  //Max length of buffer

static const uint32_t rtr1 = 0x0401A8C0; /* Anuj: 192.168.1.8 */
static const uint32_t rtr2 = 0x1401A8C0; /* Adhips VM: 192.168.1.20 */

class CustomOspf {
 public:
  CustomOspf(RouteTable *routeTable);
  void start();
  void recvInfo();
  void sendInfo(uint32_t addr);

 private:
  void getMyIpInfo();
  std::vector<uint32_t> ipVector_;
  RouteTable *routeTable_;
};
