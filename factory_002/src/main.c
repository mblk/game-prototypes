#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <raygui.h>

#include "utils.h"
#include "game_state.h"
#include "renderer.h"

static bool rectangle_contains(Rectangle r, Vector2 p) {
    return r.x <= p.x && p.x <= r.x + r.width &&
        r.y <= p.y && p.y <= r.y + r.height;
}

static void build_some_stuff(game_state_t *gs, int32_t x, int32_t y) {
    size_t miner1a = spawn_miner(gs, (coord_t){x+1, y+1});
    size_t belt1a = spawn_belt(gs, (coord_t){x+3, y+1});
    size_t belt2a = spawn_belt(gs, (coord_t){x+4, y+1});
    size_t factory1a = spawn_factory(gs, (coord_t){x+5, y+1});
    size_t belt3a = spawn_belt(gs, (coord_t){x+7, y+1});

    connect_buildings(gs, miner1a, belt1a);
    connect_buildings(gs, belt1a, belt2a);
    connect_buildings(gs, belt2a, factory1a);
    connect_buildings(gs, factory1a, belt3a);

    // Note: same as above but in reverse
    size_t belt3b = spawn_belt(gs, (coord_t){x+7, y+4});
    size_t factory1b = spawn_factory(gs, (coord_t){x+5, y+4});
    size_t belt2b = spawn_belt(gs, (coord_t){x+4, y+4});
    size_t belt1b = spawn_belt(gs, (coord_t){x+3, y+4});
    size_t miner1b = spawn_miner(gs, (coord_t){x+1, y+4});

    size_t belt4b = spawn_belt(gs, (coord_t){x+8, y+4});
    size_t belt5b = spawn_belt(gs, (coord_t){x+8, y+3});

    connect_buildings(gs, miner1b, belt1b);
    connect_buildings(gs, belt1b, belt2b);
    connect_buildings(gs, belt2b, factory1b);
    connect_buildings(gs, factory1b, belt3b);
    connect_buildings(gs, belt3b, belt4b);
    connect_buildings(gs, belt4b, belt5b);

    size_t factory2 = spawn_factory(gs, (coord_t){x+8, y+1});
    size_t belt10 = spawn_belt(gs, (coord_t){x+10, y+2});

    connect_buildings(gs, belt3a, factory2);
    connect_buildings(gs, belt5b, factory2);
    connect_buildings(gs, factory2, belt10);
}

static void build_some_more_stuff(game_state_t *gs) {

    for (int32_t x=0; x<8; x++) {
        for (int32_t y=0; y<8; y++) {
            spawn_belt(gs, (coord_t){x,y});
        }
    }

    //size_t miner1 = spawn_miner(gs, (coord_t){1, 7});
    //size_t belt1 = spawn_belt(gs, (coord_t){3, 7});
    //size_t belt2 = spawn_belt(gs, (coord_t){3, 8});
    //size_t belt3 = spawn_belt(gs, (coord_t){4, 8});
    //size_t belt4 = spawn_belt(gs, (coord_t){4, 7});
    //size_t belt5 = spawn_belt(gs, (coord_t){4, 6});
    //size_t belt6 = spawn_belt(gs, (coord_t){3, 6});

    //connect_buildings(gs, miner1, belt1);
    //connect_buildings(gs, belt1, belt2);
    //connect_buildings(gs, belt2, belt3);
    //connect_buildings(gs, belt3, belt4);
    //connect_buildings(gs, belt4, belt5);
    //connect_buildings(gs, belt5, belt6);
}

