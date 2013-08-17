#include "server.h"
#include "response.h"
#include "request.h"

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

unsigned long get_file_length (FILE *file){
    unsigned long length;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    return length;
}

void gg_file_handler(GGHttpRequest* request, GGHttpResponse* response, gchar *segment){
    FILE *file;
    char *buffer;
    unsigned long file_size;

    //Check if you can access with access() and also 404/500 on read error
    file = fopen(segment, "rb");

    //TODO: check if its a directory, and if its a directory...
    if(!file) {
        gg_write(response, "No resource here homie.");
        response->status = 404;
        return;
    }

    file_size = get_file_length(file);

    buffer = (char*) malloc(file_size+1); //TODO: create a gg_malloc function that cleans up after response done

    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';
    fclose(file);

    gg_set_response_header_num(response, "Content-Length", (gint) file_size);

    if (strstr(segment, "html")) {
        gg_set_response_header(response, "Content-Type", "text/html");
    } else if (strstr(segment, "jpg")) {
        gg_set_response_header(response, "Content-Type", "image/jpeg");
    } else if (strstr(segment, "gif")) {
        gg_set_response_header(response, "Content-Type", "image/gif");
    } else if (strstr(segment, "ico")) {
        gg_set_response_header(response, "Content-Type", "image/x-icon");
    }

    GChecksum *checksum = g_checksum_new(G_CHECKSUM_SHA1);
    g_checksum_update (checksum, (guchar*) buffer, file_size);

    gchar checksum_string [64];
    sprintf(checksum_string, "\"%s\"", g_checksum_get_string(checksum));
    gg_set_response_header(response, "ETag", checksum_string);

    g_checksum_free(checksum);

    gchar *if_none_match = gg_get_request_header(request, "If-None-Match");
    //printf("SUPGIRL:%s\n",if_none_match);

    if (if_none_match && strcmp(if_none_match, checksum_string) == 0){
        response->status = 304;
    } else {
        gg_write_len(response, buffer, file_size);
    }

    printf("Serving hot n fresh: %s\n", segment);
    free(buffer);
}

void gg_swag_handler(GGHttpRequest* request, GGHttpResponse* response, gchar *segment){
    static const char* thug = "<html><head><style>body { font-family: Arial; background-color: #FFE; }</style></head><body>Swaggamuffin</body></html>";
    gg_write(response, (char*) thug);
}

void gg_null_handler(GGHttpRequest* request, GGHttpResponse* response, gchar *segment){ //TODO: there has to be a nicer way to do this... printf style optional args?
    static const char* thug = "<html><head><style>body { font-family: Arial; background-color: #EFF; }</style></head><body>Thugination Extreme edition</body></html>";
    gg_write(response, (char*) thug);
}

int main(int argc, char *argv[]) {
    RouteEntry routes[] = {
                {"/swag/", gg_swag_handler},
                {"/(.+)", gg_file_handler},
                {"/", gg_null_handler},
                ROUTES_END
                };

    gchar *port = "8001";
    if (argc == 2){
        port = argv[1];
    }

    gg_server_app(routes, port);

    return 0;
}
