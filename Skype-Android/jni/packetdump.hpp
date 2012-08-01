#ifndef __packetdump_hpp_included__
#define __packetdump_hpp_included__

#include "tcprotoV3.hpp"
#include "bmcrypto/AttributeContainer.hpp"
#include <stdio.h>

extern void PacketDump(const char *title,const AttributeContainer& a,int level=1);

#endif
