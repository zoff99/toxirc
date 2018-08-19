#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include "bot.h"
#include "irc.h"
#include "logging.h"
#include "macros.h"
#include "network.h"
#include "save.h"
#include "settings.h"
#include "tox.h"

#include "callbacks/irc_callbacks.h"

bool exit_bot = false;

static void signal_catch(int UNUSED(sig)){
    exit_bot = true;
    printf("signal caught\n");
}

int main(void){
    signal(SIGINT, signal_catch);

    if (!settings_load(SETTINGS_FILE)) {
        DEBUG("WARNING", "Settings could not be loaded, default settings will be used.");
    }

    DEBUG("main", "Starting bot");

    Bot *bot = bot_init();
    if (!bot) {
        return 1;
    }

    bot_add_server(bot, settings.server, settings.port);

    bot->tox = tox_init();
    if (!bot->tox) {
        return 2;
    }

    if (!irc_connect(bot->irc[0])) {
        irc_free(bot->irc[0]);
        tox_kill(bot->tox);
        return 3;
    }

    irc_callbacks_setup(bot->irc[0]);

    TOX_ERR_CONFERENCE_NEW err;
    uint32_t group_num = tox_conference_new(bot->tox, &err);
    if (group_num == UINT32_MAX) {
        DEBUG("main", "Could not create groupchat for default group. Error number: %u", err);
        tox_kill(bot->tox);
        irc_disconnect(bot->irc[0]);
        irc_free(bot->irc[0]);
        return 4;
    }

    tox_conference_set_title(bot->tox, group_num, (const uint8_t *)settings.default_channel, strlen(settings.default_channel), NULL);
    irc_join_channel(bot->irc[0], settings.default_channel, group_num);

    while (!exit_bot) {
        irc_loop(bot->irc[0], bot->tox);
        tox_iterate(bot->tox, bot->irc[0]);
        usleep(tox_iteration_interval(bot->tox));
    }

    irc_disconnect(bot->irc[0]);
    irc_free(bot->irc[0]);

    save_write(bot->tox, SAVE_FILE);
    tox_kill(bot->tox);

    settings_save(SETTINGS_FILE);

    DEBUG("main", "Shutting down bot...");

    return 0;
}
