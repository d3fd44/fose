#include <assert.h>
#include <cjson/cJSON.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define UNIT_SIZE        200
#define OMEGA_MULTIPLIER 1.0f
#define ARROW_THICKNESS  2.0f

#define DEEP_SPACE       (Color){ 24, 21, 34, 255 }

typedef struct Harmonic
{
    double           mag;
    double           omega;
    double           phase;
    double           theta;
    Vector2         *base; // points to the preious harmonic's tip; translate a single point instead of two
    Vector2          tip;
    struct Harmonic *next;
    struct Harmonic *prev;

} Harmonic;

Harmonic *series_head = NULL;
Harmonic *series_tail = NULL;

Vector2 ORIGIN = { 0.0f, 0.0f };

// idk why i am so pissed off from the json mechanism i did here...
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
        cur->theta += cur->omega * dt * OMEGA_MULTIPLIER;
        cur->tip.x = cur->base->x + (cur->mag * cos(cur->theta));
        cur->tip.y = cur->base->y - (cur->mag * sin(cur->theta));

        cur = cur->next;
    }
}

// read json file, parse it, pass each harmonic to mkharmonic function and add it to the list
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
        int    i = 0;

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

// increase/decrease thickness based on the magnitude
float give_thickness(float mag)
{
    float thickness = ARROW_THICKNESS * powf(mag / UNIT_SIZE, 0.85f);

    return Clamp(thickness, 0.00001f, 6.0f);
}

