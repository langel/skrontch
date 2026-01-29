 #include <SDL.h>
 
 #include "window_manager.h"
 
 int main(int argc, char **argv)
 {
     (void)argc;
     (void)argv;
 
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
         SDL_Log("SDL_Init failed: %s", SDL_GetError());
         return 1;
     }
 
     window_manager_t window_manager;
     if (window_manager_init(&window_manager, "Skrontch", 1200, 800) != SKRONTCH_OK) {
         SDL_Quit();
         return 1;
     }
 
     SDL_Event event;
 
     Uint64 last_ticks = SDL_GetTicks64();
 
    while (window_manager.is_running) {
         while (SDL_PollEvent(&event) != 0) {
             if (event.type == SDL_QUIT) {
                window_manager.is_running = 0;
             }
 
             window_manager_handle_event(&window_manager, &event);
         }
 
         Uint64 current_ticks = SDL_GetTicks64();
         float delta_seconds = (float)(current_ticks - last_ticks) / 1000.0f;
         last_ticks = current_ticks;
 
         window_manager_update(&window_manager, delta_seconds);
         window_manager_render(&window_manager);
     }
 
     window_manager_shutdown(&window_manager);
     SDL_Quit();
 
     return 0;
 }
