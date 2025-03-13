#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

int main (int argc, char *argv[]) {
    if (argc != 2) {
        perror("Wrong number of arguments. Use ./client <host>\n");
        exit(1);
    }
    int sockfd;
    socklen_t len;
    struct sockaddr_in address;
    sockfd = socket (AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_port = htons(9734);
    address.sin_addr.s_addr = inet_addr(argv[1]);
    if (address.sin_addr.s_addr == INADDR_NONE) {
        perror("Address not recognized");
        exit(1);
    }

    len = sizeof(address);
    int result = connect(sockfd, (struct sockaddr *)&address, len);
    if (result == -1) {
        perror("Connection error! Check if server is running.\n");
        exit(2);
    }

    int exit = 0;
    enum {SQRT, TIME} mode;
    char command[20];
    uint32_t count = 0;
    while (! exit) {
        //processing user input
        fgets(command, sizeof(command), stdin);
        if (strncmp(command, "exit", 4) == 0) {
            exit = 1;
            continue;
        }
        if (strncmp(command, "quit", 4) == 0) {
            exit = 1;
            continue;
        }

        if (strncmp(command, "time", 4) == 0) {
            mode = TIME;
        }
        else if (strncmp(command, "sqrt", 4) == 0) {
            mode = SQRT;
        }
        else {
            perror("Wrong command. Use: exit or time or sqrt <number>\n");
            continue;
        }

        double number = 0;
        if (mode == SQRT) {
            errno=0;
            number = strtod(command+4, NULL);
            if (errno != 0) {
                perror("Error occurred while processing the number. Use: sqrt <number>\n");
                continue;
            }
        }

        //sending request
        uint32_t request = (mode == SQRT) ? 1 : 2;
        request = htonl(request);
        uint32_t request_id = htonl(count);

        int res = write(sockfd, &request, sizeof(request));
        if (res==-1 || res==0){
            perror("Connection error!\nCheck if server is runing.\n");
            exit=1;
        }
        write(sockfd, &request_id, sizeof(request_id));

        if (mode == SQRT) {
            write(sockfd, &number, sizeof(number));
        }

        //getting response
        uint32_t response = 0;
        uint32_t response_id = 0;

        read(sockfd, &response, sizeof(response));
        response = ntohl(response);
        request = ntohl(request);
        if (response != (request | 0x01000000)) {
            perror("Server error! Server responded with wrong response code!\n");
            continue;
        }
        read(sockfd, &response_id, sizeof(response_id));
        response_id = ntohl(response_id);
        if (response_id != request_id) {
            perror("Server error! Server responded with wrong response code!\n");
            continue;
        }
        if (mode == SQRT) {
            double result;
            read(sockfd, &result, sizeof(result));
            printf("Result of sqrt(%lf): %lf\n", number, result);
        }
        else {
            uint32_t result_len;
            read(sockfd, &result_len, sizeof(result_len));
            result_len = ntohl(result_len);
            char *result = (char *) calloc(result_len+1, sizeof(char));
            if (!result){
                perror("Memory error!\n");
                continue;
            }
            res = read(sockfd, result, result_len);
            if (res!=result_len){
                perror("Communication error! Recived less data then expected.\n");
                free(result);
                continue;
            }
            printf("Server time: %s\n", result);
            free(result);
        }

        count++;
    }
    shutdown(sockfd, SHUT_RDWR);
    return 0;
}
