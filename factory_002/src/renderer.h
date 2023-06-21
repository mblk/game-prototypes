#pragma once

#include "game_state.h"

#define WORLD_CELL_SIZE (100.0f)

typedef struct {
    uint32_t ticks;
} render_state_t;

coord_t world_position_to_coord(Vector2 world_position);
Vector2 coord_to_world_position(coord_t coord); 

void render_world(const game_state_t *gs, render_state_t *rs);

