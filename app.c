#include "server.c"

unsigned long get_file_length (FILE *file){
    unsigned long length;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    return length;
}

void gg_file_handler(ggHttpResponse* response, gchar *segment){
    FILE *file;
    char *buffer;
    unsigned long file_size;

    //Check if you can access with access() and also 404/500 on read error
    file = fopen(segment, "rb");
    if(!file) {
        gg_write(response, "This should be an actual 404!");
        response->status = 404;
        return;
    }

    file_size = get_file_length(file);

    printf("File SIZE: %lu\n", file_size);

    buffer = (char*) malloc(file_size+1); //TODO: create a gg_malloc function that cleans up after response done

    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';
    fclose(file);

    gg_set_response_header_num(response, "Content-Length", (gint) file_size);

    if (strstr(segment, "html")) {
        gg_set_response_header(response, "Content-Type", "text/html");
    } else if (strstr(segment, "jpg")) {
        gg_set_response_header(response, "Content-Type", "image/jpeg");
    } else if (strstr(segment, "ico")) {
        gg_set_response_header(response, "Content-Type", "image/x-icon");
    }

    gg_write_len(response, buffer, file_size);
    free(buffer);
}

void gg_null_handler(ggHttpResponse* response, gchar *segment){ //TODO: there has to be a nicer way to do this... printf style optional args?
    static char* thug = "<html><head><style>body { font-family: Arial; background-color: #EFF; }</style></head><body>Thugination Extreme edition</body></html>";
    gg_write(response, thug);
}

int main(int argc, char *argv[]) {
    RouteEntry routes[] = {
                {"/(.+)", gg_file_handler},
                {"/", gg_null_handler},
                {NULL, NULL}
                };

    gchar *port = "8001";
    if (argc == 2){
        port = argv[1];
    }
    server_app(routes, port);
    //gg_app app = server_app(routes);
    //app.listen(8001);
    //gg_app_listen(app);

    return 0;
}
