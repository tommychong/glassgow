#ifndef SERVER_H
#define SERVER_H

#include <glib.h>
#include "response.h"

typedef enum {GET, POST, PUT, DELETE} GG_HTTP_METHODS;

typedef enum {HTTP_1_0, HTTP_1_1} http_version;

typedef struct RouteEntry {
    char* route_pattern;
    void (*handler)(GGHttpResponse*, gchar*);
} RouteEntry;

int gg_server_app (RouteEntry *routes, gchar *port);

#endif