int main(void) {
    InitWindow(1600, 1200, "bubu");
    SetTargetFPS(60);

    Camera2D camera = {};
    camera.target = (Vector2){-100.0f, -100.0f};
    camera.offset = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    int32_t zoom_level = 0;

    // "game-state double-buffer"
    game_state_t *game_state_1 = create_game_state();
    game_state_t *game_state_2 = create_game_state();
    game_state_t *active_gs = game_state_1;
    game_state_t *next_gs = game_state_2;

    render_state_t render_state = {};

    size_t selected_building = 0;
    size_t building_recipe = 0;

    bool game_update_enabled = true;
    bool game_update_once = false;
    bool render_quad_tree = false;

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    int ui_height = 300;
    int ui_width = 150;

    const Rectangle ui_rect = {
        .x = 0,
        .y = screen_height / 2 - ui_height / 2,
        .width = ui_width,
        .height = ui_height,
    };

    // -----------------
#if 1
    for (int32_t xi=0; xi<712; xi++) {
        for (int32_t yi=0; yi<10 /*100*/; yi++) {
            int32_t x = xi * 12 + 10;
            int32_t y = yi * 8;
            build_some_stuff(active_gs, x, y);
        }
    }
#endif
    build_some_more_stuff(active_gs);
    printf("entities: %lu | %lu %lu %lu\n", active_gs->building_count, active_gs->miner_count,
            active_gs->belt_count, active_gs->factory_count);
    printf("game state: %lu MiB\n", sizeof(game_state_t)/1024/1024);
    // -----------------

    while (!WindowShouldClose()) {

        if (game_update_enabled || game_update_once) {
            game_update_once = false;
            
            CHECK_TIME(update_game_state(active_gs, next_gs));

            game_state_t *temp = active_gs;
            active_gs = next_gs;
            next_gs = temp;
        }

        Vector2 mouse_pos_screen = GetMousePosition();
        Vector2 mouse_pos_world = GetScreenToWorld2D(mouse_pos_screen, camera);
        coord_t mouse_coord = world_position_to_coord(mouse_pos_world);
        bool mouse_is_over_ui = rectangle_contains(ui_rect, mouse_pos_screen);

        if (!mouse_is_over_ui) {
            // Camera
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
            }

            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
                camera.offset = GetMousePosition();
                camera.target = mouseWorldPos;

                zoom_level += (wheel > 0) ? (1) : (-1);
                camera.zoom = powf(1.2f, zoom_level);
            }

            // Building
            // if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                size_t clicked_building = get_building(active_gs, mouse_coord);
                if (clicked_building && selected_building &&
                        clicked_building != selected_building) {
                    connect_buildings(active_gs, selected_building, clicked_building);
                }
                if (clicked_building) {
                    selected_building = clicked_building;
                } else {
                    size_t new_building = 0;
                    switch (building_recipe) {
                        case 1: new_building = spawn_miner(active_gs, mouse_coord); break;
                        case 2: new_building = spawn_belt(active_gs, mouse_coord); break;
                        case 3: new_building = spawn_factory(active_gs, mouse_coord); break;
                    }
                    if (selected_building && new_building) {
                        connect_buildings(active_gs, selected_building, new_building);
                    }
                    selected_building = new_building;
                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                selected_building = 0;
            }
        }

        if (IsKeyPressed(KEY_DELETE)) {
            if (selected_building) {
                delete_building(active_gs, selected_building);
                selected_building = 0;
            }
        }

        if (IsKeyPressed(KEY_U)) game_update_enabled = !game_update_enabled;
        if (IsKeyPressed(KEY_T)) game_update_once = true;
        if (IsKeyPressed(KEY_Q)) render_quad_tree = !render_quad_tree;

        // -----
        Vector2 visible_world_min = GetScreenToWorld2D((Vector2){ 0, 0 }, camera);
        Vector2 visible_world_max = GetScreenToWorld2D((Vector2){ screen_width, screen_height }, camera);
        coord_t visible_coord_min = world_position_to_coord(visible_world_min);
        coord_t visible_coord_max = world_position_to_coord(visible_world_max);

        const quad_aabb_t qb = {
            .x_min = visible_coord_min.x - 2,
            .y_min = visible_coord_min.y - 2,
            .x_max = visible_coord_max.x + 2,
            .y_max = visible_coord_max.y + 2,
            /*.x_min = mouse_coord.x - 5,
            .y_min = mouse_coord.y - 5,
            .x_max = mouse_coord.x + 5,
            .y_max = mouse_coord.y + 5,*/
        };

        size_t qt_qr_items[1000 * 10];
        quad_tree_query_result_t qt_qr = {
            .capacity = ARRAY_LENGTH(qt_qr_items),
            .count = 0,
            .items = qt_qr_items,
        };
        quad_tree_query(active_gs->quad_tree, qb, &qt_qr);

        //printf("query result: %lu\n", qt_qr.count);
        //for (size_t i=0; i<qt_qr.count; i++) {
        //    printf("> %lu\n", qt_qr.items[i]);
        //}
        // -----

        BeginDrawing();
        {
            ClearBackground(BROWN);

            BeginMode2D(camera);
            {
                // Grid
                DrawLine(-1000, 0, 1000, 0, BLACK);
                DrawLine(0, -1000, 0, 1000, BLACK);
                DrawLine(-1000, -WORLD_CELL_SIZE, 1000, -WORLD_CELL_SIZE, BLACK);
                DrawLine(-1000, WORLD_CELL_SIZE, 1000, WORLD_CELL_SIZE, BLACK);
                DrawLine(-WORLD_CELL_SIZE, -1000, -WORLD_CELL_SIZE, 1000, BLACK);
                DrawLine(WORLD_CELL_SIZE, -1000, WORLD_CELL_SIZE, 1000, BLACK);

                if (render_quad_tree) {
                    //quad_tree_dump(active_gs->quad_tree);
                    quad_tree_render(active_gs->quad_tree);
                }

                // World
                render_world(active_gs, &render_state, qt_qr.count, qt_qr.items);

                // Mouse
                if (!mouse_is_over_ui) {
                    Vector2 mouse_pos_aligned = coord_to_world_position(mouse_coord);
                    Rectangle mouse_rect_aligned = {
                        .x = mouse_pos_aligned.x,
                        .y = mouse_pos_aligned.y,
                        .width = WORLD_CELL_SIZE,
                        .height = WORLD_CELL_SIZE,
                    };
                    DrawRectangleRec(mouse_rect_aligned, Fade(PINK, 0.33f));
                }

                // Selection
                if (selected_building != 0) {
                    building_t *sb = active_gs->buildings + selected_building;
                    Vector2 wp = coord_to_world_position(sb->pos);
                    Rectangle r = {
                        .x = wp.x,
                        .y = wp.y,
                        .width = sb->size.w * WORLD_CELL_SIZE,
                        .height = sb->size.h * WORLD_CELL_SIZE,
                    };
                    DrawRectangleLinesEx(r, 5.0f, RED);
                }
            }
            EndMode2D();

            // UI
            DrawRectangleRec(ui_rect, Fade(LIGHTGRAY, 0.5f));
            DrawRectangleLinesEx(ui_rect, 2.0f, BLACK);

            GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0);
            GuiLabel((Rectangle){ui_rect.x + 10, ui_rect.y + 0, 100, 30}, "Build mode:");

            const char *recipe_names[4] = {
                "---",
                "Miner",
                "Belt",
                "Factory",
            };

            for (size_t recipe_index = 0; recipe_index < ARRAY_LENGTH(recipe_names); recipe_index++) {
                const bool is_selected = recipe_index == building_recipe;
                const Rectangle r = {
                    .x = ui_rect.x + 25,
                    .y = ui_rect.y + 40 + recipe_index * 45,
                    .width = 100,
                    .height = 40,
                };
                int prev_style = 0;
                if (is_selected) {
                    prev_style = GuiGetStyle(BUTTON, BORDER_COLOR_NORMAL);
                    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0xFF000000);
                }
                if (GuiButton(r, recipe_names[recipe_index])) {
                    building_recipe = recipe_index;
                }
                if (is_selected) {
                    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, prev_style);
                }
            }

            int next_text_y = 0;
            DrawFPS(10, next_text_y += 20);
            DrawText(TextFormat("total buildings: %lu (%lu, %lu, %lu)", active_gs->building_count,
                        active_gs->miner_count, active_gs->belt_count, active_gs->factory_count), 
                    10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("rendered: %lu", qt_qr.count), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("zoom %d %.2f", zoom_level, camera.zoom), 10, next_text_y+=20, 20, WHITE);

            if (selected_building != 0) {
                DrawText(TextFormat("sel: %u", selected_building), 10,
                        next_text_y += 20, 20, WHITE);
            }
        }
    EndDrawing();
    }

    destroy_game_state(game_state_1);
    destroy_game_state(game_state_2);

    CloseWindow();

    return 0;
}
