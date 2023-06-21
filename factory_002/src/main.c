#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

#include "game_state.h"
#include "renderer.h"

int main(void) {
    InitWindow(1600, 1200, "bubu");
    SetWindowState(FLAG_WINDOW_MAXIMIZED);
    SetTargetFPS(60);

    Camera2D camera = {};
    camera.target = (Vector2) { -100.0f, -100.0f };
    camera.offset = (Vector2) { 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    game_state_t game_state = {};
    reset_game_state(&game_state);

    render_state_t render_state = {};

    int build_mode = 0;
    size_t selected_building = 0;

    bool game_update_enabled = true;
    bool game_update_once = false;

    // -----------------
    {
        size_t miner1a = spawn_miner(&game_state, (coord_t) { 1, 1 });
        size_t belt1a = spawn_belt(&game_state, (coord_t) { 3, 1 });
        size_t belt2a = spawn_belt(&game_state, (coord_t) { 4, 1 });
        size_t factory1a = spawn_factory(&game_state, (coord_t) { 5, 1 });
        size_t belt3a = spawn_belt(&game_state, (coord_t) { 7, 1 });

        connect_buildings(&game_state, miner1a, belt1a);
        connect_buildings(&game_state, belt1a, belt2a);
        connect_buildings(&game_state, belt2a, factory1a);
        connect_buildings(&game_state, factory1a, belt3a);

        // Note: same as above but in reverse
        size_t belt3b = spawn_belt(&game_state, (coord_t) { 7, 4 });
        size_t factory1b = spawn_factory(&game_state, (coord_t) { 5, 4 });
        size_t belt2b = spawn_belt(&game_state, (coord_t) { 4, 4 });
        size_t belt1b = spawn_belt(&game_state, (coord_t) { 3, 4 });
        size_t miner1b = spawn_miner(&game_state, (coord_t) { 1, 4 });

        size_t belt4b = spawn_belt(&game_state, (coord_t) { 8,4 });
        size_t belt5b = spawn_belt(&game_state, (coord_t) { 8,3 });

        connect_buildings(&game_state, miner1b, belt1b);
        connect_buildings(&game_state, belt1b, belt2b);
        connect_buildings(&game_state, belt2b, factory1b);
        connect_buildings(&game_state, factory1b, belt3b);
        connect_buildings(&game_state, belt3b, belt4b);
        connect_buildings(&game_state, belt4b, belt5b);

        size_t factory2 = spawn_factory(&game_state, (coord_t) { 8,1 });
        size_t belt10 = spawn_belt(&game_state, (coord_t) { 10,2 });

        connect_buildings(&game_state, belt3a, factory2);
        connect_buildings(&game_state, belt5b, factory2);
        connect_buildings(&game_state, factory2, belt10);
    }
    // -----------------
    {
        size_t miner1 = spawn_miner(&game_state, (coord_t) { 1, 7 });
        size_t belt1 = spawn_belt(&game_state, (coord_t) { 3, 7 });
        size_t belt2 = spawn_belt(&game_state, (coord_t) { 3, 8 });
        size_t belt3 = spawn_belt(&game_state, (coord_t) { 4, 8 });
        size_t belt4 = spawn_belt(&game_state, (coord_t) { 4, 7 });
        size_t belt5 = spawn_belt(&game_state, (coord_t) { 4, 6 });
        size_t belt6 = spawn_belt(&game_state, (coord_t) { 3, 6 });

        connect_buildings(&game_state, miner1, belt1);
        connect_buildings(&game_state, belt1, belt2);
        connect_buildings(&game_state, belt2, belt3);
        connect_buildings(&game_state, belt3, belt4);
        connect_buildings(&game_state, belt4, belt5);
        connect_buildings(&game_state, belt5, belt6);
    }
    // -----------------

    while(!WindowShouldClose()) {

        if (game_update_enabled || game_update_once) {
            game_update_once = false;
            update_game_state(&game_state);
        }

        Vector2 mouse_pos_screen = GetMousePosition();
        Vector2 mouse_pos_world = GetScreenToWorld2D(mouse_pos_screen, camera);
        coord_t mouse_coord = world_position_to_coord(mouse_pos_world);

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
            camera.zoom += wheel * 0.15f;
            if (camera.zoom < 0.1f) camera.zoom = 0.1f;
        }

        // Building        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            size_t clicked_building = get_building(&game_state, mouse_coord);
            if (clicked_building) {
                selected_building = clicked_building;
            } else {
                switch (build_mode) {
                    case 0:
                        selected_building = spawn_miner(&game_state, mouse_coord);
                        break;
                    case 1:
                        selected_building = spawn_factory(&game_state, mouse_coord);
                        break;
                    case 2:
                        selected_building = spawn_belt(&game_state, mouse_coord);
                        break;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            selected_building = 0;
        }

        if (IsKeyPressed(KEY_ONE)) build_mode = 0;
        if (IsKeyPressed(KEY_TWO)) build_mode = 1;
        if (IsKeyPressed(KEY_THREE)) build_mode = 2;
        if (IsKeyPressed(KEY_FOUR)) build_mode = 3;

        // Settings
        if (IsKeyPressed(KEY_U)) game_update_enabled = !game_update_enabled;
        if (IsKeyPressed(KEY_T)) game_update_once = true;

        /*if (IsKeyPressed(KEY_F)) {
            ToggleFullscreen();
        }*/


        BeginDrawing();
            ClearBackground(BROWN);

            BeginMode2D(camera);
                // Grid
                DrawLine(-1000, 0, 1000, 0, BLACK);
                DrawLine(0, -1000, 0, 1000, BLACK);
                DrawLine(-1000, -WORLD_CELL_SIZE, 1000, -WORLD_CELL_SIZE, BLACK);
                DrawLine(-1000, WORLD_CELL_SIZE, 1000, WORLD_CELL_SIZE, BLACK);
                DrawLine(-WORLD_CELL_SIZE, -1000, -WORLD_CELL_SIZE, 1000, BLACK);
                DrawLine(WORLD_CELL_SIZE, -1000, WORLD_CELL_SIZE, 1000, BLACK);

                // World
                render_world(&game_state, &render_state); 

                // Mouse
                {
                    //Vector2 mmm = coord_to_world_position(mouse_coord);
                    //DrawRectangle(mmm.x-10, mmm.y-10, 20, 20, PINK);
                }

                // Selection
                if (selected_building != 0) {
                    building_t *sel_b = game_state.buildings + selected_building;
                    Vector2 wp = coord_to_world_position(sel_b->pos);
                    Rectangle r = {
                        .x = wp.x,
                        .y = wp.y,
                        .width = WORLD_CELL_SIZE,
                        .height = WORLD_CELL_SIZE,
                    };
                    DrawRectangleLinesEx(r, 5.0f, RED);
                }

            EndMode2D();

            int next_text_y = 0;
            DrawFPS(10, next_text_y+=20);
            //DrawText(TextFormat("cam target %.1f %0.1f", camera.target.x, camera.target.y), 10, next_text_y+=20, 20, WHITE);
            //DrawText(TextFormat("cam offset %.1f %0.1f", camera.offset.x, camera.offset.y), 10, next_text_y+=20, 20, WHITE);
            //DrawText(TextFormat("cam rotation %.1f", camera.rotation), 10, next_text_y+=20, 20, WHITE);
            //DrawText(TextFormat("cam zoom %.1f", camera.zoom), 10, next_text_y+=20, 20, WHITE);
            //DrawText(TextFormat("bm %d", build_mode), 10, next_text_y+=20, 20, WHITE);
            //DrawText(TextFormat("buildings %u", game_state.building_count), 10, next_text_y+=20, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

