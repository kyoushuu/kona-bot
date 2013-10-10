/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * irc.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "io-utils.h"
#include "irc.h"

// get sockaddr, IPv4 or IPv6:
static void *
get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int
open_irc_socket (char **host, char **port, char **chan, char **nick, char **pass, char **name)
{
    char path[512], *buf, *saveptr, *field, *value;
    char res[MAXDATASIZE];
    FILE *file;
    int len, pos;

    struct addrinfo hints, *servinfo, *p;
    int sockfd, rv;
    char s[INET6_ADDRSTRLEN];

    char *irc_host, *irc_port, *irc_chan, *irc_nick, *irc_pass, *irc_name;

    snprintf (path, 512, "%s/.konabotrc", getenv ("HOME"));

    file = fopen (path, "r");
    if (!file)
    {
        return -1;
    }

    if (fseek (file, 0, SEEK_END) != 0)
    {
        fclose (file);
        return -1;
    }

    len = ftell (file);
    buf = malloc (len + 1);

    pos = 0;
    rewind (file);
    while (pos < len)
    {
        int ret;

        ret = fread (buf, len - pos, 1, file);
        if (ret <= 0)
        {
            if (feof (file))
                break;
            else
            {
                fclose (file);
                free (buf);
                return -1;
            }
        }

        pos += ret;
    }

    irc_host = NULL;
    irc_port = NULL;
    irc_chan = NULL;
    irc_nick = NULL;
    irc_pass = NULL;
    irc_name = NULL;

    field = strtok_r (buf, "\n", &saveptr);
    while (field) {
        value = strchr (field, '=');
        if (value)
        {
            *value = '\0';
            value++;

            if (strcmp (field, "irc_host") == 0)
                irc_host = value;
            else if (strcmp (field, "irc_port") == 0)
                irc_port = value;
            else if (strcmp (field, "irc_chan") == 0)
                irc_chan = value;
            else if (strcmp (field, "irc_nick") == 0)
                irc_nick = value;
            else if (strcmp (field, "irc_pass") == 0)
                irc_pass = value;
            else if (strcmp (field, "irc_name") == 0)
                irc_name = value;
        }

        field = strtok_r (NULL, "\n", &saveptr);
    }

    fclose (file);

    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo (irc_host, irc_port, &hints, &servinfo)) != 0)
    {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket (p->ai_family, p->ai_socktype,
                              p->ai_protocol)) == -1)
        {
            perror ("socket");
            continue;
        }

        if (connect (sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close (sockfd);
            perror ("connect");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf (stderr, "Failed to connect\n");
        return -1;
    }

    inet_ntop (p->ai_family, get_in_addr ((struct sockaddr *)p->ai_addr),
               s, sizeof s);
    printf ("Connecting to %s\n", s);

    freeaddrinfo (servinfo);

    snprintf (res, MAXDATASIZE, "NICK %s\r\n", irc_nick);
    send_all (sockfd, res);

    snprintf (res, MAXDATASIZE, "USER %s 0 * :%s\r\n", irc_nick, irc_name);
    send_all (sockfd, res);

    if (irc_pass && irc_pass[0] != '\0')
    {
        snprintf (res, MAXDATASIZE, "PRIVMSG NickServ :IDENTIFY %s\r\n", irc_pass);
        send_all (sockfd, res);
    }

    snprintf (res, MAXDATASIZE, "JOIN %s\r\n", irc_chan);
    send_all (sockfd, res);

    *host = strdup (irc_host);
    *port = strdup (irc_port);
    *chan = strdup (irc_chan);
    *nick = strdup (irc_nick);
    *pass = strdup (irc_pass);
    *name = strdup (irc_name);

    free (buf);

    return sockfd;
}
