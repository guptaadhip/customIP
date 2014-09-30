#pragma once
#include <pcap.h>
#include <string>

/* Ip Table Entry Object */
class Sniffer {
 public:
  Sniffer(std::string device);
  
  /* register the Callback handler function */
  void registerCallback(void (*func)(u_char *args,
                                     const struct pcap_pkthdr* pkthdr,
                                     const u_char* packet));
 private:
  void init();
  pcap_t* descr_;
  std::string device_;
};
