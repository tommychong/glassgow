#ifndef RESPONSE_H
#define RESPONSE_H

#include <glib.h>

typedef struct GGHttpResponse {
    unsigned int status;
    GString *body;
    GHashTable* headers;
} GGHttpResponse;

gchar* gg_status_code_to_message (guint status);

void gg_write (GGHttpResponse *response, char *chunk);

void gg_write_len (GGHttpResponse *response, char *chunk, guint len);

gchar* gg_get_response_header(GGHttpResponse *response, gchar *key);

void gg_set_response_header(GGHttpResponse *response, gchar *key, gchar *value);

void gg_set_response_header_num(GGHttpResponse *response, gchar *key, gint value);

GGHttpResponse* gg_http_response_new();

void gg_http_response_free(GGHttpResponse *response);

#endif
