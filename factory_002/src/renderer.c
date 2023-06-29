#include "renderer.h"

#include <stdio.h>
#include <assert.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

coord_t world_position_to_coord(Vector2 world_position) {
    int32_t x = world_position.x / WORLD_CELL_SIZE;
    int32_t y = world_position.y / WORLD_CELL_SIZE;
    if (world_position.x < 0) x--;
    if (world_position.y < 0) y--;
    return (coord_t) { x, y };
}

Vector2 coord_to_world_position(coord_t coord) {
    float x = coord.x * WORLD_CELL_SIZE;
    float y = coord.y * WORLD_CELL_SIZE;
    return (Vector2) { x, y };
}

Color get_item_color(uint8_t item) {
    switch(item) {
        case 1: return RED;
        case 2: return BLUE;
        case 3: return GREEN;
        case 4: return ORANGE;
        default: return PURPLE;
    }
}

void render_miner(render_state_t *rs, const miner_t *miner, Rectangle r) {

    const Vector2 center = (Vector2) {
        .x = r.x + r.width * 0.5f,
        .y = r.y + r.height * 0.5f,
    };

    const float progress = (float)miner->work / (float)MINER_WORK_PER_ITEM;
    const float radius = WORLD_CELL_SIZE / 3.0f;

    DrawRectangleRec(r, GRAY);
    DrawRectangleLinesEx(r, 2.0f, BLACK);
    DrawText("Miner", r.x+5, r.y+5, 20, BLACK);

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

void render_factory(render_state_t *rs, const factory_t *factory, Rectangle r) {

    const Vector2 center = (Vector2) {
        .x = r.x + r.width * 0.5f,
        .y = r.y + r.height * 0.5f,
    };

    const float progress = (float)factory->work / (float)FACTORY_WORK_PER_ITEM;
    const float radius = WORLD_CELL_SIZE / 3.0f;

    DrawRectangleRec(r, GRAY);
    DrawRectangleLinesEx(r, 2.0f, BLACK);
    DrawText("Factory", r.x+5, r.y+5, 20, BLACK);
    //DrawText(TextFormat("Factory %u %u", factory->state, factory->work), r.x+10, r.y+20, 10, BLACK);

    for(int item=0; item<4; item++) {
        DrawText(TextFormat("%u", factory->items[item]), r.x+10, r.y+30 + item*10, 10, BLACK);
        const Rectangle item_rect = {
            .x = r.x + 20,
            .y = r.y + 40 + item * 30,
            .width = 20,
            .height = 20,
        };
        if (factory->items[item] != 0) {
            DrawRectangleRec(item_rect, get_item_color(factory->items[item]));
        }
        DrawRectangleLinesEx(item_rect, 2.0f, WHITE);
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

void render_belt(render_state_t *rs, const belt_t *belt, Rectangle r) {

    //size_t total_belt_work = BELT_WORK_PER_ITEM * BELT_ITEM_COUNT;
    //float anim_progress = (float)(rs->ticks % total_belt_work) / (float)total_belt_work;
    // 0..1

    const float item_w = (float)WORLD_CELL_SIZE / (float)BELT_ITEM_COUNT;
    const float lane_size = item_w * 1.5f;

    Rectangle in_rect = {};
    Rectangle out_rect = {};

    switch (belt->in_dir) {
        case DIR_RIGHT:
            in_rect = (Rectangle) {
                .x = r.x,
                .y = r.y + r.height * 0.5f - lane_size * 0.5f,
                .width = r.width * 0.5f,
                .height = lane_size,
            };
            break;

        case DIR_LEFT:
            in_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f,
                .y = r.y + r.height * 0.5f - lane_size * 0.5f,
                .width = r.width * 0.5f,
                .height = lane_size,
            };
            break;

        case DIR_DOWN:
            in_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f - lane_size * 0.5f,
                .y = r.y,
                .width = lane_size,
                .height = r.height * 0.5f,
            };
            break;

        case DIR_UP:
            in_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f - lane_size * 0.5f,
                .y = r.y + r.height * 0.5f,
                .width = lane_size,
                .height = r.height * 0.5f,
            };
            break;
    }

    switch (belt->out_dir) {
        case DIR_RIGHT:
            out_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f,
                .y = r.y + r.height * 0.5f - lane_size * 0.5f,
                .width = r.width * 0.5f,
                .height = lane_size,
            };
            break;

        case DIR_LEFT:
            out_rect = (Rectangle) {
                .x = r.x,
                .y = r.y + r.height * 0.5f - lane_size * 0.5f,
                .width = r.width * 0.5f,
                .height = lane_size,
            };
            break;

        case DIR_DOWN:
            out_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f - lane_size * 0.5f,
                .y = r.y + r.height * 0.5f,
                .width = lane_size,
                .height = r.height * 0.5f,
            };
            break;

        case DIR_UP:
            out_rect = (Rectangle) {
                .x = r.x + r.width * 0.5f - lane_size * 0.5f,
                .y = r.y,
                .width = lane_size,
                .height = r.height * 0.5f,
            };
            break;
    }

    Rectangle in_rect_border = in_rect;
    in_rect_border.x -= 2;
    in_rect_border.y -= 2;
    in_rect_border.width += 4;
    in_rect_border.height += 4;

    Rectangle out_rect_border = out_rect;
    out_rect_border.x -= 2;
    out_rect_border.y -= 2;
    out_rect_border.width += 4;
    out_rect_border.height += 4;

    DrawRectangleRec(in_rect_border, BLACK); // super ugly&hackish border
    DrawRectangleRec(out_rect_border, BLACK);

    DrawRectangleRec(in_rect, GRAY);
    DrawRectangleRec(out_rect, GRAY);

    if (belt->in_dir != belt->out_dir || belt->in_dir == DIR_NONE || belt->out_dir == DIR_NONE) {
        Vector2 joint_pos = (Vector2) {
            .x = r.x + r.width * 0.5f,
            .y = r.y + r.height * 0.5f,
        };
        float joint_radius = lane_size * 0.5f;
        DrawCircleV(joint_pos, joint_radius, GRAY);
    }

    // Debug
    //DrawText(TextFormat("%u/%u", belt->in_dir, belt->out_dir), r.x, r.y, 10, WHITE);
}

