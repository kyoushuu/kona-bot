/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * io-utils.c
 * Copyright (C) 2013 Arnel A. Borja <kyoushuu@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "io-utils.h"


ssize_t
send_all (int sockfd, const void *buf)
{
    size_t len;
    ssize_t pos, ret;

    len = strlen (buf);
    pos = 0;

    while (pos < len)
    {
        ret = send (sockfd, buf + pos, len - pos, 0);
        if (ret < 0)
            return ret;
        else if (ret == 0)
            break;

        pos += ret;
    }

    return pos;
}
