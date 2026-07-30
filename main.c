#include <assert.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>

#define UNIT_SIZE  200

#define DEEP_SPACE (Color){ 24, 21, 34, 255 }

typedef struct Harmonic
{
    double           mag;
    double           omega;
    double           phase;
    double           theta;  // current angle
    Vector2         *base;
    Vector2          tip;
    struct Harmonic *next;
    struct Harmonic *prev;

} Harmonic;

Harmonic *series_head = NULL;
Harmonic *series_tail = NULL;

Vector2 ORIGIN = (Vector2){ 0.0f, 0.0f };

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

void mkharmonic(double coe, double omega, double phase)
{
    Harmonic *new = (Harmonic *)calloc(1, sizeof(Harmonic));
    assert(new);

    new->mag = UNIT_SIZE *coe;
    new->omega = omega;
    new->phase = phase;
    new->theta = phase;

    if (series_head == NULL)
    {
        series_head = new;
        series_tail = new;
        series_head->next = NULL;
        series_head->prev = NULL;

        series_head->base = &ORIGIN;

        series_head->tip
          = (Vector2){ series_head->mag * cos(phase), -series_head->mag * sin(phase) };

        return;
    }
    series_tail->next = new;
    series_tail->next->prev = series_tail;
    series_tail = new;

    series_tail->base = &series_tail->prev->tip;
    float basex = series_tail->base->x;
    float basey = series_tail->base->y;

    series_tail->tip
      = (Vector2){ series_tail->mag * cos(phase) + basex, -series_tail->mag * sin(phase) + basey };

    series_tail->next = NULL;
}

void translate_series(Harmonic *series)
{
    Harmonic *cur = series;
    float     dt = GetFrameTime();

    while (cur != NULL)
    {
        cur->theta += cur->omega * dt;
        cur->tip.x = cur->base->x + (cur->mag * cos(cur->theta));
        cur->tip.y = cur->base->y - (cur->mag * sin(cur->theta));

        cur = cur->next;
    }
}

void init(const char *path)
{
    cJSON *json = cJSON_Parse(rjsonassert(path));
    assert(json);

    cJSON *njson = cJSON_GetObjectItemCaseSensitive(json, "count");

    if (!cJSON_IsNumber(njson))
        assert(0);

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

    Harmonic *cur;

    SetTraceLogLevel(LOG_NONE);
    InitWindow(0, 0, "GG");
    SetTargetFPS(60);

    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();

    Vector2       line[2] = { series_tail->tip, series_tail->tip };
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

        cur = series_head;

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
                            DrawLineEx(*cur->base, cur->tip, 3.0f, BLACK);
                            cur = cur->next;
                        }
                            DrawLineEx(*cur->base, cur->tip, 3.0f, RED);
                        line[c] = cur->tip;
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
