#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <SDL.h>

SDL_Surface *image_loader_load_rgba(const char *path);
SDL_Surface *image_loader_load_bmp_rgba(const char *path);

#endif
