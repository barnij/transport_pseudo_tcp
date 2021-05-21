// Bartosz Jaskiewicz, 307893
#include <stdint.h>
#include <stdbool.h>
#define DATA_MAX 1000
#define WINDOW_SIZE 1000

struct data{
    char        data[DATA_MAX];
    uint32_t    start;
    uint32_t    length;
    bool        ready;
};