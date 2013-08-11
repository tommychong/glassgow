#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define MAX_QUEUE 20
#define RECEIVE_BUFFER_SIZE 4096

typedef enum {GET, POST, PUT, DELETE} GG_HTTP_METHODS;

typedef enum {HTTP_1_0, HTTP_1_1} http_version;

typedef struct ggHttpResponse {
    unsigned int response_code;
    char *body;
    //headers;
    http_version version;
} ggHttpResponse;

typedef struct RouteEntry {
    char* route_pattern;
    void (*handler)(ggHttpResponse*);
} RouteEntry;


char* marshall_response (ggHttpResponse *response){
    char *buffer = (char*) malloc(4096);
    char *cursor = buffer;
    int msg_len = strlen(response->body);
    sprintf(buffer, "HTTP/1.0 200 OK\r\nContent-Length: %d\r\n\r\n%s\r\n", msg_len, response->body);

    return buffer;
}

int server_app (RouteEntry* routes) {
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

        struct ggHttpResponse resp;
        resp.response_code = 200;

        //int n = sizeof(routes)/ sizeof(RouteEntry);
        int n = 2;
        for (int i=0; i<n; i++) {
            if(1){
                printf("%d of %d\n",i,n);
                routes[i].handler(&resp);
                break;
            }
        }

        char *send_buf = marshall_response(&resp);
        printf("%s",send_buf);

        send(client_fd, send_buf, strlen(send_buf), 0);
        free(send_buf);
    }

    return 0;
}
