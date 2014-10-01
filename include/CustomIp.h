#include "RouteTable.h"
#include "MyIp.h"
#include "PacketEngine.h"

/* global Route table */
static RouteTable routeTable;
/* Self Ip address */
static MyIp myIps;
static PacketEngine packetEngine;
