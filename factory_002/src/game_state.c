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

bool coord_is_in_building(const building_t *b, coord_t pos) {
    assert(b);
    return b->pos.x <= pos.x && pos.x < b->pos.x + b->size.w &&
           b->pos.y <= pos.y && pos.y < b->pos.y + b->size.h;
}

size_t get_building(game_state_t *gs, coord_t pos) {
    for(uint32_t i=1; i<gs->building_count; i++) {
        building_t *b = &gs->buildings[i];
        if (coord_is_in_building(b, pos))
            return i;
    }
    return 0;
}

bool space_is_free(game_state_t *gs, coord_t pos_min, coord_t pos_max) {
    for (int32_t y=pos_min.y; y<=pos_max.y; y++) {
        for (int32_t x=pos_min.x; x<=pos_max.x; x++) {
            if (get_building(gs, (coord_t) { x, y }))
                return false;
        }
    }
    return true;
}

size_t spawn_miner(game_state_t *gs, coord_t pos) {
    if (!space_is_free(gs, pos, (coord_t) { pos.x+1, pos.y })) {
        printf("no free space for miner at %d,%d\n", pos.x, pos.y);
        return 0;
    }

    size_t building_id = gs->building_count++;
    size_t miner_id = gs->miner_count++;

    building_t *building = gs->buildings + building_id;
    //miner_t *miner = gs->miners + miner_id;

    building->pos = pos;
    building->size = (building_size_t){ 2, 1 };
    building->type = BUILDING_TYPE_MINER;
    building->data_index = miner_id;

    return building_id;
}

size_t spawn_factory(game_state_t *gs, coord_t pos) {
    if (!space_is_free(gs, pos, (coord_t) { pos.x+1, pos.y+1 })) {
        printf("no free space for factory at %d,%d\n", pos.x, pos.y);
        return 0;
    }

    size_t building_id = gs->building_count++;
    size_t factory_id = gs->factory_count++;

    building_t *building = gs->buildings + building_id;
    //factory_t *factory = gs->factories + factory_id;

    building->pos = pos;
    building->size = (building_size_t){ 2, 2 };
    building->type = BUILDING_TYPE_FACTORY;
    building->data_index = factory_id;

    return building_id;
}

size_t spawn_belt(game_state_t *gs, coord_t pos) {
    if (!space_is_free(gs, pos, (coord_t) { pos.x, pos.y })) {
        printf("no free space for belt at %d,%d\n", pos.x, pos.y);
        return 0;
    }

    size_t building_id = gs->building_count++;
    size_t belt_id = gs->belt_count++;

    building_t *building = gs->buildings + building_id;
    //belt_t *belt = gs->belts + belt_id;

    building->pos = pos;
    building->size = (building_size_t){ 1, 1 };
    building->type = BUILDING_TYPE_BELT;
    building->data_index = belt_id;

    return building_id;
}

static uint8_t get_direction(coord_t from, coord_t to) {
    int dx = to.x - from.x;
    int dy = to.y - from.y;
    if (dx == 0 && dy != 0) {
        return dy > 0 ? DIR_DOWN : DIR_UP;
    }
    if (dy == 0 && dx != 0) {
        return dx > 0 ? DIR_RIGHT : DIR_LEFT;
    }
    return DIR_NONE;
}

static bool buildings_can_connect(game_state_t *gs, size_t source_id, size_t target_id, uint8_t *out_dir) {
    assert(gs);
    assert(source_id > 0);
    assert(target_id > 0);
    assert(source_id < gs->building_count);
    assert(target_id < gs->building_count);
    assert(source_id != target_id);

    building_t *source = gs->buildings + source_id;
    building_t *target = gs->buildings + target_id;

    // check left/right edge of source.
    for (int32_t y=source->pos.y; y<source->pos.y+source->size.h; y++) {
        int32_t x_left = source->pos.x - 1;
        int32_t x_right = source->pos.x + source->size.w;

        if (coord_is_in_building(target, (coord_t) { x_left, y })) {
            if (out_dir) *out_dir = DIR_LEFT;
            return true;
        }
        if (coord_is_in_building(target, (coord_t) { x_right, y })) {
            if (out_dir) *out_dir = DIR_RIGHT;
            return true;
        }
    }

    // check top/bottom edge of source.
    for (int32_t x=source->pos.x; x<source->pos.x+source->size.w; x++) {
        int32_t y_up = source->pos.y - 1;
        int32_t y_down = source->pos.y + source->size.h;

        if (coord_is_in_building(target, (coord_t) { x, y_down })) {
            if (out_dir) *out_dir = DIR_DOWN;
            return true;
        }
        if (coord_is_in_building(target, (coord_t) { x, y_up })) {
            if (out_dir) *out_dir = DIR_UP;
            return true;
        }
    }
    
    return false;
}

