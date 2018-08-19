#ifndef BOT_H
#define BOT_H

#include <tox/tox.h>
#include "irc.h"

struct bot {
    Tox *tox;

    IRC **irc;
    uint32_t num_servers;
    uint32_t size_servers;
};

typedef struct bot Bot;

Bot *bot_init(void);

bool bot_add_server(Bot *bot, char *server, char *port);

void bot_free(Bot *bot);

#endif
