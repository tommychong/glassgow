#ifndef REQUEST_H
#define REQUEST_H

#include <glib.h>

typedef struct GGHttpRequest  {
    char* method;
    char* uri;
    GString *body;
    gint body_len;
    GHashTable* headers;
} GGHttpRequest;

gchar* gg_get_request_header(GGHttpRequest *request, gchar *key);

void set_header(GGHttpRequest *request, gchar *key, gchar *value);


GGHttpRequest* gg_http_request_new();

void gg_http_request_free(GGHttpRequest *request);

int parse_http_request(char *data, GGHttpRequest *request);

#endif
