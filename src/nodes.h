#ifndef NODES_H
#define NODES_H

#include <stdint.h>
#include <stdio.h>

struct Node {
    char *ip;
    char *key;
    uint16_t udp_port;
    uint16_t tcp_port;
} nodes[] = {
    { "185.14.30.213", "2555763C8C460495B14157D234DD56B86300A2395554BCAE4621AC345B8C1B1B", 443, 443 },      // dvor
    { "136.243.141.187", "6EE1FADE9F55CC7938234CC07C864081FC606D8FE7B751EDA217F268F1078A39", 443, 443 },    // CeBe
    { "nodes.tox.chat", "6FC41E2BD381D37E9748FC0E0328CE086AF9598BECC8FEB7DDF2E440475F300E", 33445, 33445 }, // Impyy
    {
        "node.tox.biribiri.org",
        "F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67",
        33445,
        33445,
    }, // nurupo
    { NULL, NULL, 0, 0 }
};

#endif
