#include "tox_callbacks.h"

#include "../irc.h"
#include "../macros.h"
#include "../logging.h"
#include "../save.h"
#include "../settings.h"

#include "../commands/group_commands.h"
#include "../commands/friend_commands.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <tox/tox.h>

static void friend_message_callback(Tox *tox, uint32_t fid, TOX_MESSAGE_TYPE type, const uint8_t *message,
                                    size_t length, void *userdata) {
    if (type != TOX_MESSAGE_TYPE_NORMAL) {
        return;
    }

    IRC *irc = userdata;

    char msg[TOX_MAX_MESSAGE_LENGTH];
    length = MIN(TOX_MAX_MESSAGE_LENGTH - 1, length);
    memcpy(msg, message, length);
    msg[length] = '\0';

    length++;

    size_t cmd_length;
    char * cmd = command_parse(msg, length, &cmd_length);
    if (!cmd) {
        return;
    }

    size_t arg_length;
    char * arg = command_parse_arg(msg, length, cmd_length, &arg_length);

    bool valid = false;
    for (int i = 0; friend_commands[i].cmd; i++) {
        if (strncmp(cmd, friend_commands[i].cmd, strlen(friend_commands[i].cmd)) == 0) {
            friend_commands[i].func(tox, irc, fid, arg);
            valid = true;
        }
    }

    free(cmd);
    if (arg) {
        free(arg);
    }

    if (!valid) {
        tox_friend_send_message(tox, fid, TOX_MESSAGE_TYPE_NORMAL,
                                (uint8_t *)"Invalid command send me help to find out what commands I support",
                                sizeof("Invalid command send me help to find out what commands I support") - 1, NULL);
    }
}

static void group_message_callback(Tox *tox, uint32_t groupnumber, uint32_t peer_number, TOX_MESSAGE_TYPE type,
                                   const uint8_t *message, size_t length, void *userdata) {

  /*  if (tox_conference_peer_number_is_ours(tox, groupnumber, peer_number, NULL)) {
        return;
    }

    */

    IRC *irc = userdata;

    char *nosync_prefix = settings_get_prefix(CHAR_NO_SYNC_PREFIX);
    if (command_prefix_cmp((char *)message,
                           nosync_prefix)) { // messages beggining with ~ are not synced with irc
        return;
    }

    char msg[TOX_MAX_MESSAGE_LENGTH];
    length = MIN(TOX_MAX_MESSAGE_LENGTH - 1, length);
    memcpy(msg, message, length);
    msg[length] = '\0';

    length++;

    char *cmd_prefix = settings_get_prefix(CHAR_CMD_PREFIX);
    if (command_prefix_cmp((char *)message, cmd_prefix)) {
        size_t cmd_length;
        char * cmd = command_parse(msg, length, &cmd_length);
        if (!cmd) {
            return;
        }

        size_t arg_length;
        char * arg = command_parse_arg(msg, length, cmd_length, &arg_length);

        // bool valid = false;
        for (int i = 0; group_commands[i].cmd; i++) {
            if (strncmp(cmd, group_commands[i].cmd, strlen(group_commands[i].cmd)) == 0) {
                group_commands[i].func(tox, irc, groupnumber, NULL);
                // valid = true;
            }
        }

        free(cmd);
        if (arg) {
            free(arg);
        }

        
        /*
        if (!valid) {
            tox_group_send_message(tox, groupnumber, TOX_MESSAGE_TYPE_NORMAL,
                                        (uint8_t *)"Invalid command send me help to find out what commands I support",
                                        sizeof("Invalid command send me help to find out what commands I support") - 1,
                                        NULL);
        }

        */

        

        return;
        
    }

    uint8_t                       name[TOX_MAX_NAME_LENGTH];
    Tox_Err_Group_Peer_Query err;
    int                           name_len = tox_group_peer_get_name_size(tox, groupnumber, peer_number, &err);

    if (name_len == 0 || err != TOX_ERR_GROUP_PEER_QUERY_OK) {
        memcpy(name, "unknown", 7);
        name_len = 7;
    } else {
        tox_group_peer_get_name(tox, groupnumber, peer_number, name, NULL);
    }

    name[name_len] = '\0';

/*
    char *channel = irc_get_channel_by_group(irc, groupnumber);
    if (!channel) {
        DEBUG("Tox", "Could not get channel name, unable to send message for group: %u.", groupnumber);
        return;
    }
*/

    int next_character = 0;
    for (unsigned int i = 0; i < length; i++) {
        if (msg[i] == '\n' || msg[i] == '\0') {
            size_t message_size = i - next_character;

            char message_line[message_size];
            strncpy(message_line, msg + next_character, message_size);
            message_line[message_size] = '\0';
            message_size++;

            if (type == TOX_MESSAGE_TYPE_ACTION)
            {

                const size_t buffer_size = name_len + message_size + 3;
                char         buffer[buffer_size];

                snprintf(buffer, buffer_size, "<%s> %s", name, message_line);

                irc_send_action_message(irc, settings.default_channel, buffer);

            } else {

                const size_t buffer_size = name_len + message_size + 3;
                char         buffer[buffer_size];

                snprintf(buffer, buffer_size, "<%s> %s", name, message_line);

                irc_send_message(irc, settings.default_channel, buffer);

            }

            next_character = i + 1;

            usleep(250000); // sleep for a quarter of a second to prevent the bot from being kicked for spamming
        }
    }
}

