#include <raylib.h>

#include "fose.h"

// clang-format off
Harmonic* series = NULL;
Harmonic* tail   = NULL;
int       n      = 0   ;
// clang-format on

int main(int argc, char* argv[])
{
    SetTraceLogLevel(LOG_NONE);

    init(argc > 1 ? argv[1] : "./harmonics.json");

    Harmonic* cur = series;

    InitWindow(0, 0, "GG");

    SetTargetFPS(60);
    // ToggleBorderlessWindowed();

    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();

    Vector2       line[2] = { tail->points[ARR_TIP_END], tail->points[ARR_TIP_END] };
    unsigned char c = 0;

    Camera2D cam = { 0 };
    cam.target = (Vector2){ 0.0f, 0.0f };
    cam.offset = (Vector2){ screenWidth * 0.5f, screenHeight * 0.5f };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    RenderTexture2D canvas = LoadRenderTexture(screenWidth, screenHeight);

    // clang-format off
    BeginTextureMode(canvas);
            ClearBackground((Color){ 0, 0, 0, 0 });
    EndTextureMode();

    while (!WindowShouldClose())
    {
        updatestate(series);

        BeginDrawing();
                BeginMode2D(cam);

                        ClearBackground(RAYWHITE);
                        for (int x = -2000; x <= 2000; x += UNIT_SIZE) DrawLine(x, -2000, x, 2000, DEEP_SPACE);
                        for (int y = -2000; y <= 2000; y += UNIT_SIZE) DrawLine(-2000, y, 2000, y, DEEP_SPACE);
                        DrawFPS(- screenWidth / 2,screenHeight /-2);

                        DrawTextureRec(
                                canvas.texture,
                                (Rectangle) {0, 0, (float)canvas.texture.width, -(float)canvas.texture.height},
                                (Vector2)   { -screenWidth * 0.5f, -screenHeight * 0.5 }, WHITE
                                );

                        cur = series;

                        while (cur->next != NULL)
                        {
                            DrawSplineLinear(cur->points, 6, 3.0f, BLUE);
                            cur = cur->next;
                        }
                        DrawSplineLinear(cur->points, 6, 3.0f, RED);
                        line[c] = cur->points[ARR_TIP_END];
                        c = (c + 1) % 2;
                                
                EndMode2D();
        EndDrawing();
        BeginTextureMode(canvas);
                BeginMode2D(cam);
                        
                        DrawLineEx(line[0], line[1], 4.5f, DEEP_SPACE);

                EndMode2D();
        EndTextureMode();
        // clang-format on
    }

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
