#include <SDL.h>
#include <stdio.h>

#include "app_state.h"

static const char *log_priority_label(SDL_LogPriority priority)
{
    switch (priority) {
        case SDL_LOG_PRIORITY_VERBOSE:
            return "VERBOSE";
        case SDL_LOG_PRIORITY_DEBUG:
            return "DEBUG";
        case SDL_LOG_PRIORITY_INFO:
            return "INFO";
        case SDL_LOG_PRIORITY_WARN:
            return "WARN";
        case SDL_LOG_PRIORITY_ERROR:
            return "ERROR";
        case SDL_LOG_PRIORITY_CRITICAL:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

static void log_to_file(void *userdata, int category, SDL_LogPriority priority,
    const char *message)
{
    (void)category;
    FILE *file = (FILE *)userdata;
    if (file == NULL) {
        return;
    }
    fprintf(file, "[%s] %s\n", log_priority_label(priority), message);
    fflush(file);
}
 
 int main(int argc, char **argv)
 {
     (void)argc;
     (void)argv;
 
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
         return 1;
     }
 
    FILE *log_file = fopen("skrontch.log", "w");
    if (log_file != NULL) {
        SDL_LogSetOutputFunction(log_to_file, log_file);
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
        SDL_Log("logging enabled");
    }

    app_state_t app_state;
    SDL_Log("initializing app state");
    if (app_state_init(&app_state, "Skrontch", 1200, 800) != SKRONTCH_OK) {
        SDL_Log("app_state_init failed");
        if (log_file != NULL) {
            fclose(log_file);
        }
         SDL_Quit();
         return 1;
     }
 
     SDL_Event event;
 
     Uint64 last_ticks = SDL_GetTicks64();
 
    while (app_state.is_running) {
         while (SDL_PollEvent(&event) != 0) {
            int state_changed = app_state_handle_event(&app_state, &event);
            if (state_changed) {
                workspace_manager_mark_dirty(&app_state.workspace);
            }
         }
 
         Uint64 current_ticks = SDL_GetTicks64();
         float delta_seconds = (float)(current_ticks - last_ticks) / 1000.0f;
         last_ticks = current_ticks;
 
        app_state_update(&app_state, delta_seconds);
        app_state_render(&app_state);
     }
 
    app_state_shutdown(&app_state);
    SDL_Log("shutdown complete");
    if (log_file != NULL) {
        fclose(log_file);
    }
     SDL_Quit();
 
     return 0;
 }
