#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "stuff.h"
#define WINDOW_SIZE 15
const uint32_t DATA_MAXSIZE = 1000;
uint32_t act_start;

bool is_valid_ip(char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result == 1;
}

bool is_number(char *str){
    for (int i = 0; str[i] != '\0'; i++) {
        if(isdigit(str[i]) == 0){
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

struct data window[WINDOW_SIZE];

int send_get(int sockfd, uint16_t port, char *server_ip){
    for(int k=0; k<WINDOW_SIZE && window[k].length != 0; k++){
        struct sockaddr_in server_address;
        bzero(&server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        inet_pton(AF_INET, server_ip, &server_address.sin_addr);

        char buffer[100];
        sprintf(buffer, "GET %d %d\n", window[k].start, window[k].length);
        ssize_t buffer_size = strlen(buffer);
        if(sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr*) &server_address, sizeof(server_address)) != buffer_size){
            fprintf(stderr, "sendto error: %s\n", strerror(errno)); 
		    return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

void prepare_window(int k, uint32_t end){
    uint32_t i = act_start;
    for(; i<end && k<WINDOW_SIZE; k++){
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

int32_t find_data(uint32_t start, uint32_t length){
    for(int k=0; k<WINDOW_SIZE; k++){
        if(window[k].start == start && window[k].length == length)
            return k;
    }
    return -1;
}

void move_window(){
    
}

int main(int argc, char* argv[]){

    if(!is_number(argv[2])){
        printf("TO!");
    }

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
	bzero (&server_address, sizeof(server_address));
	server_address.sin_family      = AF_INET;
	server_address.sin_port        = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		fprintf(stderr, "bind error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

    prepare_window(0, 0, dest_length);

    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;

    while(if_loop()) //dopóki nie mam wszystkich danych
    {
        // wyślij prośby o dane, których nie mamy
        send_get(sockfd, port, server_ip);

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

        char sender_ip_str[20];
        inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str, sizeof(sender_ip_str));

        if(strcmp(sender_ip_str, server_ip) != 0){
            continue;
        }

        buffer[datagram_len] = 0;
        int posn = uint8_find(buffer, '\n');
        if(posn == -1){
            continue;
        }

        char header[50];
        substr(header, buffer, 0, posn);
        printf(header);
        printf("\n");
        uint32_t start, length;
        sscanf(header, "%*s %d %d", &start, &length);

        int posd = find_data(start, length);

        if(posd < 0){
            continue;
        }

        window[posd].ready = true;
        window[posd].data  = buffer+posd; 

        // sprawdź czy możesz przesunąć okno i zapisać prefix okna do pliku
    }

    return EXIT_SUCCESS;
}