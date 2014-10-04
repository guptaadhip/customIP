#include "include/Sniffer.h"
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <cstring>

using namespace std;

Sniffer::Sniffer(string device) 
  : device_(device) {
  init();
}

void Sniffer::init() {
  char *device = (char *) device_.c_str();
  char errbuf[PCAP_ERRBUF_SIZE];
  const u_char *packet;
  struct bpf_program filter;        /* hold compiled program */
  bpf_u_int32 mask;            /* subnet mask */
  bpf_u_int32 networkAddress;             /* ip */
  
  /* get the default device */
  if (device == NULL) {
    cout << errbuf << endl;
    exit(1);
  }
  pcap_lookupnet(device, &networkAddress, &mask, errbuf);
  /* Promiscuous Mode */
  descr_ = pcap_open_live(device, BUFSIZ, 1, 1000, errbuf);
  if (descr_ == NULL) {
    cout << errbuf << endl;
    exit(1);
  }
}

void Sniffer::registerCallback(void (*func)(u_char *args,
                                            const struct pcap_pkthdr* pkthdr,
                                            const u_char* packet)) {
  if (descr_ != NULL) {
    pcap_loop(descr_, -1, func, NULL);
  }
}
