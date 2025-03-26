//
// Created by janek on 3/12/25.
//
#include <math.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "common.h"

int main() {
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons (9734);
    server_len = sizeof (server_addr);
    bind (server_sockfd, (struct sockaddr *) &server_addr, server_len);


    listen(server_sockfd, 5);

    while (1) {
        uint32_t request=0;
        uint32_t request_id=0;
        printf ("Waiting for client connection...\n");
        client_len = sizeof (client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (client_sockfd==-1){
            perror("Connection error!");
            continue;
        }
        printf ("Connection accepted\n");
        int end=0;
        while (!end){
            int res = read(client_sockfd, &request, sizeof(request));
            if (res==0){
                end = 1;
                continue;
            }
            if (res==-1){
                end = 1;
                continue;
            }
            request = ntohl(request);
            read(client_sockfd, &request_id, sizeof(request_id));
            request_id = ntohl(request_id);
            printf ("Request ID: %d\n", request_id);
            printf ("Request: %d\n", request);
            if (request==1) {
                double number = 0;
                read(client_sockfd, &number, sizeof(number));
                number = htond(number);
                double out = sqrt(number);
                out = ntohd(out);
                request |= 0x01000000;
                request = htonl(request);
                write(client_sockfd, &request, sizeof(request));
                write(client_sockfd, &request_id, sizeof(request_id));
                write(client_sockfd, &out, sizeof(out));
            }
            else if (request==2) {
                request |= 0x01000000;
                request = htonl(request);
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                char out[30];
                strftime(out, sizeof(out), "%Y-%m-%d %H:%M:%S", t);
                uint32_t out_len = strlen(out);
                out_len = htonl(out_len);
                write(client_sockfd, &request, sizeof(request));
                write(client_sockfd, &request_id, sizeof(request_id));
                write(client_sockfd, &out_len, sizeof(out_len));
                write(client_sockfd, &out, strlen(out));

            }
        }
        close(client_sockfd);
        printf("Connection with client ended!\n");
    }
}
