/*
 * $Id$
 *
 * xcap module - XCAP operations module
 *
 * Copyright (C) 2012 AG Projects
 *
 * This file is part of opensips, a free SIP server.
 *
 * opensips is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * opensips is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdlib.h>
#include <string.h>
#include "../../dprint.h"
#include "../../ut.h"
#include "uri.h"


int parse_xcap_uri(const str *uri, xcap_uri_t *xcap_uri)
{
    char *ns_ptr, *tree_ptr, *tmp;
    str unescaped_uri;

    if (uri == NULL || uri->s == NULL ||xcap_uri == NULL) {
        return -1;
    }

    if (uri->len > MAX_URI_SIZE-1) {
        LM_ERR("XCAP URI is too long\n");
        return -1;
    }

    memset(xcap_uri, 0, sizeof(xcap_uri_t));

    unescaped_uri.s = xcap_uri->buf;
    if (un_escape((str *)uri, &unescaped_uri) < 0) {
        LM_ERR("Error un-escaping XCAP URI\n");
        return -1;
    }
    xcap_uri->buf[uri->len] = '\0';

    xcap_uri->uri.s = xcap_uri->buf;
    xcap_uri->uri.len = uri->len;

    /* selector */
    if ((ns_ptr = strstr(xcap_uri->uri.s, "/~~/"))) {
        xcap_uri->selector.s = ns_ptr+3;
        xcap_uri->selector.len = xcap_uri->uri.s+xcap_uri->uri.len - xcap_uri->selector.s;
    }

    /* tree */
    if ((tree_ptr = strstr(xcap_uri->uri.s, "/global/"))) {
        xcap_uri->tree.s = tree_ptr+1;
        xcap_uri->tree.len = 6;
    } else if ((tree_ptr = strstr(xcap_uri->uri.s, "/users/"))) {
        xcap_uri->tree.s = tree_ptr+1;
        xcap_uri->tree.len = 5;
    } else {
        LM_ERR("Unknown XCAP URI tree\n");
        return -1;
    }

    /* AUID */
    tmp = tree_ptr-1;
    while (tmp > xcap_uri->uri.s) {
        if (tmp[0] == '/')
            break;
        tmp--;
    }
    if (tmp < xcap_uri->uri.s) {
        LM_ERR("Error parsing AUID\n");
        return -1;
    }
    xcap_uri->auid.s = tmp+1;
    xcap_uri->auid.len = tree_ptr-1 - tmp;

    /* XCAP root */
    xcap_uri->root.s = xcap_uri->uri.s;
    xcap_uri->root.len = xcap_uri->auid.s - xcap_uri->root.s;

    /* XUI */
    xcap_uri->xui.s = xcap_uri->tree.s+xcap_uri->tree.len+1;
    tmp = xcap_uri->xui.s;
    while (*tmp) {
        if (tmp[0] == '/')
            break;
        tmp++;
    }
    if (tmp >= xcap_uri->uri.s+xcap_uri->uri.len) {
        LM_ERR("Error parsing XUI\n");
        return -1;
    }
    xcap_uri->xui.len = tmp-xcap_uri->xui.s;

    /* filename */
    xcap_uri->filename.s = xcap_uri->xui.s+xcap_uri->xui.len+1;
    xcap_uri->filename.len = (ns_ptr ? ns_ptr : xcap_uri->uri.s+xcap_uri->uri.len)-xcap_uri->filename.s;
    return 0;
}


