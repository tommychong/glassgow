#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define MAX_QUEUE 20

typedef enum {HTTP_1_0, HTTP_1_1} http_version;

struct gg_http_response {
    unsigned int response_code;
    char *body;
    //headers;
    http_version version;
};

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
    }

    listen(s_fd, MAX_QUEUE);

    static const char* thing = "HTTP/1.0 200 OK\nContent-Length: 228\n\n<html><body>Helloworld<br/>Hello say the world<br/>Testing testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testingTesting testing testing testing</body></html>\n";
    while(1) {
        char buf[9];
        buf[8] = '\0';
        socklen_t sin_size = sizeof client_addr;
        int client_fd = accept(s_fd, (struct sockaddr *)&client_addr, &sin_size);
        printf("Accepted\n");

//int readed = read(client_fd, buf, 8);
        recv(client_fd, buf, 8, 0);
        printf("Got msgs %s\n",buf);

        send(client_fd, thing, strlen(thing), 0);
        printf("What do?\n");
    }

    return 0;
}