void render_belt_items(render_state_t *rs, const belt_t *belt, Rectangle r) {

    const float item_w = (float)WORLD_CELL_SIZE / (float)BELT_ITEM_COUNT;
    const float item_h = item_w;

    for (size_t item=0; item<BELT_ITEM_COUNT; item++) {
        float off_factor = (float)belt->works[item] / (float)BELT_WORK_PER_ITEM;
        float item_x, item_y;
        uint8_t relevant_dir = (item < BELT_ITEM_COUNT/2) ? belt->in_dir : belt->out_dir;

        switch (relevant_dir) {
            case DIR_RIGHT:
                item_x = r.x + (float)item*item_w + off_factor*item_w;
                item_y = r.y + WORLD_CELL_SIZE*0.5f;
                break;

            case DIR_LEFT:
                item_x = r.x + WORLD_CELL_SIZE - (float)item*item_w - off_factor*item_w;
                item_y = r.y + WORLD_CELL_SIZE*0.5f;
                break;

            case DIR_DOWN:
                item_x = r.x + WORLD_CELL_SIZE*0.5f;
                item_y = r.y + (float)item*item_h + off_factor*item_h;
                break;

            case DIR_UP:
                item_x = r.x + WORLD_CELL_SIZE*0.5f;
                item_y = r.y + WORLD_CELL_SIZE - (float)item*item_h - off_factor*item_h;
                break;

            default:
                item_x = r.x + WORLD_CELL_SIZE*0.5f + (item%(BELT_ITEM_COUNT/2))*10.0f + off_factor*10.0f ;
                item_y = r.y + WORLD_CELL_SIZE*0.5f + (item%(BELT_ITEM_COUNT/2))*10.0f + off_factor*10.0f ;
                break;
        }

        const Rectangle item_rect = {
            .x = item_x - item_w*0.5f + 1,
            .y = item_y - item_h*0.5f + 1,
            .width = item_w - 2,
            .height = item_h - 2,
        };

        if (belt->items[item]) {
            DrawRectangleRec(item_rect, get_item_color(belt->items[item]));
        }
    }

}

void render_world(const game_state_t *gs, render_state_t *rs,
        size_t building_count, size_t *building_ids) {
    assert(gs);
    assert(rs);

    rs->ticks++;

    for(uint32_t i=0; i<building_count; i++) {
        const building_t *b = gs->buildings + building_ids[i];
        const Vector2 world_pos = coord_to_world_position(b->pos);
        const Rectangle r = {
            .x = world_pos.x,
            .y = world_pos.y,
            .width = b->size.w * WORLD_CELL_SIZE,
            .height = b->size.h * WORLD_CELL_SIZE,
        };

        switch (b->type) {
            case BUILDING_TYPE_MINER: render_miner(rs, gs->miners + b->data_index, r); break;
            case BUILDING_TYPE_FACTORY: render_factory(rs, gs->factories + b->data_index, r); break;
            case BUILDING_TYPE_BELT: render_belt(rs, gs->belts + b->data_index, r); break;
        }
    }

    for(uint32_t i=0; i<building_count; i++) {
        const building_t *b = gs->buildings + building_ids[i];
        const Vector2 world_pos = coord_to_world_position(b->pos);
        const Rectangle r = {
            .x = world_pos.x,
            .y = world_pos.y,
            .width = b->size.w * WORLD_CELL_SIZE,
            .height = b->size.h * WORLD_CELL_SIZE,
        };

        switch (b->type) {
            case BUILDING_TYPE_BELT: render_belt_items(rs, gs->belts + b->data_index, r); break;
        }
    }
}

