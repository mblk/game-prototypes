#pragma once

//#define _POSIX_C_SOURCE 199309L
#include <time.h>

#define CHECK_TIME(x) {\
    struct timespec start,end;\
    clock_gettime(CLOCK_REALTIME, &start);\
    x;\
    clock_gettime(CLOCK_REALTIME, &end);\
    double f = ((double)end.tv_sec*1e9 + end.tv_nsec) - ((double)start.tv_sec*1e9 + start.tv_nsec); \
    printf("time '" #x "': %f ms\n", f/1000000); \
}


