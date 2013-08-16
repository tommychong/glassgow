#ifndef RESPONSE_H
#define RESPONSE_H

#include <glib.h>

typedef struct ggHttpResponse {
    unsigned int status;
    GString *body;
    GHashTable* headers;
} ggHttpResponse;

gchar* gg_status_code_to_message (guint status);

void gg_write (ggHttpResponse *response, char *chunk);

void gg_write_len (ggHttpResponse *response, char *chunk, guint len);

gchar* gg_get_response_header(ggHttpResponse *response, gchar *key);

void gg_set_response_header(ggHttpResponse *response, gchar *key, gchar *value);

void gg_set_response_header_num(ggHttpResponse *response, gchar *key, gint value);

ggHttpResponse* gg_http_response_new();

void gg_http_response_free(ggHttpResponse *response);

#endif
