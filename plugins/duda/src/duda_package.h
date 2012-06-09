/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*  Monkey HTTP Daemon
 *  ------------------
 *  Copyright (C) 2001-2012, Eduardo Silva P.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DUDA_PACKAGE_H
#define DUDA_PACKAGE_H

#include <stdlib.h>
#include "MKPlugin.h"

/* Plugin events thread key */
pthread_key_t duda_package_event_k;

struct duda_package {
    char *name;
    char *version;

    void *api;
    
    /* mk_list */
    struct mk_list _head;
};

struct package_event{
    int socket;
    struct duda_package *handler;
    
    /* mk_list */
    struct mk_list _head;
};

/* Package events interface */
int duda_package_event_add(int socket, int mode,
                        struct plugin *handler,
                        struct client_session *cs,
                        struct session_request *sr,
                        int behavior);
int duda_package_event_del(int socket);

/* Reference the duda API */
struct duda_api_objects *duda_api;
#define duda_package_init() duda_api = duda_api_master();

/* Data type */
typedef struct duda_package duda_package_t;

/* Define package loader */
duda_package_t *duda_package_load(const char *pkgname);
#endif
