#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "request.h"
#include "response.h"

#define MAX_QUEUE 20
#define RECEIVE_BUFFER_SIZE 4096

typedef enum {GET, POST, PUT, DELETE} GG_HTTP_METHODS;

typedef enum {HTTP_1_0, HTTP_1_1} http_version;

typedef struct RouteEntry {
    char* route_pattern;
    void (*handler)(ggHttpResponse*, gchar*);
} RouteEntry;

GString* marshall_response (ggHttpResponse *response){
    int msg_len = strlen(response->body);
    //TODO: what's the best default allocation size for the string?
    GString *response_string = g_string_sized_new(1024);
    
    g_string_append_printf (response_string, "HTTP/1.0 %d %s\r\n", response->status, gg_status_code_to_message(response->status));

    if (!gg_get_response_header(response, "Content-Length")) {
        gg_set_response_header_num(response, "Content-Length", msg_len);
    }

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, response->headers);

    while (g_hash_table_iter_next (&iter, &key, &value)) {
        g_string_append_printf (response_string, "%s: %s\r\n", key, value);
    }

    g_string_append(response_string, "\r\n");
     
    if (response->body_len >= 0) {
        g_string_append_len(response_string, response->body, response->body_len);
    } else {
        g_string_append(response_string, response->body);
        g_string_append(response_string, "\r\n");
    }

    return response_string;
}

int server_app (RouteEntry *routes, gchar *port) {
    struct sockaddr_storage client_addr;
    struct addrinfo hints, *res;
    int s_fd = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Check for error
    getaddrinfo(NULL, port, &hints, &res);

    s_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int stat = bind(s_fd, res->ai_addr, res->ai_addrlen);

    if (stat != 0) {
        printf("ERROR! Cannot bind to address\n");
        return -1;
    }

    listen(s_fd, MAX_QUEUE);
    printf("Glassgow started on port %s.\n", port);

    while(1) {
        char buf[RECEIVE_BUFFER_SIZE];
        socklen_t sin_size = sizeof client_addr;
        int client_fd = accept(s_fd, (struct sockaddr *)&client_addr, &sin_size);
        printf("Accepted\n");

        int recv_size = recv(client_fd, buf, RECEIVE_BUFFER_SIZE-1, 0);

        if (recv_size < 0){
            printf("Error! Could not receive packet from port\n");
        }

        buf[recv_size] = '\0';
        printf("Got msg\r\nsize:%d\r\n%s\r\n", recv_size, buf);

        ggHttpRequest *request = gg_http_request_new();
        parse_http_request(buf, request);

        ggHttpResponse *resp = gg_http_response_new();
        resp->status = 200;

        int n = 2;
        for (int i=0; i<n; i++) {
            //TODO: pre-compile the regexes before we start serving
            GRegex *route_regex;
            route_regex = g_regex_new(routes[i].route_pattern, 0, 0, NULL);
            GMatchInfo *match_info;

            if(g_regex_match(route_regex, request->uri, 0, &match_info)){
                gchar *matched_segment = g_match_info_fetch(match_info ,1);
                printf("Serving up handler for %s\n", routes[i].route_pattern);

                routes[i].handler(resp, matched_segment);

                g_free(matched_segment);
                break;
            }

            g_match_info_free(match_info);
            g_regex_unref(route_regex);
        }

        GString *send_buf = marshall_response(resp);
        printf("%s", send_buf->str);

        send(client_fd, send_buf->str, send_buf->len, 0);
        //TODO free request and response objects
        //free(send_buf);
        g_string_free(send_buf, TRUE);
        //gg_http_request_free(request);
        gg_http_response_free(resp);
    }

    return 0;
}
