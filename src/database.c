/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * database.c
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
#include <string.h>

#include <mysql.h>

#include "database.h"


MYSQL *
open_database ()
{
    MYSQL *db, *conn;
    char path[512], *buf, *saveptr, *field, *value;
    FILE *file;
    int len, pos;

    char *db_host, *db_user, *db_pass, *db_name;
    int db_port;

    snprintf (path, 512, "%s/.konabotrc", getenv ("HOME"));

    file = fopen (path, "r");
    if (!file)
    {
        return NULL;
    }

    if (fseek (file, 0, SEEK_END) != 0)
    {
        fclose (file);
        return NULL;
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
                return NULL;
            }
        }

        pos += ret;
    }

    db_host = NULL;
    db_user = NULL;
    db_pass = NULL;
    db_name = NULL;
    db_port = 0;

    field = strtok_r (buf, "\n", &saveptr);
    while (field) {
        value = strchr (field, '=');
        if (value)
        {
            *value = '\0';
            value++;

            if (strcmp (field, "db_host") == 0)
                db_host = value;
            else if (strcmp (field, "db_user") == 0)
                db_user = value;
            else if (strcmp (field, "db_pass") == 0)
                db_pass = value;
            else if (strcmp (field, "db_name") == 0)
                db_name = value;
            else if (strcmp (field, "db_port") == 0)
                db_port = atoi (value);
        }

        field = strtok_r (NULL, "\n", &saveptr);
    }

    fclose (file);

    db = mysql_init (NULL);
    if (!db)
    {
        fprintf (stderr, "%s\n", mysql_error (db));
        free (buf);
        return NULL;
    }

    conn = mysql_real_connect (db, db_host, db_user, db_pass,
                               db_name, db_port, NULL, 0);
    free (buf);

    if (!conn)
    {
        fprintf (stderr, "%s\n", mysql_error (db));
        mysql_close (db);
        return NULL;
    }
    else
        return db;
}


int
get_id_from_query (MYSQL *db, const char *query)
{
    int result, id;

    MYSQL_RES *result_set;
    MYSQL_ROW row;

    result = mysql_query (db, query);
    if (result != 0)
        return ERR_QUERY_FAILED;

    result_set = mysql_use_result (db);

    row = mysql_fetch_row (result_set);
    if (row == NULL)
        return ERR_QUERY_FAILED;

    id = atoi (row[0]);

    while (mysql_fetch_row (result_set) != NULL);
    mysql_free_result (result_set);

    return id;
}
