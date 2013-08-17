#include "request.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

gchar* gg_get_request_header(GGHttpRequest *request, gchar *key) {
    return g_hash_table_lookup(request->headers, key);
}

void set_header(GGHttpRequest *request, gchar *key, gchar *value) {
    g_hash_table_insert(request->headers, g_strdup(key), g_strdup(value));
}

GGHttpRequest* gg_http_request_new() {
    GGHttpRequest *request = malloc(sizeof(GGHttpRequest));
    request->headers = g_hash_table_new(g_str_hash, g_str_equal);

    return request;
}

void gg_http_request_free(GGHttpRequest *request) {
    g_hash_table_destroy(request->headers);
    //g_string_free(request->body, TRUE);
    g_free(request->method);
    g_free(request->uri);
    free(request);
}

int parse_http_request(char *data, GGHttpRequest *request) {
    gchar **lines = g_strsplit(data, "\r\n", 0);

    //Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
    gchar **request_line = g_strsplit(lines[0], " ", 0);
    printf("Req:%s\n",lines[0]);

    if (g_strv_length (request_line) != 3 || !g_str_has_prefix(request_line[2], "HTTP/")){
        g_strfreev(request_line);
        g_strfreev(lines);
        return -1;
    }

    request->method = g_strdup(request_line[0]);
    request->uri = g_strdup(request_line[1]);
    g_strfreev(request_line);

    //Headers

    for(int i = 1; i < g_strv_length (lines); i++){
        if (strlen(lines[i]) > 0) {
            gchar **header = g_strsplit(lines[i], ": ", 0);
            //printf("SETTING HEADERS \"%s\":\"%s\"", header[0], header[1]);
            set_header(request, header[0], header[1]);
            g_strfreev(header);
        }
    }

    g_strfreev(lines);

    return 0;
}
