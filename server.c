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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <errno.h>
#include <signal.h>

#define MAX_CLIENTS 100
volatile int client_sockets[MAX_CLIENTS]={0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
atomic_int closing = 0;

int is_closing(){
  return atomic_load(&closing);
}

int add_socket(int socket){
    int index = -1;
    pthread_mutex_lock(&clients_mutex);
    if (is_closing()){
      pthread_mutex_unlock(&clients_mutex);
      return -2;
    }

    for(int i = 0; i < MAX_CLIENTS; i++){
        if (client_sockets[i] == 0){
              index = i;
              client_sockets[index] = socket;
              break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    return index;
}

void remove_socket(int socket_index){
    if (socket_index<0 || socket_index>=MAX_CLIENTS) return;
    pthread_mutex_lock(&clients_mutex);
    client_sockets[socket_index] = 0;
    pthread_mutex_unlock(&clients_mutex);
}

void close_all_sockets(){
    atomic_store(&closing, 1);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_sockets[i] == 0) continue;
        shutdown(client_sockets[i], SHUT_RDWR);
        close(client_sockets[i]);
        client_sockets[i] = 0;
    }
    pthread_mutex_unlock(&clients_mutex);
    printf("Closing server...\n");
}


void handle_shutdown(int sig){
  close_all_sockets();
}

void *client_thread(void *arg){
    int client_sockfd = *(int*)arg;
    int client_index = ((int*)arg)[1];
    int end=0;
    uint32_t request=0;
    uint32_t request_id=0;
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
        if (request==1) {
            double number = 0;
            read(client_sockfd, &number, sizeof(number));
            number = ntohd(number);
            double out = sqrt(number);
            out = htond(out);
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
    if (!is_closing()){
        close(client_sockfd);
        remove_socket(client_index);
        printf("Connection with client ended!\n");
    }
    return NULL;
}


int main(int argc, char *argv[]) {
  	long port = 9734;
    char *end;
  	if (argc >= 2){
          port = strtol(argv[1], &end, 10);
		  if (end == argv[1] || port > 65535 || port<=0){
              printf("Invalid port number!\n");
              exit(1);
          }
  	}
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl (INADDR_ANY);
    server_addr.sin_port = htons ((uint16_t) port);
    server_len = sizeof (server_addr);
    bind (server_sockfd, (struct sockaddr *) &server_addr, server_len);


    listen(server_sockfd, 5);

    struct sigaction sa;
    sa.sa_handler = handle_shutdown;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    while (!is_closing()) {
        printf ("Waiting for client connection...\n");
        client_len = sizeof (client_addr);
        client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (client_sockfd==-1){
            if (errno != EINTR) perror("Connection error!");
            continue;
        }
        printf ("Connection accepted\n");
        int index = add_socket(client_sockfd);
        if (index == -1){
          perror("Maximum number of clients achieved!");
        }
        if (index<0){
            shutdown(client_sockfd, SHUT_RDWR);
            close(client_sockfd);
            continue;
        }

        int args[2];
        args[0] = client_sockfd;
        args[1] = index;
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, (void*)&args);
        pthread_detach(thread_id);
    }
}
