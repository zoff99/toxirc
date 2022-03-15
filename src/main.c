#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #define WIN32_LEAN_AND_MEAN
#else
    #include <sys/socket.h>
    #include <sys/ioctl.h>
#endif

#include "irc.h"
#include "logging.h"
#include "macros.h"
#include "network.h"
#include "save.h"
#include "settings.h"
#include "tox.h"
#include "../../testing/misc_tools.h"

#include "callbacks/irc_callbacks.h"

bool exit_bot = false;

static void signal_catch(int UNUSED(sig)) {
    exit_bot = true;
    printf("signal caught\n");
}

int main(void) {
    signal(SIGINT, signal_catch);

    if (!settings_load(SETTINGS_FILE)) {
        DEBUG("WARNING", "Settings could not be loaded, default settings will be used.");
    }

    DEBUG("main", "Starting bot");

    Tox *tox = tox_init();
    if (!tox) {
        return 1;
    }

    IRC *irc = irc_init(settings.server, settings.port);
    if (!irc) {
        tox_kill(tox);
        return 2;
    }

    if (!irc_connect(irc, settings.name, settings.password)) {
        irc_free(irc);
        tox_kill(tox);
        return 3;
    }

    uint8_t *id_bin = hex_string_to_bin("4B61BB3CF3F505BD6D9650452130D4AE4802CB82BCA8D75526A82DEA29C7FA17");

    size_t nick_len = tox_self_get_name_size(tox);
    char self_nick[TOX_MAX_NAME_LENGTH + 1];
    tox_self_get_name(tox, (uint8_t *) self_nick);
    self_nick[nick_len] = '\0';

    uint32_t group_number = tox_group_join(tox, (const uint8_t *) id_bin, (const uint8_t *) self_nick, nick_len, NULL, 0, NULL);
    free(id_bin);

    irc_callbacks_setup(irc);


    irc_join_channel(irc, (char *) "#toktok", group_number);

/*
    uint32_t num_group = tox_group_get_number_groups(tox);
    if (num_group > 0) {
        uint32_t num_group;

        for (uint32_t i = 0; i < num_group; i++) {
            uint32_t title_size = tox_group_get_name_size(tox, num_group, NULL);
            if (title_size == 0) {
                continue;
            }

            uint8_t title[title_size];
            tox_group_get_name(tox, num_group, title, NULL);
            title[title_size] = '\0';

            irc_join_channel(irc, (char *)title, num_group);
        }
    } else {
        TOX_ERR_CONFERENCE_NEW err;
        uint32_t               group_num = tox_conference_new(tox, &err);
        if (group_num == UINT32_MAX) {
            DEBUG("main", "Could not create groupchat for default group. Error number: %u", err);
            tox_kill(tox);
            irc_disconnect(irc);
            irc_free(irc);
            return 4;
        }

        
        tox_conference_set_title(tox, group_num, (const uint8_t *)settings.default_channel,
                                 strlen(settings.default_channel), NULL);
                                 

        irc_join_channel(irc, settings.default_channel, group_num);
    }
    */

    while (!exit_bot) {
        irc_loop(irc, tox);
        tox_iterate(tox, irc);
        usleep(tox_iteration_interval(tox));
    }

    irc_disconnect(irc);
    irc_free(irc);

    save_write(tox, SAVE_FILE);
    tox_kill(tox);

    settings_save(SETTINGS_FILE);

    DEBUG("main", "Shutting down bot...");

    return 0;
}
