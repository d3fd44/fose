#include <assert.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>

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
# define UNIT_SIZE 100
#endif

#define container_of(ptr, type, member) ((type *)((char *)(ptr) - offsetof(type, member)))

#define CRIMSON_ROSE                    (Color){ 230, 65, 91, 255 }
#define AQUA_MINT                       (Color){ 70, 239, 207, 255 }
#define INDIGO_VIOLET                   (Color){ 92, 74, 230, 255 }
#define DEEP_SPACE                      (Color){ 24, 21, 34, 255 }
#define MISTY_GRAY                      (Color){ 230, 232, 240, 255 }

typedef struct Harmonic
{
    int              n;
    double           mag;
    double           omega;
    double           phase;
    Vector2         *base;
    Vector2          tip;
    Vector2          points[6];  // this might be nuked
    struct Harmonic *next;
    struct Harmonic *prev;

} Harmonic;

Harmonic *series_head = NULL;
Harmonic *series_tail = NULL;
int       n = 0;

const Vector2 ORIGIN = (Vector2){ 0.0f, 0.0f };

char *rjsonassert(const char *path)
{
    FILE *fh = fopen(path, "r");
    assert(fh);

    fseek(fh, 0, SEEK_END);
    long fsize = ftell(fh);
    if (fsize == 0)
    {
        fclose(fh);
        assert(0);
    }
    fseek(fh, 0, SEEK_SET);

    char *buffer = (char *)calloc(fsize + 1, sizeof(char));
    if (buffer == NULL)
    {
        fclose(fh);
        assert(0);
    }

    fread(buffer, 1, fsize, fh);
    buffer[fsize] = '\0';
    fclose(fh);

    return buffer;
}

// ** TODO ** it works fine, but looks shit
void mkharmonic(double coe, double omega, double phase)
{
    Harmonic *new = (Harmonic *)calloc(1, sizeof(Harmonic));
    assert(new);

    new->n = n++;
    new->mag = UNIT_SIZE *coe;
    new->omega = omega;
    new->phase = phase;

    if (series_head == NULL)
    {
        series_head = new;
        series_tail = new;
        series_head->next = NULL;
        series_head->prev = NULL;

        series_head->points[ARR_BASE] = ORIGIN;
        float basex = series_head->points[ARR_BASE].x;
        float basey = series_head->points[ARR_BASE].y;

        series_head->points[ARR_TIP_LAST] = series_head->points[ARR_BASE_END]
          = (Vector2){ ((series_head->mag - ARR_TIP_HEIGHT) * cos(phase)) + basex,
                       -((series_head->mag - ARR_TIP_HEIGHT) * sin(phase)) + basey };
        series_head->points[ARR_TIP_END] = (Vector2){ series_head->mag * cos(phase) + basex,
                                                      -series_head->mag * sin(phase) + basey };

        double arrleftdeg = atan(tan(ARR_TIP_WIDTH / (series_head->mag - ARR_TIP_HEIGHT))) + phase;
        double arrrightdeg = phase - atan(tan(ARR_TIP_WIDTH / (series_head->mag - ARR_TIP_HEIGHT)));
        double diag
          = sqrt(25 + ((series_head->mag - ARR_TIP_HEIGHT) * (series_head->mag - ARR_TIP_HEIGHT)));

        series_head->points[ARR_TIP_LEFT]
          = (Vector2){ diag * cos(arrleftdeg) + basex, -diag * sin(arrleftdeg) + basey };
        series_head->points[ARR_TIP_RIGHT]
          = (Vector2){ diag * cos(arrrightdeg) + basex, -diag * sin(arrrightdeg) + basey };

        return;
    }
    series_tail->next = new;
    series_tail->next->prev = series_tail;
    series_tail = new;

    series_tail->points[ARR_BASE] = series_tail->prev->points[ARR_TIP_END];
    float basex = series_tail->points[ARR_BASE].x;
    float basey = series_tail->points[ARR_BASE].y;

    series_tail->points[ARR_TIP_LAST] = series_tail->points[ARR_BASE_END]
      = (Vector2){ ((series_tail->mag - ARR_TIP_HEIGHT) * cos(phase)) + basex,
                   -((series_tail->mag - ARR_TIP_HEIGHT) * sin(phase)) + basey };
    series_tail->points[ARR_TIP_END]
      = (Vector2){ series_tail->mag * cos(phase) + basex, -series_tail->mag * sin(phase) + basey };

    double arrleftdeg = atan(tan(ARR_TIP_WIDTH / (series_tail->mag - ARR_TIP_HEIGHT))) + phase;
    double arrrightdeg = phase - atan(tan(ARR_TIP_WIDTH / (series_tail->mag - ARR_TIP_HEIGHT)));
    double diag
      = sqrt(25 + ((series_tail->mag - ARR_TIP_HEIGHT) * (series_tail->mag - ARR_TIP_HEIGHT)));

    series_tail->points[ARR_TIP_LEFT]
      = (Vector2){ diag * cos(arrleftdeg) + basex, -diag * sin(arrleftdeg) + basey };
    series_tail->points[ARR_TIP_RIGHT]
      = (Vector2){ diag * cos(arrrightdeg) + basex, -diag * sin(arrrightdeg) + basey };

    series_tail->next = NULL;
}

