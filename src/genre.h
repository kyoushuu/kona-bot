/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * genre.h
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

#define ERR_GENRE_TITLE_EMPTY           -0x00010001
#define ERR_GENRE_NAME_EMPTY            -0x00010002
#define ERR_GENRE_TITLE_EXIST           -0x00010003
#define ERR_GENRE_TITLE_DONT_EXIST      -0x00010004
#define ERR_GENRE_NAME_EXIST            -0x00010005
#define ERR_GENRE_NAME_DONT_EXIST       -0x00010006

int genre_get_id_with_title (MYSQL *db, const char *title);
int genre_get_id_with_name (MYSQL *db, const char *name);
int genre_add (MYSQL *db, const char *title, const char *name);
int genre_remove_with_title (MYSQL *db, const char *title);
int genre_remove_with_name (MYSQL *db, const char *name);
