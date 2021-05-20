#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

bool is_valid_ip(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result == 1;
}

bool is_number(char *str){
    for (int i = 0; str[i] != '\0'; i++) {
        if(isdigit(str[i]) != 0){
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]){

    if (argc != 5 || !is_valid_ip(argv[1]) || !is_number(argv[2]) || !is_number(argv[4])) {
        fprintf(stderr, "Input error.\n");
        return EXIT_FAILURE;
    }

    char *server_ip = argv[1];
    uint16_t port   = (uint16_t)atoi(argv[2]);
    int dest_length = atoi(argv[4]); 

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
    }

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    

    return EXIT_SUCCESS;
}