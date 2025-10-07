#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>

#include "raylib.h"

#define ARR_TIP_WIDTH                   5
#define ARR_TIP_HEIGHT                  10
#define ARR_POINTS                      6
#define ARR_TIP_END                     ((ARR_POINTS - 3))

#define container_of(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

typedef struct Harmony
{
    int             n;
    double          coe;
    double          omega;
    Vector2         points[6];
    struct Harmony* next;
    struct Harmony* prev;

} Harmony;

Harmony* series = NULL;

Vector2 rotpoint(Vector2 point, Vector2 center, float av)
{
    float psin = sinf(av), pcos = cosf(av);
    float x = point.x - center.x;
    float y = point.y - center.y;

    // [x'] = [x][cos theta   -sin theta]
    // [y'] = [y][sin theta    cos theta]
    return (Vector2){ center.x + (x * pcos - y * psin), center.y + (x * psin + y * pcos) };
}
void rotarr(Vector2* points, float av)
{
    Harmony* prev = container_of(points, Harmony, points)->prev;
    Vector2  base = { 0 };

    if (prev != NULL)
        base = prev->points[ARR_TIP_END];

    for (size_t i = 0; i < ARR_POINTS; i++)
        points[i] = rotpoint(points[i], base, av);
}

void mvarr(Vector2* points, Vector2 dist)
{
    for (size_t i = 0; i < ARR_POINTS; i++)
    {
        points[i].x += dist.x;
        points[i].y += dist.y;
    }
}

void updateState(Harmony* series)
{
    Harmony* cur = series;
    float    omega;

    Vector2 diff = { 0 };
    while (cur->next != NULL)
    {
        omega = cur->omega * GetFrameTime();
        Vector2 nextarrbaseold = cur->points[ARR_TIP_END];

        rotarr(cur->points, omega);
        diff.x -= nextarrbaseold.x - cur->points[ARR_TIP_END].x;
        diff.y -= nextarrbaseold.y - cur->points[ARR_TIP_END].y;
        mvarr(cur->next->points, diff);
        cur = cur->next;
    }
    omega = cur->omega * GetFrameTime();
    rotarr(cur->points, omega);
}

int main(void)
{
    series = calloc(1, sizeof(Harmony));
    Harmony* cur = series;
    float    base = 0;
    double   mag = 0;

    cur->n = 0;
    cur->coe = 1.0f;
    cur->omega = 2.0f;

    mag = cur->coe * 100;

    cur->points[0] = (Vector2){ 0, 0 };
    cur->points[1] = (Vector2){ mag - ARR_TIP_HEIGHT, 0 };
    cur->points[2] = (Vector2){ mag - ARR_TIP_HEIGHT, 0 - ARR_TIP_WIDTH };
    cur->points[3] = (Vector2){ mag, 0 };
    cur->points[4] = (Vector2){ mag - ARR_TIP_HEIGHT, 0 + ARR_TIP_WIDTH };
    cur->points[5] = (Vector2){ mag - ARR_TIP_HEIGHT, 0 };

    cur->prev = NULL;

    cur->next = calloc(1, sizeof(Harmony));
    cur->next->prev = cur;
    cur = cur->next;

    cur->n = 3;
    cur->coe = 0.5f;
    cur->omega = -6.0f;

    base = cur->prev->points[ARR_TIP_END].x;
    mag = cur->coe * 100;

    cur->points[0] = (Vector2){ base + 0, 0 };
    cur->points[1] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 };
    cur->points[2] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 - ARR_TIP_WIDTH };
    cur->points[3] = (Vector2){ base + mag, 0 };
    cur->points[4] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 5 };
    cur->points[5] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 };

    cur->next = calloc(1, sizeof(Harmony));
    cur->next->prev = cur;
    cur = cur->next;

    cur->n = 4;
    cur->coe = 0.5f;
    cur->omega = 2.0f;

    base = cur->prev->points[ARR_TIP_END].x;
    mag = cur->coe * 100;

    cur->points[0] = (Vector2){ base + 0, 0 };
    cur->points[1] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 };
    cur->points[2] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 - ARR_TIP_WIDTH };
    cur->points[3] = (Vector2){ base + mag, 0 };
    cur->points[4] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 5 };
    cur->points[5] = (Vector2){ base + mag - ARR_TIP_HEIGHT, 0 };

    cur->next = NULL;

    InitWindow(0, 0, "GG");

    SetTargetFPS(60);
    ToggleBorderlessWindowed();

    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();

    Vector2       line[2] = { cur->points[ARR_TIP_END], cur->points[ARR_TIP_END] };
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
    // clang-format on

    while (!WindowShouldClose())
    {
        updateState(series);

        // clang-format off
        BeginDrawing();
                BeginMode2D(cam);

                        ClearBackground((Color){ 255, 255, 255, 255 });
                        for (int x = -2000; x <= 2000; x += 100) DrawLine(x, -2000, x, 2000, (Color){ 10, 10, 10, 50 });
                        for (int y = -2000; y <= 2000; y += 100) DrawLine(-2000, y, 2000, y, (Color){ 10, 10, 10, 50 });
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
                        
                        DrawLineEx(line[0], line[1], 10.0f, BLACK);

                EndMode2D();
        EndTextureMode();
        // clang-format on
    }

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
