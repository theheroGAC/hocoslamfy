# Hocoslamfy for PlayStation Vita

A PlayStation Vita port of **Hocoslamfy**, a Flappy Bird-style arcade game originally created by Nebuleon Fumika.

## Features
- Full PlayStation Vita widescreen display support (960x544, 60 FPS with hardware VSync).
- Native LiveArea screen assets and custom icons.
- Per-pixel 32-bit RGBA alpha blending.
- Audio and BGM powered by SDL_mixer.
- High score saving to `ux0:data/hocoslamfy/highscore`.

## Controls
- **Cross / Start**: Fly upwards / Start game / Replay
- **Circle / Select**: Exit game
- **Start**: Pause game

## Building from source
Prerequisites:
- [VitaSDK](https://vitasdk.org/) installed and in your PATH (with `VITASDK` environment variable set).
- CMake 3.16+ and Ninja or Make.

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VITASDK/share/vita.toolchain.cmake
cmake --build build
```
The output file `hocoslamfy.vpk` will be located in the `build/` directory.

## Credits
- Original Game: Nebuleon Fumika
- Vita Port: Developed with VitaSDK
