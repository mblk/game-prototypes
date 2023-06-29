#pragma once

#include <stdint.h>

typedef struct {
    int32_t x;
    int32_t y;
} coord_t;

int coord_equals(coord_t c1, coord_t c2);

