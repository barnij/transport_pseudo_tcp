// Bartosz Jaskiewicz, 307893
#include "defines.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int send_get(struct data *window,int sockfd, uint16_t port, char *server_ip) {
    for (int k=0; k<WINDOW_SIZE && window[k].length != 0; k++) {
        if(window[k].ready)
            continue;

        struct sockaddr_in server_address;
        bzero(&server_address, sizeof(server_address));
        server_address.sin_family   = AF_INET;
        server_address.sin_port     = htons(port);
        inet_pton(AF_INET, server_ip, &server_address.sin_addr);

        char buffer[100];
        sprintf(buffer, "GET %d %d\n", window[k].start, window[k].length);
        ssize_t buffer_size = strlen(buffer);
        if(sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size) {
            fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
		    return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}