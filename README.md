# Explode Generator

A simple application to generate exploding animated images.

## Quick Start

This application depends on `raylib` and `MagickWand`, and uses `meson` for building, so make sure you have these installed.  
To build and run the application, you can run:

```console
$ meson setup build
$ meson compile -C build
$ ./build/src/explode-generator
```

This will generate the animation at `./neocat_floof_explode.png` as an animated PNG, and then show it on the application's window.
