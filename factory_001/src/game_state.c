#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "game_state.h"

int coord_equals(coord_t c1, coord_t c2) {
    return c1.x == c2.x && c1.y == c2.y;
}

coord_t world_position_to_coord(Vector2 world_position) {
    int32_t x = world_position.x / BUILDING_WIDTH;
    int32_t y = world_position.y / BUILDING_HEIGHT;
    return (coord_t) { x, y };
}

Vector2 coord_to_world_position(coord_t coord) {
    float x = coord.x * BUILDING_WIDTH;
    float y = coord.y * BUILDING_HEIGHT;
    return (Vector2) { x, y };
}

building_t* get_building(game_state_t *gs, coord_t pos) {
    for(uint32_t i=1; i<gs->building_count; i++) {
        building_t *b = &gs->buildings[i];
        if (coord_equals(b->pos, pos))
            return b;
    }
    return NULL;
}

building_t *spawn_miner(game_state_t *gs, coord_t pos) {

    size_t building_id = gs->building_count++;
    size_t miner_id = gs->miner_count++;

    building_t *building = gs->buildings + building_id;
    miner_t *miner = gs->miners + miner_id;

    building->pos = pos;
    building->dir = 0;
    building->type = BUILDING_TYPE_MINER;
    building->data_index = miner_id;

    miner->work = 0;

    return building;
}

building_t *spawn_factory(game_state_t *gs, coord_t pos) {
    size_t building_id = gs->building_count++;
    size_t factory_id = gs->factory_count++;

    building_t *building = gs->buildings + building_id;
    factory_t *factory = gs->factories + factory_id;

    building->pos = pos;
    building->dir = 0;
    building->type = BUILDING_TYPE_FACTORY;
    building->data_index = factory_id;

    factory->work = 0;

    return building;
}

building_t *spawn_belt(game_state_t *gs, coord_t pos) {
    size_t building_id = gs->building_count++;
    size_t belt_id = gs->belt_count++;

    building_t *building = gs->buildings + building_id;
    belt_t *belt = gs->belts + belt_id;

    building->pos = pos;
    building->dir = 0;
    building->type = BUILDING_TYPE_BELT;
    building->data_index = belt_id;

    //belt->items = ...;
    
    return building;
}

void connect_buildings(game_state_t *gs, building_t *a, building_t *b) {
    assert(gs);
    assert(a);
    assert(b);

    if (a->type == BUILDING_TYPE_MINER && b->type == BUILDING_TYPE_BELT) {
        miner_t *miner = gs->miners + a->data_index;
        miner->output_belt = b->data_index;
    }

    if (a->type == BUILDING_TYPE_BELT && b->type == BUILDING_TYPE_BELT) {
        belt_t *belt = gs->belts + a->data_index;
        belt->output_belt = b->data_index;
        belt->output_factory = 0;
    }

    if (a->type == BUILDING_TYPE_BELT && b->type == BUILDING_TYPE_FACTORY) {
        belt_t *belt = gs->belts + a->data_index;
        belt->output_factory = b->data_index;
        belt->output_belt = 0;
    }

    if (a->type == BUILDING_TYPE_FACTORY && b->type == BUILDING_TYPE_BELT) {
        factory_t *factory = gs->factories + a->data_index;
        factory->output_belt = b->data_index;
    }
}

void reset_game_state(game_state_t *gs) {
    assert(gs);
    memset(gs, 0, sizeof(game_state_t));

    // Never use the 0-index. This way, output_belt=0 can be used to express a missing connection.
    gs->building_count = 1;
    gs->miner_count = 1;
    gs->factory_count = 1;
    gs->belt_count = 1;
}

void update_game_state(game_state_t *gs) {

    for(size_t i=1; i<gs->miner_count; i++) {
        miner_t *miner = gs->miners + i;
        miner->work++;
        if (miner->work >= 100) {
            miner->work = 0;
            if (miner->output_belt != 0) {
                belt_t *output_belt = gs->belts + miner->output_belt;
                if (output_belt->items[0] == 0) {
                    printf("miner producing item...\n");
                    output_belt->items[0] = 1 + miner->next_item_type;
                }
            }
            miner->next_item_type++;
            if (miner->next_item_type == 5) {
                miner->next_item_type = 0;
            }
        }
    }

    for(size_t i=1; i<gs->factory_count; i++) {
        factory_t *factory = gs->factories + i;

        switch (factory->state) {
            case FACTORY_STATE_WAIT_ITEMS:
            {
                bool has_all_items = true;
                for(int factory_item=0; factory_item<4; factory_item++) {
                    if (factory->items[factory_item] == 0) {
                        has_all_items = false;
                        break;
                    }
                }
                if (has_all_items) {
                    factory->state = FACTORY_STATE_PRODUCE;
                    factory->work = 0;
                    memset(factory->items, 0, sizeof(uint8_t) * 4);
                }
                break;
            }

            case FACTORY_STATE_PRODUCE:
            {
                factory->work++;
                if (factory->work >= 120) {
                    factory->state = FACTORY_STATE_UNLOAD;
                }
                break;
            }

            case FACTORY_STATE_UNLOAD:
            {
                if (factory->output_belt) {
                    belt_t *output_belt = gs->belts + factory->output_belt;
                    if (output_belt->items[0] == 0) {
                        output_belt->items[0] = 9;
                        factory->state = FACTORY_STATE_WAIT_ITEMS;
                    }
                }
                break;
            }
        }
    }

    for(size_t i=1; i<gs->belt_count; i++) {
        belt_t *belt = gs->belts + i;

        belt->work++;
        if (belt->work >= 30) {
            belt->work = 0;

            if (belt->items[7] != 0) {
                // push out to factory?
                if (belt->output_factory != 0) {
                    factory_t *output_factory = gs->factories + belt->output_factory;
                    for(int factory_item=0; factory_item<4; factory_item++) {
                        if (output_factory->items[factory_item] == 0) {
                            output_factory->items[factory_item] = belt->items[7];
                            belt->items[7] = 0;
                            break;
                        }
                    }
                }
                // push out to belt?
                else if (belt->output_belt != 0) {
                    belt_t *output_belt = gs->belts + belt->output_belt;
                    if (output_belt->items[0] == 0) {
                        output_belt->items[0] = belt->items[7];
                        belt->items[7] = 0;
                    }
                }
            }
            for(int slot=7; slot>0; slot--) {
                if (belt->items[slot] == 0) {
                    belt->items[slot] = belt->items[slot-1];
                    belt->items[slot-1] = 0;
                }
            }
        }
    }

}

