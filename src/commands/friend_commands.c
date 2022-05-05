#include "friend_commands.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "commands.h"

#include "../logging.h"
#include "../macros.h"
#include "../settings.h"
#include "../tox.h"
#include "../utils.h"
#include "../save.h"

//static bool command_invite(Tox *tox, IRC *irc, uint32_t index, char *arg);
// static bool command_join(Tox *tox, IRC *irc, uint32_t index, char *arg);
//static bool command_leave(Tox *tox, IRC *irc, uint32_t index, char *arg);
static bool command_list(Tox *tox, IRC *irc, uint32_t index, char *arg);
// static bool command_id(Tox *tox, IRC *irc, uint32_t index, char *arg);
static bool command_info(Tox *tox, IRC *irc, uint32_t index, char *arg);
//static bool command_la(Tox *tox, IRC *irc, uint32_t index, char *arg);
//static bool command_name(Tox *tox, IRC *irc, uint32_t index, char *arg);
//static bool command_default(Tox *tox, IRC *irc, uint32_t index, char *arg);
//static bool command_master(Tox *tox, IRC *irc, uint32_t index, char *arg);
static bool command_help(Tox *tox, IRC *irc, uint32_t index, char *arg);
static bool command_warn(Tox *tox, IRC *irc, uint32_t fid, char *arg);
//static bool command_limit(Tox *tox, IRC *irc, uint32_t fid, char *arg);

// clang-format off
struct Command friend_commands[MAX_CMDS] = {
    //{ "invite",  "invite #channelname to get invited to a channel the bot has joined.",   false, command_invite  },
    // { "join",    "join #channelname to join a specific channel.",                         false, command_join    },
    //{ "leave",   "leave #channelname to leave a specific channel.",                       true,  command_leave   },
    { "list",    "Shows all channels the bot has joined.",                                false, command_list    },
    // { "id",      "Displays this bot's tox ID.",                                           false, command_id      },
    { "info",    "Shows additional info about this bot.",                                 false, command_info    },
    //{ "la",      "Forces this bot to leave all channels",                                 true,  command_la      },
    // { "name",    "Set this bot's name",                                                   true,  command_name    },
    //{ "default", "default #channelname sets the default channel for invite.",             true,  command_default },
    //{ "master",  "Add a ToxID to set the bot's owner.",                                   true,  command_master  },
    { "warn",    "Warn all channels and groupchats the bot is going down.",               true,  command_warn    },
    //{ "limit",   "Limit the number of channels the bot can join.",                        true,  command_limit   },
    { "help",    "Displays this list of commands.",                                       false, command_help    },
    { NULL,      NULL,                                                                    false, NULL            },
};
// clang-format on



static bool command_list(Tox *tox, IRC *irc, uint32_t fid, char *UNUSED(arg)) {
    if (irc->num_channels == 0) {
        tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (const uint8_t *)"I'm currently in no channels.",
                                strlen("I'm currently in no channels.") - 1, NULL);
        return true;
    }

    for (uint32_t i = 0; i < irc->num_channels; i++) {
        if (irc->channels[i].in_channel) {
            tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (const uint8_t *)irc->channels[i].name,
                                    strlen(irc->channels[i].name), NULL);
        }
    }

    return true;
}


static bool command_help(Tox *tox, IRC *UNUSED(irc), uint32_t fid, char *UNUSED(arg)) {
    for (int i = 0; friend_commands[i].cmd; i++) {
        if (!friend_commands[i].master || (friend_commands[i].master && tox_is_friend_master(tox, fid))) {
            char   message[TOX_MAX_MESSAGE_LENGTH] = { 0 };
            size_t length =
                snprintf(message, sizeof(message), "%s: %s", friend_commands[i].cmd, friend_commands[i].desc);
            tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (const uint8_t *)message, length, NULL);
        }
    }

    return true;
}

static bool command_info(Tox *tox, IRC *UNUSED(irc), uint32_t fid, char *UNUSED(arg)) {
    int num_frends = tox_self_get_friend_list_size(tox);

    uint32_t friends[num_frends];
    tox_self_get_friend_list(tox, friends);

    int online = 0;
    for (int i = 0; i < num_frends; i++) {
        if (tox_friend_get_connection_status(tox, i, NULL) != TOX_CONNECTION_NONE) {
            online++;
        }
    }

    char message[TOX_MAX_MESSAGE_LENGTH];
    int  length =
        snprintf(message, sizeof(message), "I am friends with %d people. %d of them are online.", num_frends, online);

    tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (uint8_t *)message, length, NULL);

    return true;
}

static bool command_default(Tox *tox, IRC *irc, uint32_t fid, char *arg) {
    if (!tox_is_friend_master(tox, fid)) {
        return false;
    }

    if (!arg) {
        tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (uint8_t *)"An argument is required.",
                                sizeof("An argument is required.") - 1, NULL);
        return false;
    }

    if (!irc_in_channel(irc, arg)) {
        tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL, (uint8_t *)"I am not in that channel.",
                                sizeof("I am not in that channel.") - 1, NULL);
        return false;
    }

    strcpy(settings.default_channel, arg);

    settings_save(SETTINGS_FILE);

    return true;
}

static bool command_warn(Tox *tox, IRC *irc, uint32_t fid, char *UNUSED(arg)) {
    if (!tox_is_friend_master(tox, fid)) {
        return false;
    }

    char   warning[TOX_MAX_MESSAGE_LENGTH]; // TODO: allow the message to be passed as an argument
    size_t length =
        snprintf(warning, sizeof(warning), "%s is about to be shutdown. Sorry for the inconvenience.", settings.name);

    for (uint32_t i = 0; i < irc->num_channels; i++) {
        irc_send_message(irc, irc->channels[i].name, warning);
        tox_group_send_message(tox, irc->channels[i].group_num, TOX_MESSAGE_TYPE_NORMAL, (const uint8_t *)warning,
                                    length, NULL);
    }

    return true;
}

static bool command_limit(Tox *tox, IRC *UNUSED(irc), uint32_t fid, char *arg) {
    if (!tox_is_friend_master(tox, fid)) {
        return false;
    }

    long new_limit = atol(arg);
    if (new_limit == -1) {
        new_limit = UINT32_MAX;
    }

    settings.channel_limit = new_limit;

    return true;
}
