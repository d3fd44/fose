fose - fourier series (i know, ugly name)
-----------------------------------------

just hit `make` to build.\
dependencies:
- raylib
- cJSON

make sure both are installed.

**The app currently has the worst interface ever created, you have to get the points of the shape you want to draw out of the image yourself, and do FFT yourself, and then place the output nicely in a json file.** Well, there's an FFT utility i prepared for you (with an even worse interface :D).

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

To-Do:
---
- [ ] nuke the json mechanism and replace it with something more usable/efficient
- [ ] integrate the FFT module into the app 
- [ ] add free move/zoom feature
- [ ] add simple cli interface with argument processing for some options (colors, scaling, etc).
- [ ] add export-to-video feature through ffmpeg (seems possible based on tsoding-yt)

demo:
---



https://github.com/user-attachments/assets/2e31cec0-b5c3-496d-9e0a-d29b7126fa26


