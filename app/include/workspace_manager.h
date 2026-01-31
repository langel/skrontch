#ifndef WORKSPACE_MANAGER_H
#define WORKSPACE_MANAGER_H

#include <SDL.h>

#include "window_manager.h"

typedef struct app_state_t app_state_t;

typedef struct workspace_manager_t {
    char workspace_path[512];
    int is_dirty;
    Uint64 last_change_ticks;
    Uint64 debounce_ms;
} workspace_manager_t;

void workspace_manager_init(workspace_manager_t *workspace);
int workspace_manager_load(workspace_manager_t *workspace, app_state_t *app, const char *title,
    int default_width, int default_height);
void workspace_manager_mark_dirty(workspace_manager_t *workspace);
void workspace_manager_update(workspace_manager_t *workspace, const app_state_t *app);

#endif
