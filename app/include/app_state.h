#ifndef APP_STATE_H
#define APP_STATE_H

#include "window_manager.h"
#include "workspace_manager.h"

#define MAX_WINDOWS 8

typedef struct app_state_t {
    window_state_t windows[MAX_WINDOWS];
    int window_count;
    int is_running;
    workspace_manager_t workspace;
} app_state_t;

skrontch_error_t app_state_init(app_state_t *app, const char *title, int width, int height);
void app_state_shutdown(app_state_t *app);
int app_state_handle_event(app_state_t *app, const SDL_Event *event);
void app_state_update(app_state_t *app, float delta_seconds);
void app_state_render(app_state_t *app);
int app_state_add_window(app_state_t *app, const char *title, int width, int height, int x, int y,
    int use_position, const tab_state_t *tabs, int tab_count, int active_tab);

#endif
