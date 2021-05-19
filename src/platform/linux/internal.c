/*
 * Copyright (c) 2021 Gscienty <gaoxiaochuan@hotmail.com>
 *
 * Distributed under the MIT software license, see the accompanying
 * file LICENSE or https://www.opensource.org/licenses/mit-license.php .
 *
 */

#include "platform/internal.h"
#include <unistd.h>

int liteco_platform_close(int fd) {
    return close(fd);
}

int liteco_udp_socket(int domain) {
    return socket(domain, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
}
