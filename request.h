#ifndef REQUEST_H
#define REQUEST_H

#include <glib.h>

typedef struct ggHttpRequest  {
    char* method;
    char* uri;
    GString *body;
    gint body_len;
    GHashTable* headers;
} ggHttpRequest;

void set_header(ggHttpRequest *request, gchar *key, gchar *value);

ggHttpRequest* gg_http_request_new();

void gg_http_request_free(ggHttpRequest *request);

int parse_http_request(char *data, ggHttpRequest *request);

#endif
