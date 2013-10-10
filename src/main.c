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
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define HOST "irc.rizon.net"
#define PORT "6667"
#define CHANNEL "#otakubytes"
#define NICK "kona-bot"
#define NAME "Izumi Konata"

#define MAXDATASIZE 512

// get sockaddr, IPv4 or IPv6:
void *get_in_addr (struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (int argc, char *argv[])
{
    int sockfd, numbytes, msglen, ret;
    char buf[MAXDATASIZE], msg[MAXDATASIZE], *newline;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset (&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo (HOST, PORT, &hints, &servinfo)) != 0)
    {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (rv));
        return 1;
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
        return 2;
    }

    inet_ntop (p->ai_family, get_in_addr ((struct sockaddr *)p->ai_addr),
               s, sizeof s);
    printf ("Connecting to %s\n", s);

    freeaddrinfo (servinfo);

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
    }

    close (sockfd);

    return 0;
}
