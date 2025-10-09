#ifndef _FOSE_H_
#define _FOSE_H_

#include <cjson/cJSON.h>

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
# define UNIT_SIZE 80
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
    Vector2          points[6];
    struct Harmonic *next;
    struct Harmonic *prev;

} Harmonic;

char   *rjsonassert(const char *path);
void    mkharmonic(double coe, double omega, double phase);
Vector2 rotpoint(Vector2 point, Vector2 center, float av);
void    rotarr(Vector2 *points, float av);
void    mvarr(Vector2 *points, Vector2 dist);
void    updatestate(Harmonic *series);
void    init(const char *path);

#endif  // !_FOSE_H_
