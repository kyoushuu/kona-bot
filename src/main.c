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
#include "genre.h"
#include "io-utils.h"
#include "irc.h"
#include "str-utils.h"


int main (int argc, char *argv[])
{
    int sockfd, numbytes, msglen, ret;
    char buf[MAXDATASIZE], msg[MAXDATASIZE], *newline;
    char res[MAXDATASIZE];
    char *irc_host, *irc_port, *irc_chan, *irc_nick, *irc_pass, *irc_name;
    char *irc_nick_ping;

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

    snprintf (res, MAXDATASIZE, "%s:", irc_nick);
    irc_nick_ping = strdup (res);

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

        if (strcmp (msgtype, "PRIVMSG") == 0)
        {
            char *send_to, *temp1;
            char **args;
            int arg_len;

            args = NULL;
            arg_len = 0;
            send_to = NULL;

            if (strcmp (recipient, irc_nick) == 0)
            {
                char *sep;

                send_to = strdup (sender);
                arg_len = parse_command (contents, &args);

                sep = strchr (send_to, '!');
                if (sep)
                    *sep = '\0';
            }
            else if (strncmp (contents, irc_nick_ping, strlen (irc_nick_ping)) == 0)
            {
                send_to = strdup (recipient);
                arg_len = parse_command (contents + strlen (irc_nick_ping), &args);
            }

            if (arg_len > 0)
            {
                if (strcmp (args[0], "add") == 0)
                {
                    if (strcmp (args[1], "genre") == 0)
                    {
                        char *title, *name;

                        title = arg_len > 2? args[2] : NULL;
                        name = arg_len > 3? args[3] : NULL;

                        switch (genre_add (db, title, name))
                        {
                            case ERR_GENRE_TITLE_EMPTY:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Please specify a title for the genre.\r\n",
                                          send_to);
                                send_all (sockfd, res);

                                break;

                            case ERR_GENRE_TITLE_EXIST:
                                temp1 = malloc (strlen (title) * 2 + 1);
                                mysql_real_escape_string (db, temp1, title, strlen (title));

                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :The genre \"%s\" already exists.\r\n",
                                          send_to, temp1);
                                send_all (sockfd, res);

                                free (temp1);

                                break;

                            case ERR_GENRE_NAME_EXIST:
                                temp1 = malloc (strlen (name) * 2 + 1);
                                mysql_real_escape_string (db, temp1, name, strlen (name));

                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :The genre with name \"%s\" already exists.\r\n",
                                          send_to, temp1);
                                send_all (sockfd, res);

                                free (temp1);

                                break;

                            case ERR_QUERY_FAILED:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Sorry, an internal error occured. Please contact the administrators.\r\n",
                                          send_to);
                                send_all (sockfd, res);

                                break;

                            default:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Successfully added genre \"%s\"!\r\n",
                                          send_to, title);
                                send_all (sockfd, res);

                                break;
                        }
                    }
                }
                else if (strcmp (args[0], "remove") == 0)
                {
                    if (strcmp (args[1], "genre") == 0)
                    {
                        char *genre;
                        int ret;

                        genre = arg_len > 2? args[2] : NULL;
                        ret = genre_remove_with_title (db, genre);

                        if (ret == ERR_GENRE_TITLE_DONT_EXIST)
                            ret = genre_remove_with_name (db, genre);

                        switch (ret)
                        {
                            case ERR_GENRE_TITLE_EMPTY:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Please specify the title of the genre.\r\n",
                                          send_to);
                                send_all (sockfd, res);

                                break;

                            case ERR_GENRE_TITLE_DONT_EXIST:
                            case ERR_GENRE_NAME_DONT_EXIST:
                                temp1 = malloc (strlen (genre) * 2 + 1);
                                mysql_real_escape_string (db, temp1, genre, strlen (genre));

                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :The genre with title nor name \"%s\" doesn't exists.\r\n",
                                          send_to, temp1);
                                send_all (sockfd, res);

                                free (temp1);

                                break;

                            case ERR_QUERY_FAILED:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Sorry, an internal error occured. Please contact the administrators.\r\n",
                                          send_to);
                                send_all (sockfd, res);

                                break;

                            default:
                                snprintf (res, MAXDATASIZE,
                                          "PRIVMSG %s :Successfully removed genre \"%s\"!\r\n",
                                          send_to, genre);
                                send_all (sockfd, res);

                                break;
                        }
                    }
                }
            }

            if (args)
            {
                int i;

                for (i = 0; i < arg_len; i++)
                    free (args[i]);

                free (args);
            }

            if (send_to)
                free (send_to);
        }
    }

    free (irc_nick_ping);

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
