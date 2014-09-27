#pragma once
#include <cstdint>

/* Ip Table Entry Object */
class RouteEntry {
 public:
  RouteEntry(std::uint32_t nwAddress,std::uint32_t nextHop,
                                        std::uint32_t subnetMask,int interface)
    : nwAddress_(nwAddress),
      subnetMask_(subnetMask),
      interface_(interface),
      nextHop_(nextHop){
  }
  
  RouteEntry() {
    nwAddress_ = 0;
    subnetMask_ = 0;
    interface_ = 0;
    nextHop_ = 0;
  }
  
  /* Returns the Network Address */
  std::uint32_t getNwAddress() const {
    return nwAddress_;
  }
  
  /* Returns the Subnet Mask */
  std::uint32_t getSubnetMask() const {
    return subnetMask_;
  }
  
  /* Returns the Interface */
  int getInterface() const {
    return interface_;
  }

 /* Returns the nextHop */
  std::uint32_t getNextHop() const {
    return nextHop_;
  }
  
private:
  std::uint32_t nwAddress_;
  std::uint32_t nextHop_;
  std::uint32_t subnetMask_;
  int interface_;
};