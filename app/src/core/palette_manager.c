 #include "palette_manager.h"
 
 #include <SDL.h>
 #include <string.h>
 
 typedef struct palette_entry_t {
     const char *name;
     SDL_Color color;
 } palette_entry_t;
 
 static const palette_entry_t default_palette[] = {
     { "black",   { 0x22, 0x23, 0x23, 0xFF } },
     { "gray1",   { 0x5F, 0x61, 0x62, 0xFF } },
     { "gray2",   { 0xA4, 0xA9, 0xA5, 0xFF } },
     { "white",   { 0xF0, 0xF6, 0xF0, 0xFF } },
     { "green2",  { 0x15, 0xDE, 0x29, 0xFF } },
     { "cyan",    { 0x11, 0xD7, 0xF2, 0xFF } },
     { "magenta", { 0xFD, 0x1F, 0xF3, 0xFF } },
     { "pink",    { 0xFF, 0x9F, 0xCB, 0xFF } },
     { "green1",  { 0x1B, 0x90, 0x45, 0xFF } },
     { "blue",    { 0x0B, 0x79, 0xEC, 0xFF } },
     { "purple",  { 0xAD, 0x2B, 0xBE, 0xFF } },
     { "red",     { 0xCD, 0x02, 0x33, 0xFF } },
     { "orange",  { 0xE5, 0x80, 0x2D, 0xFF } },
     { "yellow",  { 0xFF, 0xD3, 0x00, 0xFF } },
     { "brown",   { 0x65, 0x38, 0x19, 0xFF } },
     { "tan",     { 0xE0, 0xAC, 0x69, 0xFF } }
 };
 
 SDL_Color palette_get_color(const char *name)
 {
     SDL_Color fallback = { 0, 0, 0, 255 };
     return palette_get_color_or(name, fallback);
 }
 
 SDL_Color palette_get_color_or(const char *name, SDL_Color fallback)
 {
     if (name == NULL) {
         return fallback;
     }
 
     for (size_t i = 0; i < sizeof(default_palette) / sizeof(default_palette[0]); ++i) {
         if (strcmp(default_palette[i].name, name) == 0) {
             return default_palette[i].color;
         }
     }
 
     return fallback;
 }
