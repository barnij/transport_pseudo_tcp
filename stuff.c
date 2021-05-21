// Bartosz Jaskiewicz, 307893
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

double get_time() {
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return (now.tv_sec + now.tv_nsec*1e-9) * 1000000;
}

int32_t uint8_find(uint8_t *s, char what) {
    for(int i=0; s[i]; i++){
        if(s[i] == what)
            return i;
    }
    return -1;
}

void substr(char *sub, uint8_t *buff, int a, int n) {
    memcpy(sub, &buff[a], n);
    sub[n]='\0';
}

bool is_number(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if(isdigit(str[i]) == 0){
            return false;
        }
    }
    return true;
}

bool is_valid_ip(char *ipAddress) {
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result == 1;
}