/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * genre.c
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
#include <ctype.h>

#include <mysql.h>

#include "database.h"
#include "genre.h"
#include "str-utils.h"


static char *
genre_create_name_from_title (const char *title)
{
    int i, j;
    char *name;

    name = malloc (strlen (title));
    for (i = 0, j = 0; i < strlen (title); i++)
    {
        if (islower (title[i]) || isdigit (title[i]) || title[i] == '\0')
            name[j++] = title[i];
        else if (isupper (title[i]))
            name[j++] = tolower (title[i]);
        else if (isblank (title[i]))
            name[j++] = '-';
    }

    return name;
}


int
genre_get_id_with_title (MYSQL *db, const char *title)
{
    char query[512];
    char *temp;

    if (strempty (title))
        return ERR_GENRE_TITLE_EMPTY;

    temp = malloc (strlen (title) * 2 + 1);
    mysql_real_escape_string (db, temp, title, strlen (title));

    snprintf (query, 512, "SELECT id FROM genres WHERE title = \"%s\"", temp);

    free (temp);

    return get_id_from_query (db, query);
}


int
genre_get_id_with_name (MYSQL *db, const char *name)
{
    char query[512];
    char *temp;

    if (strempty (name))
        return ERR_GENRE_NAME_EMPTY;

    temp = malloc (strlen (name) * 2 + 1);
    mysql_real_escape_string (db, temp, name, strlen (name));

    snprintf (query, 512, "SELECT id FROM genres WHERE title = \"%s\"", temp);

    free (temp);

    return get_id_from_query (db, query);
}


int
genre_add (MYSQL *db, const char *title, const char *name)
{
    char query[512];
    char *temp1, *temp2;
    int result;

    int genre_id;

    if (strempty (title))
        return ERR_GENRE_TITLE_EMPTY;

    genre_id = genre_get_id_with_title (db, title);
    if (genre_id > 0)
        return ERR_GENRE_TITLE_EXIST;

    genre_id = genre_get_id_with_name (db, name);
    if (genre_id > 0)
        return ERR_GENRE_NAME_EXIST;

    temp1 = malloc (strlen (title) * 2 + 1);
    mysql_real_escape_string (db, temp1, title, strlen (title));

    if (strempty (name))
        temp2 = genre_create_name_from_title (title);
    else
    {
        temp2 = malloc (strlen (name) * 2 + 1);
        mysql_real_escape_string (db, temp2, name, strlen (name));
    }

    snprintf (query, 512,
              "INSERT INTO genres (title, name) VALUES (\"%s\", \"%s\")",
              temp1, temp2);

    free (temp1);
    free (temp2);

    result = mysql_query (db, query);
    if (result != 0)
        return ERR_QUERY_FAILED;

    return mysql_insert_id (db);
}


int
genre_remove_with_title (MYSQL *db, const char *title)
{
    char query[512];
    char *temp;
    int result;

    int genre_id;

    if (strempty (title))
        return ERR_GENRE_TITLE_EMPTY;

    genre_id = genre_get_id_with_title (db, title);
    if (genre_id <= 0)
        return ERR_GENRE_TITLE_DONT_EXIST;

    temp = malloc (strlen (title) * 2 + 1);
    mysql_real_escape_string (db, temp, title, strlen (title));

    snprintf (query, 512,
              "DELETE FROM genres WHERE title=\"%s\"", temp);

    free (temp);

    result = mysql_query (db, query);
    if (result != 0)
        return ERR_QUERY_FAILED;

    return mysql_insert_id (db);
}


int
genre_remove_with_name (MYSQL *db, const char *name)
{
    char query[512];
    char *temp;
    int result;

    int genre_id;

    if (strempty (name))
        return ERR_GENRE_NAME_EMPTY;

    genre_id = genre_get_id_with_name (db, name);
    if (genre_id <= 0)
        return ERR_GENRE_NAME_DONT_EXIST;

    temp = malloc (strlen (name) * 2 + 1);
    mysql_real_escape_string (db, temp, name, strlen (name));

    snprintf (query, 512,
              "DELETE FROM genres WHERE name=\"%s\"", temp);

    free (temp);

    result = mysql_query (db, query);
    if (result != 0)
        return ERR_QUERY_FAILED;

    return mysql_insert_id (db);
}