Vector2 translate_point(Vector2 point, Vector2 center, float av)
{
    float psin = sinf(av), pcos = cosf(av);
    float x = point.x - center.x;
    float y = point.y - center.y;

    // [x'] = [x][cos theta   -sin theta]
    // [y'] = [y][sin theta    cos theta]
    return (Vector2){ center.x + (x * pcos - y * psin), center.y + (x * psin + y * pcos) };
}

void translate_arrow(Vector2 *points, float av)
{
    Harmonic *prev = container_of(points, Harmonic, points)->prev;
    Vector2   base = { 0 };

    if (prev != NULL)
        base = prev->points[ARR_TIP_END];

    for (size_t i = 0; i < ARR_POINTS; i++)
        points[i] = translate_point(points[i], base, av);
}

void move_arrow(Vector2 *points, Vector2 dist)
{
    for (size_t i = 0; i < ARR_POINTS; i++)
    {
        points[i].x += dist.x;
        points[i].y += dist.y;
    }
}

void update_state(Harmonic *series)
{
    Harmonic *cur = series;
    float     omega;

    Vector2 diff = { 0 };
    while (cur->next != NULL)
    {
        omega = cur->omega * GetFrameTime();
        Vector2 nextarrbaseold = cur->points[ARR_TIP_END];

        translate_arrow(cur->points, omega);
        diff.x -= nextarrbaseold.x - cur->points[ARR_TIP_END].x;
        diff.y -= nextarrbaseold.y - cur->points[ARR_TIP_END].y;
        move_arrow(cur->next->points, diff);
        cur = cur->next;
    }
    omega = cur->omega * GetFrameTime();
    translate_arrow(cur->points, omega);
}

void init(const char *path)
{
    cJSON *json = cJSON_Parse(rjsonassert(path));
    assert(json);

    cJSON *njson = cJSON_GetObjectItemCaseSensitive(json, "count");

    if (!cJSON_IsNumber(njson))
        assert(0);

    int total = cJSON_GetNumberValue(njson);  // unused

    cJSON *harmonics = cJSON_GetObjectItemCaseSensitive(json, "harmonics");
    if (!harmonics)
        assert(0);

    cJSON *h = NULL;

    cJSON_ArrayForEach(h, harmonics)
    {
        cJSON *mag_omega_phase[3];
        cJSON *number = NULL;
        char   i = 0;

        cJSON_ArrayForEach(number, h)
        {
            assert(number);

            mag_omega_phase[i] = number;
            i++;
        }
        mkharmonic(cJSON_GetNumberValue(mag_omega_phase[0]),
                   cJSON_GetNumberValue(mag_omega_phase[1]),
                   cJSON_GetNumberValue(mag_omega_phase[2]));
    }
}

int main(int argc, char *argv[])
{
    init(argc > 1 ? argv[1] : "./harmonics.json");

    Harmonic *cur = series_head;

    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "GG");
    SetTargetFPS(60);

    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();

    Vector2       line[2] = { series_tail->points[ARR_TIP_END], series_tail->points[ARR_TIP_END] };
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
        update_state(series_head);

        BeginDrawing();
                BeginMode2D(cam);

                        ClearBackground(RAYWHITE);
                        for (int x = -2000; x <= 2000; x += UNIT_SIZE) DrawLine(x, -2000, x, 2000, DEEP_SPACE);
                        for (int y = -2000; y <= 2000; y += UNIT_SIZE) DrawLine(-2000, y, 2000, y, DEEP_SPACE);
                        DrawFPS(- screenWidth / 2,screenHeight /-2);

                        DrawTextureRec(
                                canvas.texture,
                                (Rectangle) {0, 0, (float)canvas.texture.width, -(float)canvas.texture.height},
                                (Vector2)   { -screenWidth * 0.5f, -screenHeight * 0.5 }, WHITE);

                        cur = series_head;

                        while (cur->next != NULL)
                        {
                            DrawSplineLinear(cur->points, 6, 3.0f, DEEP_SPACE);
                            cur = cur->next;
                        }
                        DrawSplineLinear(cur->points, 6, 3.0f, RED);
                        line[c] = cur->points[ARR_TIP_END];
                        c = (c + 1) % 2;
                                
                EndMode2D();
        EndDrawing();
        BeginTextureMode(canvas);
                BeginMode2D(cam);
                        
                        DrawLineEx(line[0], line[1], 5.5f, BLUE);

                EndMode2D();
        EndTextureMode();
        // clang-format on
    }

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
