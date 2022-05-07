#include "save.h"

#include "logging.h"
#include "settings.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <tox/tox.h>

bool save_write(Tox *tox, char *path) {
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        DEBUG("Save", "Can not open: %s", path);
        return false;
    }

    size_t   size = tox_get_savedata_size(tox);
    uint8_t *data = calloc(1, size + 1);
    if (!data) {
        DEBUG("Save", "Could not allocate memory for save data.");
        return false;
    }

    tox_get_savedata(tox, data);

    if (fwrite(data, size, 1, fp) != 1) {
        DEBUG("Save", "Could not write save data to %s", path);
        free(data);
        fclose(fp);
        return false;
    }

    free(data);
    fclose(fp);

    return true;
}

void log_callback(Tox *tox, Tox_Log_Level level, const char *file, uint32_t line, const char *func,
                        const char *message, void *user_data)
{
    char *level_char = "?";

    switch (level) {
        case 0:
            level_char[0] = 'E';
            break;

        case 1:
            level_char[0] = 'W';
            break;

        case 2:
            level_char[0] = 'I';
            break;

        default:
            if (level > 2) {
                level_char[0] = 'D';
            } else {
                level_char[0] = '?';
            }
            break;
    }

    DEBUG("TOXCORE", "%s %s:%u(%s) %s", level_char, file, line, func, message);
}


Tox *save_load(char *path, int *status) {
    Tox *              tox = NULL;
    struct Tox_Options options;

    //memset(&options, 0, sizeof(struct Tox_Options));
    tox_options_default(&options);
    tox_options_set_ipv6_enabled(&options, settings.ipv6);
    tox_options_set_local_discovery_enabled(&options, false);
    tox_options_set_udp_enabled(&options, false);
    tox_options_set_log_callback(&options, log_callback);

    #ifdef _WIN32
        FILE *fp = fopen(path, "rb");
    #else
        int   fd = open(path, O_RDWR | O_CREAT,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // TODO: this needs to be improved
        FILE *fp = fdopen(fd, "rb");
    #endif

    if (!fp) {
        #ifdef _WIN32
            DEBUG("Save", "Could not open/find tox save. Assuming new profile.", path); //@todo This needs to be properly addressed
            fclose(fp);
            tox     = tox_new(&options, NULL);
            *status = 2;
            return tox;
        #else
            DEBUG("Save", "Could not open %s", path);
            return NULL;
        #endif
    }

    off_t size = get_file_size(path);
    if (size == 0) {
        DEBUG("Save", "Could not get the file size for %s. Assuming new profile.", path);
        fclose(fp);
        tox     = tox_new(&options, NULL);
        *status = 2;
        return tox;
    }

    uint8_t data[size];

    if (fread(data, size, 1, fp) != 1) {
        DEBUG("Save", "Could not read the save data from %s", path);
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    options.savedata_data   = data;
    options.savedata_length = size;
    options.savedata_type   = TOX_SAVEDATA_TYPE_TOX_SAVE;

    TOX_ERR_NEW err;
    tox = tox_new(&options, &err);

    if (err != TOX_ERR_NEW_OK) {
        DEBUG("Tox", "Failed to create tox instance. Error number: %d", err);
        return NULL;
    }

    *status = 1;

    return tox;
}