static void friend_request_callback(Tox *tox, const uint8_t *public_key, const uint8_t *UNUSED(data),
                                    size_t UNUSED(length), void *UNUSED(userdata)) {
    TOX_ERR_FRIEND_ADD err;
    tox_friend_add_norequest(tox, public_key, &err);

    save_write(tox, SAVE_FILE);

    if (err != TOX_ERR_FRIEND_ADD_OK) {
        DEBUG("Tox", "Error accepting friend request. Error number: %d", err);
        return;
    }
}

static void self_connection_change_callback(Tox *tox, TOX_CONNECTION status, void *UNUSED(userdata)) {

    tox;

    switch (status) {
        case TOX_CONNECTION_NONE:
            DEBUG("Tox", "Lost connection to the Tox network.");
            // printf("Lost connection to the Tox network\n");
            break;
        case TOX_CONNECTION_TCP:
            DEBUG("Tox", "Connected using TCP.");
            // printf("Connected using TCP\n");
            break;
        case TOX_CONNECTION_UDP:
            DEBUG("Tox", "Connected using UDP.");
            // printf("Connected using UDP\n");
            break;
    }
}

static void group_invite_cb(Tox *tox, uint32_t friend_number, const uint8_t *invite_data, size_t length,
                                 const uint8_t *group_name, size_t group_name_length, void *user_data)
{
    size_t nick_len = tox_self_get_name_size(tox);
    char self_nick[TOX_MAX_NAME_LENGTH + 1];
    tox_self_get_name(tox, (uint8_t *) self_nick);
    self_nick[nick_len] = '\0'; 

    Tox_Err_Group_Invite_Accept error;
    tox_group_invite_accept(tox, friend_number, invite_data, length,
                                 (const uint8_t *)self_nick, nick_len, NULL, 0,
                                 &error);

    DEBUG("Tox", "tox_group_invite_accept:%d", error);

    save_write(tox, SAVE_FILE);
}

static void friend_connection_status_callback(Tox *tox, uint32_t friend_number, Tox_Connection connection_status,
        void *user_data)
{
    switch (connection_status) {
        case TOX_CONNECTION_NONE:
            DEBUG("Tox", "Lost connection to friend #%d.", friend_number);
            break;
        case TOX_CONNECTION_TCP:
            DEBUG("Tox", "Connected to friend #%d using TCP.", friend_number);
            break;
        case TOX_CONNECTION_UDP:
            DEBUG("Tox", "Connected to friend #%d using UDP.", friend_number);
            break;
    }
}

static void group_peer_join_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, void *user_data)
{
    DEBUG("Tox", "Peer %d joined group %d", peer_id, group_number);
}

static void group_peer_exit_cb(Tox *tox, uint32_t group_number, uint32_t peer_id, Tox_Group_Exit_Type exit_type,
                                    const uint8_t *name, size_t name_length, const uint8_t *part_message, size_t length, void *user_data)
{
    switch (exit_type) {
        case TOX_GROUP_EXIT_TYPE_QUIT:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_QUIT", peer_id, group_number, exit_type);
            break;
        case TOX_GROUP_EXIT_TYPE_TIMEOUT:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_TIMEOUT", peer_id, group_number, exit_type);
            break;
        case TOX_GROUP_EXIT_TYPE_DISCONNECTED:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_DISCONNECTED", peer_id, group_number, exit_type);
            break;
        case TOX_GROUP_EXIT_TYPE_SELF_DISCONNECTED:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_SELF_DISCONNECTED", peer_id, group_number, exit_type);
            break;
        case TOX_GROUP_EXIT_TYPE_KICK:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_KICK", peer_id, group_number, exit_type);
            break;
        case TOX_GROUP_EXIT_TYPE_SYNC_ERROR:
        DEBUG("Tox", "Peer %d left group %d reason: %d TOX_GROUP_EXIT_TYPE_SYNC_ERROR", peer_id, group_number, exit_type);
            break;
    }
}

static void group_self_join_cb(Tox *tox, uint32_t group_number, void *user_data)
{
    DEBUG("Tox", "You joined group %d", group_number);
}

static void group_join_fail_cb(Tox *tox, uint32_t group_number, Tox_Group_Join_Fail fail_type, void *user_data)
{
    DEBUG("Tox", "Joining group %d failed. reason: %d", group_number, fail_type);
}

void tox_callbacks_setup(Tox *tox) {
    tox_callback_self_connection_status(tox, self_connection_change_callback);
    tox_callback_friend_connection_status(tox, friend_connection_status_callback);
    tox_callback_friend_message(tox, friend_message_callback);
    tox_callback_friend_request(tox, friend_request_callback);
    tox_callback_group_message(tox, group_message_callback);
    tox_callback_group_invite(tox, group_invite_cb);
    tox_callback_group_peer_join(tox, group_peer_join_cb);
    tox_callback_group_peer_exit(tox, group_peer_exit_cb);
    tox_callback_group_self_join(tox, group_self_join_cb);
    tox_callback_group_join_fail(tox, group_join_fail_cb);
}
