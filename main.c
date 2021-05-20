#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "stuff.h"

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

double timer;

struct data{
    uint8_t     *data;
    uint32_t    start;
    uint32_t    length;
    bool        ready;
};

const int WINDOW_SIZE = 15;
const int DATA_MAXSIZE = 1000;
struct data window[WINDOW_SIZE];

void send_get(int sockfd, uint16_t port, char *server_ip){
    for(int k=0; k<WINDOW_SIZE; k++){
        struct sockaddr_in server_address;
        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        inet_pton(AF_INET, server_ip, &server_address.sin_addr);

        uint8_t buffer[100];
        sprintf(buffer, "GET %d %d\n", window[k].start, window[k].length);
        ssize_t buffer_size = strlen(buffer);
        if(sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size){
            fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
		    return EXIT_FAILURE;
        }
    }
}

void prepare_window(uint32_t end){
    uint32_t i=0;
    for(int k=0; i<end && k<WINDOW_SIZE; k++){
        window[k].start  = i;
        window[k].length = (i + DATA_MAXSIZE <= end) ? DATA_MAXSIZE : (end - i);
        
        i += DATA_MAXSIZE;
    }
}

bool if_loop(){
    if(window[0].start == 0 && window[0].length == 0)
        return false;
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

    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;

    while(if_loop()) //dopóki nie mam wszystkich danych
    {
        // wyślij prośby o dane, których nie mamy

        // odbieranie danych
        fd_set 	descriptors;
        FD_ZERO (&descriptors);
        FD_SET 	(sockfd, &descriptors);

        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0) {
            fprintf(stderr, "select error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        tv.tv_sec = 4 - (get_time() - timer);
        tv.tv_sec = tv.tv_sec < 0 ? 0 : tv.tv_sec;

        if (ready == 0) {
            tv.tv_sec = 4;
            tv.tv_usec = 0;
            continue;
        }

        struct sockaddr_in 	sender;
		socklen_t 			sender_len = sizeof(sender);
		uint8_t 			buffer[IP_MAXPACKET];

        ssize_t datagram_len = recvfrom (
            sockfd,
            buffer,
            IP_MAXPACKET,
            MSG_DONTWAIT,
            (struct sockaddr*)&sender,
            &sender_len
        );

        if (datagram_len < 0) {
			fprintf(stderr, "recvfrom error: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

        // sprawdź czy możesz przesunąć okno i zapisać prefix okna do pliku
    }

    return EXIT_SUCCESS;
}