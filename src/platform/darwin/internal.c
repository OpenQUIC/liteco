/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "platform/internal.h"

#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>

int liteco_platform_cloexec(int fd, bool set) {
    int r;
    do {
        r = ioctl(fd, set ? FIOCLEX : FIONCLEX);
    } while (r == -1 && errno == EINTR);

    if (r) {
        return errno;
    }

    return 0;
}

int liteco_platform_pipe(int fds[2], const int flags) {
    (void) flags;

    if (pipe(fds)) {
        return errno;
    }

    int err;
    if ((err = liteco_platform_cloexec(fds[0], 1))) {
        goto failed;
    }
    if ((err = liteco_platform_cloexec(fds[1], 1))) {
        goto failed;
    }

    return 0;
failed:
    close(fds[0]);
    close(fds[1]);

    return err;
}

int liteco_platform_nonblock(int fd, bool set) {
    int r;
    do {
        r = ioctl(fd, FIONBIO, &set);
    } while (r == -1 && errno == EINTR);

    if (r) {
        return errno;
    }

    return 0;
}

int liteco_platform_close(int fd) {
    return close(fd);
}

