#include "bot.h"

#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

Bot *bot_init(void){
    Bot *bot = malloc(sizeof(Bot));
    if (!bot) {
        return NULL;
    }

    memset(bot, 0, sizeof(Bot));

    return bot;
}

bool bot_add_server(Bot *bot, char *server, char *port){
    if ((bot->num_servers + 1) >= bot->size_servers) {
        DEBUG("Bot", "Reallocating from %d to %d", bot->size_servers, bot->size_servers + 1);
        void *temp = realloc(bot->irc, sizeof(IRC) * (bot->size_servers + 1));
        if (!temp) {
            DEBUG("Bot", "Could not reallocate memory from %d to %d.", bot->size_servers, bot->size_servers + 1);
            return false;
        }

        bot->irc = temp;

        bot->size_servers++;
        bot->num_servers++;
    }

    const uint32_t index = bot->num_servers - 1;
    bot->irc[index] = irc_init(server, port);
    if (!bot->irc[index]) {
        bot->num_servers--;
        return false;
    }

    return true;
}

void bot_free(Bot *bot){
    if (bot->irc) {
        for (uint32_t i = 0; i < bot->num_servers; i++) {
            irc_free(bot->irc[i]);
        }
        free(bot->irc);
    }

    bot->num_servers = 0;

    free(bot);
    bot = NULL;
}
