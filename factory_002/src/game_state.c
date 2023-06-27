#include <stdio.h>
#include <assert.h>
#include <string.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "utils.h"
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
   
    //xxx
    return true;
    //xxx

    for (int32_t y=pos_min.y; y<=pos_max.y; y++) {
        for (int32_t x=pos_min.x; x<=pos_max.x; x++) {
            if (get_building(gs, (coord_t) { x, y }))
                return false;
        }
    }
    return true;
}

size_t spawn_miner(game_state_t *gs, coord_t pos) {

    assert(gs->building_count < MAX_ENTITY_COUNT); 
    assert(gs->miner_count < MAX_ENTITY_COUNT);

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

    assert(gs->building_count < MAX_ENTITY_COUNT);
    assert(gs->factory_count < MAX_ENTITY_COUNT);

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

    assert(gs->building_count < MAX_ENTITY_COUNT);  
    assert(gs->belt_count < MAX_ENTITY_COUNT);

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

void delete_building(game_state_t *gs, size_t building_id) {
    assert(gs);
    assert(building_id > 0);
    assert(building_id < gs->building_count);

    building_t *building = gs->buildings + building_id;

    // Mark for deletion
    building->flags |= ENTITY_FLAGS_DELETE;

    switch (building->type) {
    case BUILDING_TYPE_MINER: (gs->miners + building->data_index)->flags |= ENTITY_FLAGS_DELETE; break;
    case BUILDING_TYPE_BELT: (gs->belts + building->data_index)->flags |= ENTITY_FLAGS_DELETE; break;
    case BUILDING_TYPE_FACTORY: (gs->factories + building->data_index)->flags |= ENTITY_FLAGS_DELETE; break;
    }

    printf("marked for deletion: %lu\n", building_id);
}

void reset_game_state(game_state_t *gs) {
    assert(gs);
    //memset(gs, 0, sizeof(game_state_t)); // ~3ms -> ~30GB/s

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

static void update_output_reference(item_output_t *output, size_t *miner_mappings,
        size_t *belt_mappings, size_t *factory_mappings) {
    assert(output);
    assert(miner_mappings);
    assert(belt_mappings);
    assert(factory_mappings);

    if (!output->type && !output->index) return;

    size_t *mapping = NULL;
    switch (output->type) {
        case BUILDING_TYPE_MINER: mapping = miner_mappings; break;
        case BUILDING_TYPE_BELT: mapping = belt_mappings; break;
        case BUILDING_TYPE_FACTORY: mapping = factory_mappings; break;
    }
    if (!mapping) return;

    output->index = mapping[output->index];
}

static size_t miner_mapping[MAX_ENTITY_COUNT];
static size_t belt_mapping[MAX_ENTITY_COUNT];
static size_t factory_mapping[MAX_ENTITY_COUNT];

void update_game_state_1(const game_state_t *old, game_state_t *new) {

    //
    // Step 1: copy belts/miners/factories to new arrays
    //

    // TODO: bulk copy if nothing was deleted?
    // TODO: track bounds of modified region?

    for (size_t old_id=1; old_id<old->building_count; old_id++) {
        const building_t *old_building = old->buildings + old_id;
        if (old_building->flags & ENTITY_FLAGS_DELETE) {
            continue;
        }
        const size_t new_id = new->building_count++;
        building_t *new_building = new->buildings + new_id;
        memcpy(new_building, old_building, sizeof(building_t));
    }
    for (size_t old_id=1; old_id<old->miner_count; old_id++) {
        const miner_t *old_miner = old->miners + old_id;
        if(old_miner->flags & ENTITY_FLAGS_DELETE) {
            miner_mapping[old_id] = 0;
            continue;
        }
        const size_t new_id = new->miner_count++;
        miner_mapping[old_id] = new_id;
        miner_t *new_miner = new->miners + new_id;
        memcpy(new_miner, old_miner, sizeof(miner_t));
    }
    for (size_t old_id=1; old_id<old->belt_count; old_id++) {
        const belt_t *old_belt = old->belts + old_id;
        if(old_belt->flags & ENTITY_FLAGS_DELETE) {
            belt_mapping[old_id] = 0;
            continue;
        }
        const size_t new_id = new->belt_count++;
        belt_mapping[old_id] = new_id;
        belt_t *new_belt = new->belts + new_id;
        memcpy(new_belt, old_belt, sizeof(belt_t));
    }
    for (size_t old_id=1; old_id<old->factory_count; old_id++) {
        const factory_t *old_factory = old->factories + old_id;
        if(old_factory->flags & ENTITY_FLAGS_DELETE) {
            factory_mapping[old_id] = 0;
            continue;
        }
        const size_t new_id = new->factory_count++;
        factory_mapping[old_id] = new_id;
        factory_t *new_factory = new->factories + new_id;
        memcpy(new_factory, old_factory, sizeof(factory_t));
    }
}

void update_game_state_2(const game_state_t *old, game_state_t *new) {

    //
    // Step 2: fix references
    //

    for (size_t i=1; i<new->building_count; i++) {
        building_t *building = new->buildings + i;
        switch (building->type) {
            case BUILDING_TYPE_MINER: building->data_index = miner_mapping[building->data_index]; break;
            case BUILDING_TYPE_BELT: building->data_index = belt_mapping[building->data_index]; break;
            case BUILDING_TYPE_FACTORY: building->data_index = factory_mapping[building->data_index]; break;
        }
        assert(building->data_index);
    }
    for (size_t i=1; i<new->miner_count; i++) {
        miner_t *miner = new->miners + i;
        update_output_reference(&miner->output, miner_mapping, belt_mapping, factory_mapping);
    }
    for (size_t i=1; i<new->belt_count; i++) {
        belt_t *belt = new->belts + i;
        update_output_reference(&belt->output, miner_mapping, belt_mapping, factory_mapping);
    }
    //for (size_t i=1; i<new->factory_count; i++) {
    // no outputs
    //}

}

void update_game_state_3(const game_state_t *old, game_state_t *new) {

    for (size_t i=1; i<new->miner_count; i++) update_miner(new, new->miners + i);
    for (size_t i=1; i<new->factory_count; i++) update_factory(new, new->factories + i);
    for (size_t i=1; i<new->belt_count; i++) update_belt(new, new->belts + i);
}

void update_game_state(const game_state_t *old, game_state_t *new) {

    CHECK_TIME(reset_game_state(new));
    CHECK_TIME(update_game_state_1(old, new));
    CHECK_TIME(update_game_state_2(old, new));
    CHECK_TIME(update_game_state_3(old, new));
}