int main(int argc, char *argv[])
{
    init(argc > 1 ? argv[1] : "./harmonics.json");

    SetTraceLogLevel(LOG_NONE);
    // i'll handle resizing and zooming later, let it full screen for now
    // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(0, 0, "GG");
    SetTargetFPS(60);

    bool paused = false;
    bool follow = false;
    bool show_help = true;

    Harmonic *cur;

    int screen_height = GetScreenHeight();
    int screen_width = GetScreenWidth();

    Camera2D cam = { 0 };
    cam.target = (Vector2){ 0.0f, 0.0f };
    cam.offset = (Vector2){ screen_width * 0.5f, screen_height * 0.5f };
    cam.rotation = 0.0f;
    cam.zoom = 1.0f;

    Camera2D canvas_cam = { 0 };
    canvas_cam.target = (Vector2){ 0.0f, 0.0f };
    canvas_cam.offset = (Vector2){ screen_width * 0.5f, screen_height * 0.5f };
    canvas_cam.rotation = 0.0f;
    canvas_cam.zoom = 1.0f;

    // shape drawing happens in this canvas
    RenderTexture2D canvas = LoadRenderTexture(screen_width, screen_height);

    // clang-format off
    BeginTextureMode(canvas);
        ClearBackground((Color){ 0, 0, 0, 0 });
    EndTextureMode();

    Vector2 previous_tip = series_tail->tip;

    while (!WindowShouldClose())
    {
        cam.zoom = expf(logf(cam.zoom) + ((float)GetMouseWheelMove() * 0.075f));

        if (cam.zoom < 0.1f)
            cam.zoom = 0.1f;

        if (follow)
            cam.target = series_tail->tip;

        switch (GetKeyPressed())
        {
            case KEY_H:
                show_help = !show_help;
                break;
            case KEY_F:
                follow = !follow;
                break;
            case KEY_SPACE:
                paused = !paused;
                break;
            case KEY_C:
                BeginTextureMode(canvas);
                    ClearBackground((Color){ 0, 0, 0, 0 });
                EndTextureMode();
                break;
            case KEY_R:
                follow = false;
                cam.target = Vector2Zero();
                cam.zoom = 1.0f;
                break;
            case KEY_W:
            case KEY_UP:
            case KEY_A:
            case KEY_LEFT:
            case KEY_S:
            case KEY_DOWN:
            case KEY_D:
            case KEY_RIGHT:
                follow = false;
                break;
            default:
                break;
        }

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
            cam.target.y -= 10 * (1 / cam.zoom);
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
            cam.target.x -= 10 * (1 / cam.zoom);
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
            cam.target.y += 10 * (1 / cam.zoom);
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
            cam.target.x += 10 * (1 / cam.zoom);

        if (!paused)
        {
            translate_series(series_head);

            Vector2 currentTip = series_tail->tip;

            BeginTextureMode(canvas);
                BeginMode2D(canvas_cam);
                    DrawLineEx(previous_tip, currentTip, 1.0f, BLUE);
                EndMode2D();
            EndTextureMode();

            previous_tip = currentTip;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            BeginMode2D(cam);

                for (int x = -2000; x <= 2000; x += UNIT_SIZE)
                    DrawLine(x, -2000, x, 2000, DEEP_SPACE);

                for (int y = -2000; y <= 2000; y += UNIT_SIZE)
                    DrawLine(-2000, y, 2000, y, DEEP_SPACE);

                DrawTextureRec(
                    canvas.texture,
                    (Rectangle){ 0.0f, 0.0f, (float)canvas.texture.width, -(float)canvas.texture.height },
                    (Vector2){ -canvas.texture.width * 0.5f, -canvas.texture.height * 0.5f },
                    WHITE
                );

                cur = series_head;

                while (cur->next != NULL)
                {
                    // draw arrow's line
                    DrawLineEx(
                        *cur->base,
                        Vector2MoveTowards(cur->tip, *cur->base, cur->mag * 0.125f), // scale down to avoid overlapping with the triangle head
                        give_thickness(cur->mag),
                        BLACK
                    );
                    DrawCircleV(*cur->base, give_thickness(cur->mag) * 0.5f, BLACK); // covers the sharp edges

                    // draw arrow's triangle head
                    float length = Vector2Length(Vector2Subtract(cur->tip, *cur->base));
                    if (length >= 0.000001f) // zero guard
                    {
                        Vector2 unit_vector = Vector2Scale(Vector2Subtract(cur->tip, *cur->base), 1 / length);
                        Vector2 normal_vector = { -unit_vector.y, unit_vector.x }; // the perpendicular vector, idk if the naming here is correct

                        float head_height = cur->mag * 0.125f; // same sclaing-down value applied to the line
                        float head_base = head_height * 0.866f; // the height ration to the base length in 0.866 in equilateral triangle

                        // the point where it goes perpendicularly to the left/right from the original line to form the triangle head with the tip
                        Vector2 head_anchor = Vector2Subtract(cur->tip, Vector2Scale(unit_vector, head_height));
                        DrawTriangle(
                            cur->tip,
                            Vector2Subtract(head_anchor, Vector2Scale(normal_vector, head_base * 0.5f)),
                            Vector2Add(head_anchor, Vector2Scale(normal_vector, head_base * 0.5f)),
                            BLACK
                        );
                    }

                    cur = cur->next;
                }

                DrawLineEx(*cur->base, Vector2MoveTowards(cur->tip, *cur->base, cur->mag * 0.125f), give_thickness(cur->mag), RED);
                DrawCircleV(*cur->base, give_thickness(cur->mag) * 0.5f, RED);

                float length = Vector2Length(Vector2Subtract(cur->tip, *cur->base));
                if (length >= 0.000001f)
                {
                    Vector2 unit_vector = Vector2Scale(Vector2Subtract(cur->tip, *cur->base), 1 / length);
                    Vector2 normal_vector = { -unit_vector.y, unit_vector.x };

                    float head_height = cur->mag * 0.125f;
                    float head_base = head_height * 0.866f;

                    Vector2 head_anchor = Vector2Subtract(cur->tip, Vector2Scale(unit_vector, head_height));
                    DrawTriangle(
                        cur->tip,
                        Vector2Subtract(head_anchor, Vector2Scale(normal_vector, head_base * 0.5f)),
                        Vector2Add(head_anchor, Vector2Scale(normal_vector, head_base * 0.5f)),
                        RED
                    );
                }

            EndMode2D();
            DrawFPS(10, 10);
            if (show_help)
            {
                Rectangle help_rect = { screen_width - 740, 20, 720, 320 };
                DrawRectangleRec(help_rect, WHITE);
                DrawRectangleLinesEx(help_rect, 8.0f, BLACK);
                DrawText("H: Show/Hide Help\n"
                         "F: Follow Drawing Head\n"
                         "C: Clear Canvas\n"
                         "R: Reset Zoom\n"
                         "<Space>: Stop/Continue Drawing\n"
                         "<WASD/Arrows>: Move\n"
                         "<Mouse-Wheel>: Zoom",
                    help_rect.x + 20,
                    help_rect.y + 20,
                    40,
                    BLACK
                );
            }
        EndDrawing();
    }
    // clang-format on

    UnloadRenderTexture(canvas);
    CloseWindow();

    return 0;
}
