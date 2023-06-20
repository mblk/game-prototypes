#pragma once

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#define MAX_ENTITY_COUNT (1000)

#define BUILDING_WIDTH  (100.0f)
#define BUILDING_HEIGHT (100.0f)

#define BUILDING_TYPE_MINER   (0)
#define BUILDING_TYPE_FACTORY (1)
#define BUILDING_TYPE_BELT    (2)

#define FACTORY_STATE_WAIT_ITEMS (0)
#define FACTORY_STATE_PRODUCE    (1)
#define FACTORY_STATE_UNLOAD     (2)

typedef struct {
    int32_t x;
    int32_t y;
} coord_t;

typedef struct {
    coord_t pos;
    uint8_t dir;
    uint32_t type;
    uint32_t data_index;
} building_t;

typedef struct {
    uint32_t work;
    uint32_t next_item_type;
    uint32_t output_belt;
} miner_t;

typedef struct {
    uint32_t work;
    uint32_t recipe;
    uint8_t state;
    uint8_t items[4];
    uint32_t output_belt;
} factory_t;

typedef struct {
    uint32_t work;
    uint8_t items[8];
    uint32_t output_belt;
    uint32_t output_factory;
} belt_t;

typedef struct {
    building_t buildings[MAX_ENTITY_COUNT];
    miner_t miners[MAX_ENTITY_COUNT];
    factory_t factories[MAX_ENTITY_COUNT];
    belt_t belts[MAX_ENTITY_COUNT];

    size_t building_count;
    size_t miner_count;
    size_t factory_count;
    size_t belt_count;
} game_state_t;

int coord_equals(coord_t c1, coord_t c2);
coord_t world_position_to_coord(Vector2 world_position);
Vector2 coord_to_world_position(coord_t coord); 

building_t *get_building(game_state_t *gs, coord_t pos);
building_t *spawn_miner(game_state_t *gs, coord_t pos);
building_t *spawn_factory(game_state_t *gs, coord_t pos);
building_t *spawn_belt(game_state_t *gs, coord_t pos);
void connect_buildings(game_state_t *gs, building_t *a, building_t *b);

void reset_game_state(game_state_t *gs);
void update_game_state(game_state_t *gs);

