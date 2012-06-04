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

#include <stdlib.h>

#include "duda_package.h"
#include "sha1.h"

static void sha1_encode (const void *dataIn, unsigned char *dataOut,
                         unsigned long length)
{
    SHA_CTX sha;
    SHA1_Init(&sha);
    SHA1_Update(&sha, dataIn, length);
    SHA1_Final(dataOut, &sha);
}

struct duda_api_sha1 *get_sha1_api()
{
    struct duda_api_sha1 *sha1;

    /* Alloc object */
    sha1 = malloc(sizeof(struct duda_api_sha1));

    /* Map API calls */
    sha1->encode = sha1_encode;

    return sha1;
}

duda_package_t *duda_package_main(void **api)
{
    duda_package_t *dpkg;

    /* Initialize package internals */
    duda_package_init();

    /* Package object */
    dpkg = mk_api->mem_alloc(sizeof(duda_package_t));
    dpkg->name = "sha1";
    dpkg->version = "0.1";
    dpkg->api = get_sha1_api();

    return dpkg;
}
