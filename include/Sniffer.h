#pragma once
#include <pcap.h>

/* Ip Table Entry Object */
class Sniffer {
 public:
  Sniffer();
  
  /* register the Callback handler function */
  void registerCallback(void (*func)(u_char *args,
                                     const struct pcap_pkthdr* pkthdr,
                                     const u_char* packet));
 private:
  void init();
  pcap_t* descr_;
};