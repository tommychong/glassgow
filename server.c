#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define MAX_QUEUE 20
#define RECEIVE_BUFFER_SIZE 4096

typedef enum {HTTP_1_0, HTTP_1_1} http_version;

struct gg_http_response {
    unsigned int response_code;
    char *body;
    //headers;
    http_version version;
};

char* marshall_response (struct gg_http_response *response){
    char *buffer = (char*) malloc(4096);
    char *cursor = buffer;
    int msg_len = strlen(response->body);
    sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n\r\n%s\r\n", msg_len, response->body);

    return buffer;
}

int main (void){
    struct sockaddr_storage client_addr;
    struct addrinfo hints, *res;
    int s_fd = 0;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //Check for error
    getaddrinfo(NULL, "8001", &hints, &res);

    s_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int stat = bind(s_fd, res->ai_addr, res->ai_addrlen);

    if (stat != 0){
        printf("ERROR! Cannot bind to address\n");
        return -1;
    }

    listen(s_fd, MAX_QUEUE);
    printf("Server active on port 8001.\n");

    static char* thug = "<html><head><style>body { font-family: Arial; background-color: #EFF; }</style></head><body>Thugination Extreme edition</body></html>";

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

        struct gg_http_response resp;
        resp.body = thug;

        char *send_buf = marshall_response(&resp);
        printf("%s",send_buf);

        send(client_fd, send_buf, strlen(send_buf), 0);
        free(send_buf);
    }

    return 0;
}
