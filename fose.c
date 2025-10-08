#include "fose.h"

#include <assert.h>
#include <cjson/cJSON.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Harmonic* series;
extern Harmonic* tail;
extern int       n;

char* rjsonassert(const char* path)
{
    FILE* fh = fopen(path, "r");
    assert(fh);

    fseek(fh, 0, SEEK_END);
    long fsize = ftell(fh);
    if (fsize == 0)
    {
        fclose(fh);
        assert(0);
    }
    fseek(fh, 0, SEEK_SET);

    char* buffer = (char*)calloc(fsize + 1, sizeof(char));
    if (buffer == NULL)
    {
        fclose(fh);
        assert(0);
    }

    fread(buffer, 1, fsize, fh);
    buffer[fsize] = '\0';  // do not forget this little guy
    fclose(fh);

    return buffer;
}

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

void updatestate(Harmonic* series)
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

void init(const char* path)
{
    cJSON* json = cJSON_Parse(rjsonassert(path));
    assert(json);

    cJSON* njson = cJSON_GetObjectItemCaseSensitive(json, "count");

    if (!cJSON_IsNumber(njson))
        assert(0);
    int total = cJSON_GetNumberValue(njson);

    cJSON* harmonics = cJSON_GetObjectItemCaseSensitive(json, "harmonics");
    if (!harmonics)
        assert(0);

    cJSON* h = NULL;

    cJSON_ArrayForEach(h, harmonics)
    {
        cJSON* mag_omega_phase[3];
        cJSON* number = NULL;
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
