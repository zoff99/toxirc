#ifndef NODES_H
#define NODES_H

#include <stdint.h>
#include <stdio.h>

struct Node {
    char *   ip;
    char *   key;
    uint16_t udp_port;
    uint16_t tcp_port;
} nodes[] = {
    //clang-format off
    { "tox.plastiras.org", "5E47BA1DC3913EB2CBF2D64CE4F23D8BFE5391BFABE5C43C5BAD13F0A414CD77", 38445, 38445 },   // tox.plastiras.org
    { "172.93.52.70", "79CADA4974B0926F286F025CD5FFDF3E654A3758C53E0273ECFC4D12C21DCA48", 33445, 38445 },  // jf
    { NULL, NULL, 0, 0 }
    //clang-format on
};

#endif
