fose - fourier series (i know, ugly name)
-----------------------------------------

just hit `make` to build.\
dependencies:
- raylib
- cJSON

make sure both are installed.

usage:\
    `./fose examples/random.json`

the first arg is the json file describing the series and it has the following form:
```json
{
    "count": 4,
    "harmonics":
    [
        [1.4  , -12.0 , 1.5707963267948966],
        [1    ,  1.6 ,   1.5707963267948966],
        [0.5  ,  13.6 ,   1.5707963267948966],
        [0.5  , -8 ,   1.5707963267948966]
    ]
}
```
each array has [magnitude, angular velocity (omega), phase].
