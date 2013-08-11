#include "server.c"

/*
unsigned long get_file_length (FILE *file){
    unsigned long length;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    fseek(file, 0, SEEK_SET);

    return length;
}

void file_handler() {
    FILE *file;
    char *buffer;
    unsigned long fileBytes;

    file = fopen(path, "rb");
    fileBytes = get_file_length(file);
    buffer = (char *) malloc(fileBytes + 2048);
}
*/

//void gg_file_handler(Request* request, Response* response) {
void gg_file_handler(ggHttpResponse* response){
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

    return 0;
}
