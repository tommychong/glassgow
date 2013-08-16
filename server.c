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

#define MAX_QUEUE 20
#define RECEIVE_BUFFER_SIZE 4096

GString* marshall_response (GGHttpResponse *response){
    //TODO: what's the best default allocation size for the string?
    GString *response_string = g_string_sized_new(1024);
    
    g_string_append_printf (response_string, "HTTP/1.0 %d %s\r\n", response->status, gg_status_code_to_message(response->status));

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

RouteEntry *routes;

ev_io server_io_watcher;

int set_non_blocking(int fd)
{
    int flags;

    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
        flags = 0;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}     

static void client_cb(EV_P_ ev_io *w, int revents) {
    int client_fd = w->fd;

    printf("I AM IN THE CLIENT CB\n");
    char buf[RECEIVE_BUFFER_SIZE];

    int recv_size = recv(w->fd, buf, RECEIVE_BUFFER_SIZE-1, 0);

    if (recv_size == 0) {
        ev_io_stop(EV_A_ w);
        return;
    } else if (recv_size < 0) {
        printf("Error! Could not receive packet from port\n");
        return;
    }

    buf[recv_size] = '\0';
    printf("Got msg\r\nsize:%d\r\n%s\r\n", recv_size, buf);

    GGHttpRequest *request = gg_http_request_new();
    parse_http_request(buf, request);

    GGHttpResponse *response = gg_http_response_new();
    response->status = 200;

    gboolean handled = FALSE;
    for (RouteEntry *route = routes; !handled && route->route_pattern != NULL; route++) {
        //TODO: pre-compile the regexes before we start serving
        GRegex *route_regex;
        route_regex = g_regex_new(route->route_pattern, 0, 0, NULL);
        GMatchInfo *match_info;
        //printf("dodeting %s\n", route->route_pattern);

        if(g_regex_match(route_regex, request->uri, 0, &match_info)){
            gchar *matched_segment = g_match_info_fetch(match_info ,1);
            printf("Serving up handler for %s\n", route->route_pattern);

            route->handler(response, matched_segment);

            g_free(matched_segment);
            handled = TRUE;
        }

        g_match_info_free(match_info);
        g_regex_unref(route_regex);
    }

    GString *send_buf = marshall_response(response);
    printf("%s", send_buf->str);

    send(client_fd, send_buf->str, send_buf->len, 0);

    g_string_free(send_buf, TRUE);
    gg_http_request_free(request);
    gg_http_response_free(response);
}

static void server_cb(struct ev_loop *loop, ev_io *w, int revents) {
    printf("Hey server got stuff~\n");
    while(1) {
        int client_fd = accept(w->fd, NULL, NULL);
        printf("Accept! %d\n", client_fd);
        
        if(client_fd < 0){
            break;
        }

        ev_io *client_io_watcher = (ev_io*) malloc(sizeof(ev_io));
        set_non_blocking(client_fd);

        ev_io_init(client_io_watcher, client_cb, client_fd, EV_READ);
        ev_io_start (loop, client_io_watcher);
    }
}

int gg_server_app (RouteEntry *routez, gchar *port) {
    routes = routez;
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

    set_non_blocking(s_fd);

    int stat = bind(s_fd, res->ai_addr, res->ai_addrlen);

    if (stat != 0) {
        printf("ERROR! Cannot bind to address\n");
        return -1;
    }

    listen(s_fd, MAX_QUEUE);
    printf("Glassgow started on port %s.\n", port);

    struct ev_loop *loop = EV_DEFAULT;

    ev_io_init (&server_io_watcher, server_cb, s_fd, EV_READ);
    ev_io_start (loop, &server_io_watcher);

    ev_run (loop, 0);

    return 0;
}
