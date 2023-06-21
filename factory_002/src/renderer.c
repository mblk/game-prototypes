#include "renderer.h"

#include <stdio.h>
#include <assert.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

coord_t world_position_to_coord(Vector2 world_position) {
    int32_t x = world_position.x / WORLD_CELL_SIZE;
    int32_t y = world_position.y / WORLD_CELL_SIZE;
    return (coord_t) { x, y };
}

Vector2 coord_to_world_position(coord_t coord) {
    float x = coord.x * WORLD_CELL_SIZE;
    float y = coord.y * WORLD_CELL_SIZE;
    return (Vector2) { x, y };
}

void render_world(const game_state_t *gs)
{
    assert(gs);

    const Vector2 cell_size = (Vector2) { WORLD_CELL_SIZE, WORLD_CELL_SIZE };
    const Vector2 half_cell_size = Vector2Scale(cell_size, 0.5f);

    for(uint32_t i=1; i<gs->building_count; i++) {

        const building_t *b = gs->buildings + i;
        Vector2 wp = coord_to_world_position(b->pos);
        float x = wp.x;
        float y = wp.y;
        float w = b->size.w * WORLD_CELL_SIZE;
        float h = b->size.h * WORLD_CELL_SIZE;

        const Vector2 size = (Vector2) { w, h };
        const Vector2 half_size = Vector2Scale(size, 0.5f);

        const Vector2 center = Vector2Add(wp, half_size);

        DrawRectangle(x, y, w, h, GRAY);
        DrawRectangleLines(x, y, w, h, BLACK);
        DrawText(TextFormat("%u/%u", b->type, b->data_index), x+10, y+10, 10, BLACK);

        switch (b->type) {
            case BUILDING_TYPE_MINER:
                {
                    const miner_t *miner = gs->miners + b->data_index;
                    const float progress = (float)miner->work / (float)MINER_WORK_PER_ITEM;
                    const float radius = WORLD_CELL_SIZE / 2.0f;

                    DrawText(TextFormat("Miner %u", miner->work), x+10, y+20, 10, BLACK);

                    switch (miner->state) {
                        case MINER_STATE_MINING:
                            {
                                float end_angle = progress * 360.0f;
                                int segments = progress * 24.0f;
                                DrawCircleSector(center, radius, 0.0f, end_angle, segments, WHITE);
                            }
                            break;
                        case MINER_STATE_UNLOAD:
                            DrawCircleV(center, radius, LIGHTGRAY);
                            break;
                    }
                }
                break;

            case BUILDING_TYPE_FACTORY:
                {
                    const factory_t *factory = gs->factories + b->data_index;
                    const float progress = (float)factory->work / (float)FACTORY_WORK_PER_ITEM;
                    const float radius = WORLD_CELL_SIZE / 2.0f;

                    DrawText(TextFormat("Factory %u %u", factory->state, factory->work), x+10, y+20, 10, BLACK);

                    for(int item=0; item<4; item++) {
                        DrawText(TextFormat("%u", factory->items[item]), x+10, y+30 + item*10, 10, BLACK);
                    }

                    switch (factory->state) {
                        case FACTORY_STATE_WAIT_ITEMS:
                            DrawCircleLines(center.x, center.y, radius, WHITE);
                            break;
                        case FACTORY_STATE_PRODUCE:
                            {
                                float end_angle = progress * 360.0f;
                                int segments = progress * 24.0f;
                                DrawCircleSector(center, radius, 0.0f, end_angle, segments, WHITE);
                            }
                            break;
                        case FACTORY_STATE_UNLOAD:
                            DrawCircleV(center, radius, LIGHTGRAY);
                            break;
                    }


                }
                break;

            case BUILDING_TYPE_BELT:
                {
                    const belt_t *belt = gs->belts + b->data_index;
                    DrawText(TextFormat("Belt"), x+10, y+20, 10, BLACK);

                    for(int item=0; item<BELT_ITEM_COUNT; item++) {
                        DrawText(TextFormat("%u %u", belt->items[item], belt->works[item]), x+10, y+30 + item*10, 10, BLACK);
                    }

                    for (size_t item=0; item<BELT_ITEM_COUNT; item++) {

                        // Hm: Need orientation ?
                        // or src/tgt ?
                        // or just tgt ?

                        float item_w = (float)WORLD_CELL_SIZE / (float)BELT_ITEM_COUNT;
                        float item_h = (float)WORLD_CELL_SIZE / (float)BELT_ITEM_COUNT;

                        float off_factor = (float)belt->works[item] / (float)BELT_WORK_PER_ITEM;
                        float off_value = item_w * off_factor;

                        float item_x = wp.x + (float)item*item_w + off_value - item_w/2.0f;
                        float item_y = wp.y + WORLD_CELL_SIZE/2.0f           - item_h/2.0f;

                        if (belt->items[item]) {
                            Color item_color;
                            switch(belt->items[item]) {
                                case 1: item_color = RED; break;
                                case 2: item_color = BLUE; break;
                                case 3: item_color = GREEN; break;
                                default: item_color = PURPLE; break;
                            }
                            DrawRectangle(item_x+1, item_y+1, item_w-2, item_h-2, item_color);
                        }
                    }

                }
                break;
        }
    }



}

