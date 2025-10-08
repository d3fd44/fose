#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>

#include "raylib.h"

#define ARR_TIP_WIDTH  5
#define ARR_TIP_HEIGHT 10
#define ARR_TIP_DIAG   11.180339887f
#define ARR_POINTS     6
#define ARR_BASE       0
#define ARR_BASE_END   1
#define ARR_TIP_LEFT   2
#define ARR_TIP_END    3
#define ARR_TIP_RIGHT  4
#define ARR_TIP_LAST   5

#ifndef UNIT_SIZE
# define UNIT_SIZE 200
#endif

#define container_of(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))

#define CRIMSON_ROSE                    (Color){ 230, 65, 91, 255 }
#define AQUA_MINT                       (Color){ 70, 239, 207, 255 }
#define INDIGO_VIOLET                   (Color){ 92, 74, 230, 255 }
#define DEEP_SPACE                      (Color){ 24, 21, 34, 255 }
#define MISTY_GRAY                      (Color){ 230, 232, 240, 255 }

typedef struct Harmonic
{
    int              n;
    double           coe;  // useless for now
    double           mag;
    double           omega;
    double           phase;
    Vector2          points[6];
    struct Harmonic* next;
    struct Harmonic* prev;

} Harmonic;

// clang-format off
Harmonic* series = NULL;
Harmonic* tail   = NULL;
int       n      = 0;
// clang-format on

// ** TODO ** it works fine, but looks shit
void mkharmonic(double coe, double omega, double phase)
{
    Harmonic* new = (Harmonic*)calloc(1, sizeof(Harmonic));
    assert(new);

    new->n = n++;
    new->mag = UNIT_SIZE* coe;
    new->omega = omega;
    new->phase = phase;

    if (series == NULL)
    {
        series = new;
        tail = new;
        series->next = NULL;
        series->prev = NULL;

        series->points[ARR_BASE] = (Vector2){ 0, 0 };
        float basex = series->points[ARR_BASE].x;
        float basey = series->points[ARR_BASE].y;

        series->points[ARR_TIP_LAST] = series->points[ARR_BASE_END]
          = (Vector2){ ((series->mag - ARR_TIP_HEIGHT) * cos(phase)) + basex,
                       -((series->mag - ARR_TIP_HEIGHT) * sin(phase)) + basey };
        series->points[ARR_TIP_END]
          = (Vector2){ series->mag * cos(phase) + basex, -series->mag * sin(phase) + basey };

        double arrleftdeg = atan(tan(ARR_TIP_WIDTH / (series->mag - ARR_TIP_HEIGHT))) + phase;
        double arrrightdeg = phase - atan(tan(ARR_TIP_WIDTH / (series->mag - ARR_TIP_HEIGHT)));
        double diag = sqrt(25 + ((series->mag - ARR_TIP_HEIGHT) * (series->mag - ARR_TIP_HEIGHT)));

        series->points[ARR_TIP_LEFT]
          = (Vector2){ diag * cos(arrleftdeg) + basex, -diag * sin(arrleftdeg) + basey };
        series->points[ARR_TIP_RIGHT]
          = (Vector2){ diag * cos(arrrightdeg) + basex, -diag * sin(arrrightdeg) + basey };

        return;
    }
    tail->next = new;
    tail->next->prev = tail;
    tail = new;

    tail->points[ARR_BASE] = tail->prev->points[ARR_TIP_END];
    float basex = tail->points[ARR_BASE].x;
    float basey = tail->points[ARR_BASE].y;

    tail->points[ARR_TIP_LAST] = tail->points[ARR_BASE_END]
      = (Vector2){ ((tail->mag - ARR_TIP_HEIGHT) * cos(phase)) + basex,
                   -((tail->mag - ARR_TIP_HEIGHT) * sin(phase)) + basey };
    tail->points[ARR_TIP_END]
      = (Vector2){ tail->mag * cos(phase) + basex, -tail->mag * sin(phase) + basey };

    double arrleftdeg = atan(tan(ARR_TIP_WIDTH / (tail->mag - ARR_TIP_HEIGHT))) + phase;
    double arrrightdeg = phase - atan(tan(ARR_TIP_WIDTH / (tail->mag - ARR_TIP_HEIGHT)));
    double diag = sqrt(25 + ((tail->mag - ARR_TIP_HEIGHT) * (tail->mag - ARR_TIP_HEIGHT)));

    tail->points[ARR_TIP_LEFT]
      = (Vector2){ diag * cos(arrleftdeg) + basex, -diag * sin(arrleftdeg) + basey };
    tail->points[ARR_TIP_RIGHT]
      = (Vector2){ diag * cos(arrrightdeg) + basex, -diag * sin(arrrightdeg) + basey };

    tail->next = NULL;
}

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
    Harmonic* prev = container_of(points, Harmonic, points)->prev;
    Vector2   base = { 0 };

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

void updateState(Harmonic* series)
{
    Harmonic* cur = series;
    float     omega;

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
    mkharmonic(1.0f, 1.0f, PI / 2 + PI / 4);
    mkharmonic(1.0f, -2.0f, 0 + PI / 4);
    mkharmonic(0.5f, 4, PI / 2 + PI / 4);

    Harmonic* cur = series;

    InitWindow(0, 0, "GG");

    SetTargetFPS(60);
    ToggleBorderlessWindowed();

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
        updateState(series);

        BeginDrawing();
                BeginMode2D(cam);

                        ClearBackground(RAYWHITE);
                        for (int x = -2000; x <= 2000; x += UNIT_SIZE) DrawLine(x, -2000, x, 2000, DEEP_SPACE);
                        for (int y = -2000; y <= 2000; y += UNIT_SIZE) DrawLine(-2000, y, 2000, y, DEEP_SPACE);

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
                        
                        DrawLineEx(line[0], line[1], 10.0f, DEEP_SPACE);

                EndMode2D();
        EndTextureMode();
        // clang-format on
    }

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
