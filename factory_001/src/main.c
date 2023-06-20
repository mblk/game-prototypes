#include <stdio.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "game_state.h"

int main(void) {
    InitWindow(800, 600, "bubu");
    SetTargetFPS(60);

    Camera2D camera = {};
    camera.target = (Vector2) { -100.0f, -100.0f };
    camera.offset = (Vector2) { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    game_state_t game_state = {};
    reset_game_state(&game_state);

    int build_mode = 0;
    building_t *selected_building = NULL;

    building_t *miner1 = spawn_miner(&game_state, (coord_t) { 1, 1 });
    building_t *belt1 = spawn_belt(&game_state, (coord_t) { 2, 1 });
    building_t *belt2 = spawn_belt(&game_state, (coord_t) { 3, 1 });
    building_t *factory1 = spawn_factory(&game_state, (coord_t) { 4, 1 });
    building_t *belt3 = spawn_belt(&game_state, (coord_t) { 5, 1 });

    connect_buildings(&game_state, miner1, belt1);
    connect_buildings(&game_state, belt1, belt2);
    connect_buildings(&game_state, belt2, factory1);
    connect_buildings(&game_state, factory1, belt3);

    while(!WindowShouldClose()) {

        update_game_state(&game_state);

        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / camera.zoom);
            camera.target = Vector2Add(camera.target, delta);
        }
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_pos_screen = GetMousePosition();
            Vector2 mouse_pos_world = GetScreenToWorld2D(mouse_pos_screen, camera);
            coord_t building_pos = world_position_to_coord(mouse_pos_world);

            building_t *clicked_building = get_building(&game_state, building_pos);
            if (clicked_building) {
                selected_building = clicked_building;
            } else {
                switch (build_mode) {
                    case 0:
                        spawn_miner(&game_state, building_pos);
                        break;
                    case 1:
                        spawn_factory(&game_state, building_pos);
                        break;
                    case 2:
                        spawn_belt(&game_state, building_pos);
                        break;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            selected_building = NULL;
        }

        if (IsKeyPressed(KEY_ONE)) build_mode = 0;
        if (IsKeyPressed(KEY_TWO)) build_mode = 1;
        if (IsKeyPressed(KEY_THREE)) build_mode = 2;
        if (IsKeyPressed(KEY_FOUR)) build_mode = 3;

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
            camera.offset = GetMousePosition();
            camera.target = mouseWorldPos;
            camera.zoom += wheel * 0.15f;
            if (camera.zoom < 0.1f) camera.zoom = 0.1f;
        }

        BeginDrawing();
            ClearBackground(BROWN);

            BeginMode2D(camera);
                // Grid
                DrawLine(-1000, 0, 1000, 0, BLACK);
                DrawLine(0, -1000, 0, 1000, BLACK);

                // Buildings
                for(uint32_t i=1; i<game_state.building_count; i++) {
                    const building_t *b = &game_state.buildings[i];
                    Vector2 wp = coord_to_world_position(b->pos);
                    int gap = BUILDING_HEIGHT / 10;
                    int x = wp.x + gap;
                    int y = wp.y + gap;
                    int w = BUILDING_WIDTH - 2 * gap;
                    int h = BUILDING_HEIGHT - 2 * gap;
                    DrawRectangle(x, y, w, h, GRAY);
                    DrawRectangleLines(x, y, w, h, b == selected_building ? RED : BLACK);
                    DrawText(TextFormat("%u/%u", b->type, b->data_index), x+10, y+10, 20, BLACK);

                    switch (b->type) {
                        case BUILDING_TYPE_MINER:
                            {
                            const miner_t *miner = game_state.miners + b->data_index;
                            DrawText(TextFormat("M %u", miner->work), x+10, y+30, 20, BLACK);
                            break;
                            }

                        case BUILDING_TYPE_FACTORY:
                            {
                            const factory_t *factory = game_state.factories + b->data_index;
                            DrawText(TextFormat("F %u %u", factory->state, factory->work), x+10, y+30, 20, BLACK);
                            for(int item=0; item<4; item++) {
                                DrawText(TextFormat("%u", factory->items[item]), x+10, y+50 + item*20, 20, BLACK);
                            }
                            break;
                            }

                        case BUILDING_TYPE_BELT:
                            {
                            const belt_t *belt = game_state.belts + b->data_index;
                            DrawText(TextFormat("B"), x+10, y+30, 20, BLACK);
                            for(int item=0; item<8; item++) {
                                DrawText(TextFormat("%u", belt->items[item]), x+10, y+50 + item*20, 20, BLACK);
                            }
                            break;
                            }
                    }
                }

            EndMode2D();

            int next_text_y = 0;
            DrawFPS(10, next_text_y+=20);
            DrawText(TextFormat("cam target %.1f %0.1f", camera.target.x, camera.target.y), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("cam offset %.1f %0.1f", camera.offset.x, camera.offset.y), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("cam rotation %.1f", camera.rotation), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("cam zoom %.1f", camera.zoom), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("bm %d", build_mode), 10, next_text_y+=20, 20, WHITE);
            DrawText(TextFormat("buildings %u", game_state.building_count), 10, next_text_y+=20, 20, WHITE);

            if (selected_building) {
                DrawText(TextFormat("selected %d,%d,%u",
                            selected_building->pos.x, selected_building->pos.y, selected_building->type),
                        10, next_text_y+=20, 20, WHITE);
            }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

