#include "server.h"
#include "request.h"
#include "response.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>

#include <ev.h>

#include <time.h>

#define MAX_QUEUE 20
#define RECEIVE_BUFFER_SIZE 4096

typedef struct GGAppContext {
    gchar *static_file_path;
    RouteEntry *routes;
} GGAppContext;

enum {
    WAITING_FOR_HEADERS
} RequestParseState;

/*
 * there isn't a way to pass context to libev callbacks, which only pass the watcher,
 * so we wrap the watcher with context
 */

typedef struct ServerEvContext {
    ev_io watcher;
    GGAppContext *app;
} ServerEvContext;

typedef struct ClientEvContext{
    ev_io watcher;
    GString *read_buffer;
    GGAppContext *app;
} ClientEvContext;

static inline GString* marshall_response (GGHttpResponse *response){
    //TODO: what's the best default allocation size for the string?
    GString *response_string = g_string_sized_new(1024);
    
    g_string_append_printf (response_string, "HTTP/1.1 %d %s\r\n", response->status, gg_status_code_to_message(response->status));

    if (!gg_get_response_header(response, "Content-Length")) {
        int msg_len = response->body->len;
        gg_set_response_header_num(response, "Content-Length", msg_len);
    }

    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init (&iter, response->headers);

    while (g_hash_table_iter_next (&iter, &key, &value)) {
        g_string_append_printf (response_string, "%s: %s\r\n", key, value);
    }

    g_string_append(response_string, "\r\n");
    g_string_append_len(response_string, response->body->str, response->body->len);

    return response_string;
}

static inline int set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1) {
        flags = 0;
    }

    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}     

static inline void truncate_buffer (GString *buffer, gchar *term_point, guint len) {
    *term_point = '\0';

    //Relying on a bit of trickery here, strlen will get the length to truncate to,
    //len will be any additional length we wish to strip off
    g_string_erase(buffer, 0, strlen(buffer->str)+len);
}

static void client_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    ClientEvContext *client = (ClientEvContext*) watcher;
    GGAppContext *app = client->app;

    int client_fd = watcher->fd;

    char buf[RECEIVE_BUFFER_SIZE];

    int recv_size = recv(client_fd, buf, RECEIVE_BUFFER_SIZE, 0);

    if (recv_size <= 0) {
        ev_io_stop(loop, watcher);
        shutdown(client_fd, 2);
        //TODO: abstract out to a free function later, once we have more stuff to actually free
        g_string_free(client->read_buffer, TRUE);
        free(client);
        return;
    }

    g_string_append_len (client->read_buffer, buf, recv_size);

    //printf("Got msg chunk\nsize:%d\nReceive buffer size: %lu, contents:\n%s\n", recv_size, client->read_buffer->len, client->read_buffer->str);

    char *header_terminator;
    while (header_terminator = strstr(client->read_buffer->str, "\r\n\r\n")) {
        GGHttpRequest *request = gg_http_request_new();
        parse_http_request(client->read_buffer->str, request);

        truncate_buffer(client->read_buffer, header_terminator, 4);

        //printf("Trunc'd buffer!\nReceive buffer size: %lu, contents:\n%s\n", client->read_buffer->len, client->read_buffer->str);

        GGHttpResponse *response = gg_http_response_new();
        response->status = 200;

        gboolean handled = FALSE;
        for (RouteEntry *route = app->routes; !handled && route->route_pattern != NULL; route++) {
            //TODO: pre-compile the regexes before we start serving
            GRegex *route_regex;
            route_regex = g_regex_new(route->route_pattern, 0, 0, NULL);
            GMatchInfo *match_info;

            if(g_regex_match(route_regex, request->uri, 0, &match_info)){
                gchar *matched_segment = g_match_info_fetch(match_info ,1);
                printf("Serving up handler for %s\n", route->route_pattern);

                route->handler(request, response, matched_segment);

                g_free(matched_segment);
                handled = TRUE;
            }

            g_match_info_free(match_info);
            g_regex_unref(route_regex);
        }

        GString *send_buf = marshall_response(response);

        gulong total_sent = 0;
        gulong remaining = send_buf->len;

        do {
            int bytes_sent = send(client_fd, send_buf->str + total_sent, remaining, 0);

            if(bytes_sent == -1){
                //Socket might not be ready to send stuff all the time, make good use of libev here somehow?
                continue;
            }

            total_sent += bytes_sent;
            remaining = send_buf->len - total_sent;
            //printf("SENT %lu/%lu bytes! rem %lu\n", sent_size, send_buf->len, remaining);
        } while (remaining);
        //printf("COMPLETE SEND\n");


        g_string_free(send_buf, TRUE);
        gg_http_request_free(request);
        gg_http_response_free(response);
    }

}

static void server_cb(struct ev_loop *loop, ev_io *watcher, int revents) {
    ServerEvContext *server = (ServerEvContext*) watcher;

    while(1) {
        int client_fd = accept(watcher->fd, NULL, NULL);
        printf("Accept %d!\n", client_fd);
        
        if(client_fd < 0){
            break;
        }

        ClientEvContext *client_ev_context = (ClientEvContext*) malloc(sizeof(ClientEvContext));
        client_ev_context->app = server->app;
        client_ev_context->read_buffer = g_string_sized_new(1024);
        ev_io *client_io_watcher = (ev_io*) &client_ev_context->watcher;

        set_non_blocking(client_fd);

        ev_io_init(client_io_watcher, client_cb, client_fd, EV_READ);
        ev_io_start (loop, client_io_watcher);
    }
}

int create_socket_and_bind(gchar *port) {
    struct addrinfo hints, *res;
    int server_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Check for error
    getaddrinfo(NULL, port, &hints, &res);

    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    set_non_blocking(server_fd);

    int stat = bind(server_fd, res->ai_addr, res->ai_addrlen);

    if (stat != 0) {
        printf("ERROR! Cannot bind to address\n");
        return -1;
    }

    return server_fd;
}

int gg_server_app (RouteEntry *routes, gchar *port) {
    GGAppContext app;
    app.static_file_path = "static/";
    app.routes = routes;

    int server_fd = create_socket_and_bind(port);

    if (server_fd == -1) {
        printf("Could not start server on port %s.\n", port);
        return -1;
    }

    listen(server_fd, MAX_QUEUE);
    printf("Glassgow started on port %s.\n", port);

    struct ev_loop *loop = EV_DEFAULT;

    ServerEvContext server_ev_context;
    server_ev_context.app = &app;
    ev_io *server_io_watcher = (ev_io*) &server_ev_context.watcher;

    ev_io_init (server_io_watcher, server_cb, server_fd, EV_READ);
    ev_io_start (loop, server_io_watcher);

    ev_run (loop, 0);

    return 0;
}