void connect_buildings(game_state_t *gs, size_t source_id, size_t target_id) {
    assert(gs);
    assert(source_id > 0);
    assert(target_id > 0);
    assert(source_id < gs->building_count);
    assert(target_id < gs->building_count);
    assert(source_id != target_id);

    uint8_t conn_dir = 0;
    if (!buildings_can_connect(gs, source_id, target_id, &conn_dir)) {
        printf("can't connect buildings %lu %lu\n", source_id, target_id);
        return;
    }

    building_t *source = gs->buildings + source_id;
    building_t *target = gs->buildings + target_id;

    const item_output_t output = {
        .type = target->type,
        .index = target->data_index,
    };

    switch(source->type) {
        case BUILDING_TYPE_MINER:   (gs->miners +    source->data_index)->output = output; break;
        case BUILDING_TYPE_FACTORY: (gs->factories + source->data_index)->output = output; break;
        case BUILDING_TYPE_BELT:    (gs->belts +     source->data_index)->output = output; break;
    }

    if (source->type == BUILDING_TYPE_BELT) {
        belt_t *source_belt = gs->belts + source->data_index;
        source_belt->out_dir = conn_dir;
    }
    if (target->type == BUILDING_TYPE_BELT) {
        belt_t *target_belt = gs->belts + target->data_index;
        target_belt->in_dir = conn_dir;
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

const bool try_put_item(game_state_t *gs, item_output_t output, uint8_t item) {
    if (output.index == 0 || item == 0)
        return false;

    switch (output.type) {
        case BUILDING_TYPE_BELT:
            {
                belt_t *belt = gs->belts + output.index;
                if (belt->items[0] == 0) {
                    belt->items[0] = item;
                    belt->works[0] = 0;
                    return true;
                }
            }
            break;

        case BUILDING_TYPE_FACTORY:
            {
                factory_t *factory = gs->factories + output.index;
                for (size_t i=0; i<ARRAY_LENGTH(factory->items); i++) {
                    if (factory->items[i] == 0) {
                        factory->items[i] = item;
                        return true;
                    }
                }
            }
            break;
    }

    return false;
}

static void update_miner(game_state_t *gs, miner_t *miner) {
    assert(gs);
    assert(miner);

    switch (miner->state) {
        case MINER_STATE_MINING:
            miner->work++;
            if (miner->work >= MINER_WORK_PER_ITEM) {
                miner->work = 0;
                miner->state = MINER_STATE_UNLOAD;
            }
            break;

        case MINER_STATE_UNLOAD:
            if (try_put_item(gs, miner->output, 1 + miner->next_item)) {
                miner->state = MINER_STATE_MINING;
                miner->next_item++;
                if (miner->next_item >= 4) miner->next_item = 0;
            }
            break;
    }
}

void update_factory(game_state_t *gs, factory_t *factory) {
    assert(gs);
    assert(factory);

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
                }
            }
            break;

        case FACTORY_STATE_PRODUCE:
            factory->work++;
            if (factory->work >= FACTORY_WORK_PER_ITEM) {
                factory->state = FACTORY_STATE_UNLOAD;
                memset(factory->items, 0, sizeof(uint8_t) * 4);
            }
            break;

        case FACTORY_STATE_UNLOAD:
            if (try_put_item(gs, factory->output, 9)) {
                factory->state = FACTORY_STATE_WAIT_ITEMS;
            }
            break;
    }
}

void update_belt(game_state_t *gs, belt_t *belt) {
    assert(gs);
    assert(belt);

    // TODO: Behaviour depends on order & items of adjacent buildings are not tightly packed.
    // Introduce early_update & late_update ?

    for (size_t slot=0; slot<BELT_ITEM_COUNT; slot++) {
        if (belt->items[slot] != 0) {
            if (belt->works[slot] < BELT_WORK_PER_ITEM) {
                belt->works[slot]++;
            }
        }
    }

    const size_t last_item_index = BELT_ITEM_COUNT - 1;

    // Try to unload last item.
    if (belt->items[last_item_index] != 0 &&
        belt->works[last_item_index] == BELT_WORK_PER_ITEM) {

        if (try_put_item(gs, belt->output, belt->items[last_item_index])) {
            belt->items[last_item_index] = 0;
            belt->works[last_item_index] = 0;
        }
    }

    for (size_t slot=last_item_index; slot>0; slot--) {
        if (belt->items[slot] == 0 && 
            belt->items[slot-1] != 0 &&
            belt->works[slot-1] == BELT_WORK_PER_ITEM) {

            belt->items[slot] = belt->items[slot-1];
            belt->works[slot] = 0;
            belt->items[slot-1] = 0;
            belt->works[slot-1] = 0;
        }
    }
}


void update_game_state(game_state_t *gs) {

    for (size_t i=1; i<gs->miner_count; i++) update_miner(gs, gs->miners + i);
    for (size_t i=1; i<gs->factory_count; i++) update_factory(gs, gs->factories + i);
    for (size_t i=1; i<gs->belt_count; i++) update_belt(gs, gs->belts + i);
    
}

