#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define MAX_QUEUE 20

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
        printf("ERROR!\n");
    }

    listen(s_fd, MAX_QUEUE);

    while(1) {
        char buf[9];
        buf[8] = '\0';
        socklen_t sin_size = sizeof client_addr;
        int client_fd = accept(s_fd, (struct sockaddr *)&client_addr, &sin_size);
        printf("Accepted\n");

//int readed = read(client_fd, buf, 8);
        recv(client_fd, buf, 8, 0);
        printf("Got msgs %s\n",buf);
    }

    return 0;
}
