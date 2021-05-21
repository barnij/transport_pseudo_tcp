#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "stuff.h"
#define DATA_MAX 1000
#define WINDOW_SIZE 1000
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
    char        data[DATA_MAX];
    uint32_t    start;
    uint32_t    length;
    bool        ready;
};

const struct data empty_window;
struct data window[WINDOW_SIZE];
int sit, to_save;
bool small;

int send_get(int sockfd, uint16_t port, char *server_ip){
    for(int k=0; k<WINDOW_SIZE && window[k].length != 0; k++){
        if(window[k].ready)
            continue;

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


void prepare_window_start(uint32_t end){
    int k=0;
    for(uint32_t i = 0; i<end && k<WINDOW_SIZE; k++){
        window[k].start  = i;
        act_start        = i + DATA_MAX;
        window[k].length = (i + DATA_MAX <= end) ? DATA_MAX : (end - i);
        
        i += DATA_MAX;
    }
}

int32_t find_data(uint32_t start, uint32_t length){
    for(int k=0; k<WINDOW_SIZE; k++){
        if(window[k].start == start && window[k].length == length)
            return k;
    }
    return -1;
}


void move_window(uint32_t end, FILE * file){
    if(!window[sit].ready) return;
    if(small){
        for(;window[sit].ready && sit<WINDOW_SIZE; sit++){
            if(window[sit].length == 0){
                return;
            }
            
            printf("saved from %d, %d bytes\n", window[sit].start, window[sit].length);
            if ((int)(fwrite(window[sit].data, sizeof(char), window[sit].length, file)) != (int)window[sit].length){
                fprintf(stderr, "fwrite error.\n");
                exit(EXIT_FAILURE);
            }
            to_save -= window[sit].length;
        }
        return;
    }


    int k = 0, i = sit;
    for(; k<WINDOW_SIZE && window[i].ready; k++){
        to_save -= window[i].length;
        printf("saved from %d, %d bytes\n", window[i].start, window[i].length);
        if ((int)(fwrite(window[i].data, sizeof(char), window[i].length, file)) != (int)window[i].length){
            fprintf(stderr, "fwrite error.\n");
            exit(EXIT_FAILURE);
        }

        window[i].start  = act_start;
        act_start       += DATA_MAX;
        window[i].ready  = false;
        window[i].length = (act_start <= end) ? DATA_MAX : (end - window[i].start);

        i = (i + 1) % WINDOW_SIZE;
    }

    sit = i;

}

int main(int argc, char* argv[]){

    if (argc != 5 || !is_valid_ip(argv[1]) || !is_number(argv[2]) || !is_number(argv[4])) {
        fprintf(stderr, "Input error.\n");
        return EXIT_FAILURE;
    }

    char *server_ip = argv[1];
    uint16_t port   = (uint16_t)atoi(argv[2]);
    int dest_length = atoi(argv[4]);
    to_save         = dest_length;
    if((long long)dest_length <= (long long)DATA_MAX * (long long)WINDOW_SIZE){
        small = true;
    }
    

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
    }

    FILE * file = fopen(argv[3], "wb");
    if (file == NULL)
    {
        fprintf(stderr, "fopen error: %s\n", strerror(errno)); 
		return EXIT_FAILURE;
    }

    // prepare_window(0, dest_length);
    prepare_window_start(dest_length);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 900;

    while(to_save > 0) //dopóki nie mam wszystkich danych
    {
        // wyślij prośby o dane, których nie mamy
        send_get(sockfd, port, server_ip);

        // odbieranie danych
        fd_set 	descriptors;
        FD_ZERO (&descriptors);
        FD_SET 	(sockfd, &descriptors);

        // printf("TU");
        int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);

        if (ready < 0) {
            fprintf(stderr, "select error: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        tv.tv_usec = 100 - (get_time() - timer);
        tv.tv_usec = tv.tv_usec < 0 ? 0 : tv.tv_usec;

        if (ready == 0) {
            tv.tv_sec = 0;
            tv.tv_usec = 100;
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

        char header[50], type[10];
        substr(header, buffer, 0, posn);

        uint32_t start, length;
        sscanf(header, "%s %d %d", type, &start, &length);
        if(strcmp(type,"DATA") != 0){
            continue;
        }
        // printf(header);
        // printf("\n");

        int posd = find_data(start, length);

        if(posd < 0){
            continue;
        }

        if(window[posd].ready){
            continue;
        }

        // printf(header);
        // printf("\n");

        window[posd].ready = true;
        memcpy(window[posd].data, (char *)(buffer+posn+1), length);

        // sprawdź czy możesz przesunąć okno i zapisać prefix okna do pliku
        move_window(dest_length, file);
    }

    fclose(file);

    return EXIT_SUCCESS;
}