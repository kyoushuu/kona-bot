/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2013 Arnel A. Borja <kyoushuu@yahoo.com>
 * 
 * kona-bot is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * kona-bot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include <mysql.h>

#include "database.h"
#include "io-utils.h"
#include "irc.h"


int main (int argc, char *argv[])
{
    int sockfd, numbytes, msglen, ret;
    char buf[MAXDATASIZE], msg[MAXDATASIZE], *newline;
    char res[MAXDATASIZE];
    char *irc_host, *irc_port, *irc_chan, *irc_nick, *irc_pass, *irc_name;

    MYSQL *db;

    sockfd = open_irc_socket (&irc_host, &irc_port, &irc_chan, &irc_nick, &irc_pass, &irc_name);
    if (sockfd < 0)
    {
        return 1;
    }

    db = open_database ();
    if (db == NULL)
    {
        close (sockfd);
        return 1;
    }

    numbytes = 0;
    buf[0] = '\0';
    while (1)
    {
        char *msgtype, *recipient, *sender, *contents;

        while (!(newline = strstr (buf, "\r\n")))
        {
            ret = recv (sockfd, buf + numbytes, MAXDATASIZE - numbytes - 1, 0);
            if (ret == -1)
            {
                perror ("recv");
                break;
            }
            else if (ret == 0)
                break;

            numbytes += ret;
        }

        msglen = newline + 2 - buf;
        strncpy (msg, buf, numbytes);
        numbytes -= msglen;
        strncpy (buf, msg + msglen, numbytes);
        msg[msglen - 2] = '\0';
        buf[numbytes] = '\0';

        printf ("%s\n", msg);

        sender = msg + 1;

        msgtype = strchr (msg, ' ');
        if (!msgtype)
            continue;
        msgtype[0] = '\0';
        msgtype++;

        if (strcmp (msg, "PING") == 0)
        {
            /* PING has msgtype as the sender */
            snprintf (res, MAXDATASIZE, "PONG :%s\r\n", msgtype);
            send_all (sockfd, res);
        }

        recipient = strchr (msgtype, ' ');
        if (!recipient)
            continue;
        recipient[0] = '\0';
        recipient++;

        contents = strchr (recipient, ' ');
        if (!contents)
            continue;
        contents[0] = '\0';
        contents += 2;
    }

    mysql_close (db);
    close (sockfd);

    free (irc_host);
    free (irc_port);
    free (irc_chan);
    free (irc_nick);
    free (irc_pass);
    free (irc_name);

    return 0;
}
