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

typedef struct log_targets_t {
    FILE *file;
    SDL_LogOutputFunction default_logger;
    void *default_userdata;
} log_targets_t;

static void log_to_file_and_stderr(void *userdata, int category, SDL_LogPriority priority,
    const char *message)
{
    log_targets_t *targets = (log_targets_t *)userdata;
    if (targets != NULL && targets->file != NULL) {
        fprintf(targets->file, "[%s] %s\n", log_priority_label(priority), message);
        fflush(targets->file);
    }
    if (targets != NULL && targets->default_logger != NULL) {
        targets->default_logger(targets->default_userdata, category, priority, message);
    } else {
        fprintf(stderr, "[%s] %s\n", log_priority_label(priority), message);
        fflush(stderr);
    }
}
 
 int main(int argc, char **argv)
 {
     (void)argc;
     (void)argv;
 
	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
         return 1;
     }
 
    FILE *log_file = fopen("skrontch.log", "w");
    log_targets_t log_targets = { 0 };
    if (log_file != NULL) {
        SDL_LogOutputFunction default_logger = NULL;
        void *default_userdata = NULL;
        SDL_LogGetOutputFunction(&default_logger, &default_userdata);
        log_targets.file = log_file;
        log_targets.default_logger = default_logger;
        log_targets.default_userdata = default_userdata;
        SDL_LogSetOutputFunction(log_to_file_and_stderr, &log_targets);
        SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
        SDL_Log("logging enabled");
    }

    app_state_t app_state;
    SDL_Log("initializing app state");
	if (app_state_init(&app_state, "skrontch", 1200, 800) != SKRONTCH_OK) {
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
            if (state_changed && !app_state.suppress_workspace_save) {
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
