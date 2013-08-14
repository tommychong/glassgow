#include "http-parser/http_parser.h"
#include "stdio.h"
#include <glib.h>

typedef struct ggHttpRequest {
    char* method;
    char* uri;
    char* body;
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
