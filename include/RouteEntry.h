#pragma once
#include <cstdint>
#include <string>
#include "net.h"

/* Ip Table Entry Object */
class RouteEntry {
 public:
  RouteEntry(std::uint32_t nwAddress, std::uint32_t nextHop,
                                        std::uint32_t subnetMask,
                                          std::string interface,
                                            RoutePriority priority)
    : nwAddress_(nwAddress),
      subnetMask_(subnetMask),
      interface_(interface),
      nextHop_(nextHop),
      priority_(priority){
  }
  
  RouteEntry() {
    nwAddress_ = 0;
    subnetMask_ = 0;
    interface_ = nullptr;
    nextHop_ = 0;
    priority_ = RoutePriority::LOCAL;
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
  std::string getInterface() const {
    return interface_;
  }

  /* Returns the nextHop */
  std::uint32_t getNextHop() const {
    return nextHop_;
  }

  /* Returns the priority */
  RoutePriority getPriority() const {
    return priority_;
  }
  
private:
  std::uint32_t nwAddress_;
  std::uint32_t nextHop_;
  std::uint32_t subnetMask_;
  std::string interface_;
  RoutePriority priority_;
};