 #ifndef PALETTE_MANAGER_H
 #define PALETTE_MANAGER_H
 
 #include <SDL.h>
 
 SDL_Color palette_get_color(const char *name);
 SDL_Color palette_get_color_or(const char *name, SDL_Color fallback);
 
 #endif
