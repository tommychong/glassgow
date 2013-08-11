#include "server.c"


unsigned long get_file_length (FILE *file){
    unsigned long length;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    return length;
}

//void gg_file_handler(Request* request, Response* response) {
void gg_file_handler(ggHttpResponse* response){
    FILE *file;
    char *buffer = (char*) malloc(2048);
    unsigned long file_size;

    file = fopen("index.html", "rb");
    file_size= get_file_length(file);

    fread(buffer, file_size, 1, file);
    buffer[file_size] = '\0';
    fclose(file);

    response->body = buffer;
}

//void gg_null_handler(Request* request, Response* response) {
void gg_null_handler(ggHttpResponse* response){
    static char* thug = "<html><head><style>body { font-family: Arial; background-color: #EFF; }</style></head><body>Thugination Extreme edition</body></html>";
    response->body = thug;
}

int main(void) {
    RouteEntry routes[] = {
                {"/favicon.ico", &gg_file_handler},
                {"/", &gg_null_handler}
               };
    server_app(routes);
    //gg_app app = server_app(routes);
    //app.listen(8001);
    //gg_app_listen(app);

    return 0;
}
