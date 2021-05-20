#include <time.h>
#include <stdint.h>
#include <string.h>

double get_time()
{
	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);
	return now.tv_sec + now.tv_nsec*1e-9;
}

int32_t uint8_find(uint8_t *s, char what){
    for(int i=0; s[i]; i++){
        if(s[i] == what)
            return i;
    }
    return -1;
}

void substr(char *sub, uint8_t *buff, int a, int n){
    memcpy(sub, &buff[a], n);
    sub[n]='\0';
}