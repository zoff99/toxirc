#include "logging.h"

#include "settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

void debug(const char *fmt, ...) {
    if (!settings.verbose) {
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm tm = *localtime(&tv.tv_sec);

    const size_t len = 26 + 5 + strlen(fmt) + 2;
    char *buf = calloc(1, len);
    buf[28] = ' ';
    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d.%06ld [_] %s", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, fmt);

    va_list ap;
    va_start(ap, fmt);
    vprintf(buf, ap);
    va_end(ap);

    free(buf);
}
