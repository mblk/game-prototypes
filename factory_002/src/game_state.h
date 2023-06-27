#pragma once

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#define ARRAY_LENGTH(array) (sizeof((array))/sizeof((array)[0]))

#define DIR_NONE     (0)
#define DIR_UP       (1)
#define DIR_DOWN     (2)
#define DIR_LEFT     (4)
#define DIR_RIGHT    (5)

#define MAX_ENTITY_COUNT          (1000 * 1000)

#define BUILDING_TYPE_MINER       (0)
#define BUILDING_TYPE_FACTORY     (1)
#define BUILDING_TYPE_BELT        (2)

#define ENTITY_FLAGS_DELETE     (1)

#define MINER_STATE_MINING        (0)
#define MINER_STATE_UNLOAD        (1)

#define FACTORY_STATE_WAIT_ITEMS  (0)
#define FACTORY_STATE_PRODUCE     (1)
#define FACTORY_STATE_UNLOAD      (2)

#define MINER_WORK_PER_ITEM       (40)

#define FACTORY_WORK_PER_ITEM     (120)

#define BELT_ITEM_COUNT           (4)
#define BELT_WORK_PER_ITEM        (10)

typedef struct {
    int32_t x;
    int32_t y;
} coord_t;

typedef struct {
    uint8_t w;
    uint8_t h;
} building_size_t;

typedef struct {
    uint32_t flags;
    // ---
    coord_t pos;
    building_size_t size;
    // ---
    uint32_t type; // miner/belt/factory
    uint32_t data_index; // index into miner/belt/factory-arrays
} building_t;

typedef struct {
    uint8_t type; // miner/belt/factory
    uint32_t index; // index into miner/belt/factory-arrays
} item_output_t;

typedef struct {
    uint32_t flags;
    // ---
    uint32_t work;
    uint8_t state;
    uint8_t next_item;
    item_output_t output;
} miner_t;

typedef struct {
    uint32_t flags;
    // ---
    uint32_t work;
    uint32_t recipe;
    uint8_t state;
    uint8_t items[4];
    item_output_t output;
} factory_t;

typedef struct {
    uint32_t flags;
    // ---
    uint8_t items[BELT_ITEM_COUNT];
    uint8_t works[BELT_ITEM_COUNT];
    item_output_t output;
    uint8_t in_dir;
    uint8_t out_dir;
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

size_t get_building(game_state_t *gs, coord_t pos);
bool space_is_free(game_state_t *gs, coord_t pos_min, coord_t pos_max);

size_t spawn_miner(game_state_t *gs, coord_t pos);
size_t spawn_factory(game_state_t *gs, coord_t pos);
size_t spawn_belt(game_state_t *gs, coord_t pos);

void connect_buildings(game_state_t *gs, size_t source_id, size_t target_id); 
void delete_building(game_state_t *gs, size_t building_id);

void reset_game_state(game_state_t *gs);
void update_game_state(const game_state_t *old, game_state_t *new);

