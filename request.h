#include "http-parser/http_parser.h"
#include "stdio.h"
#include <glib.h>

typedef struct ggHttpRequest {
    char* method;
    char* uri;
    char* body;
    gint body_len;
    GHashTable* headers;
    //headers;
    //http_version version;

} ggHttpRequest;

int parse_http_request(char *data, ggHttpRequest *request) {
    gchar **lines = g_strsplit(data, "\r\n", 0);

    //Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    gchar **request_line = g_strsplit(lines[0], " ", 0);
    request->method = request_line[0];
    request->uri = request_line[1];

    //g_strfreev();

    return 0;
}

void set_header(ggHttpRequest *request, gchar *key, gchar *value) {
    g_hash_table_insert(request->headers, key, value);
}

ggHttpRequest* gg_http_request_new() {
    ggHttpRequest *request = malloc(sizeof(ggHttpRequest));
    request->headers = g_hash_table_new(g_str_hash, g_str_equal);

    return request;
}


void gg_http_request_free(ggHttpRequest *request) {
    //Free the headers
    //free the request object itself
}
