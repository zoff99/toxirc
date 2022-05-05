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
    { "tox.plastiras.org", "8E8B63299B3D520FB377FE5100E65E3322F7AE5B20A0ACED2981769FC5B43725", 33445, 443 },
    { "tox.verdict.gg", "1C5293AEF2114717547B39DA8EA6F1E331E5E358B35F9B6B5F19317911C5F976", 33445, 33445 },
//    { "144.217.167.73" "7E5668E0EE09E19F320AD47902419331FFEE147BB3606769CFBE921A2A2FD34C", 33445, 33445 },
    { NULL, NULL, 0, 0 }
    //clang-format on
};

#endif
