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
#include "utils.h"

#include "callbacks/irc_callbacks.h"

bool exit_bot = false;

static void signal_catch(int UNUSED(sig)) {
    exit_bot = true;
    printf("signal caught\n");
}

int main(void) {
    signal(SIGINT, signal_catch);

    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    if (!settings_load(SETTINGS_FILE)) {
        DEBUG("WARNING", "Settings could not be loaded, default settings will be used.");
    }

    DEBUG("main", "Starting bot");

    /*if(remove("toxirc_save.tox") == -1) {
        printf("Error in deleting a file\n");
    }*/

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

    save_write(tox, SAVE_FILE);

    DEBUG("main", "irc_callbacks_setup");
    irc_callbacks_setup(irc);

    size_t nick_len = tox_self_get_name_size(tox);
    char self_nick[TOX_MAX_NAME_LENGTH + 1];
    tox_self_get_name(tox, (uint8_t *) self_nick);
    self_nick[nick_len] = '\0'; 

    uint32_t num_friends = tox_self_get_friend_list_size(tox);
    DEBUG("Tox", "tox num_friends:%d", num_friends);

    uint32_t num_groups = tox_group_get_number_groups(tox);
    DEBUG("Tox", "tox num_groups:%d", num_groups);

    DEBUG("Tox", "irc_join_channel:%s i:%d", settings.default_channel, 0);
    irc_join_channel(irc, settings.default_channel, 0);

    DEBUG("Tox", "iterate");
    tox_iterate(tox, irc);
    usleep(tox_iteration_interval(tox));

    while (!exit_bot) {
        irc_loop(irc, tox);
        tox_iterate(tox, irc);
        usleep(tox_iteration_interval(tox));
    }

    DEBUG("main", "irc_disconnect");
    irc_disconnect(irc);

    DEBUG("main", "irc_free");
    irc_free(irc);

    DEBUG("main", "saving toxsave ...");
    save_write(tox, SAVE_FILE);
    DEBUG("main", "saving toxsave done");

    DEBUG("main", "killing tox ...");
    tox_kill(tox);
    DEBUG("main", "killing tox done");

    settings_save(SETTINGS_FILE);

    DEBUG("main", "Shutting down bot ...");

    return 0;
}
