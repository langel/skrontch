#include "app_state.h"

#include <SDL.h>

static void app_state_clear(app_state_t *app)
{
    app->window_count = 0;
    app->is_running = 1;
    app->suppress_workspace_save = 0;
}

static void app_state_remove_window(app_state_t *app, int index)
{
    if (index < 0 || index >= app->window_count) {
        return;
    }

    window_manager_shutdown(&app->windows[index]);
    for (int i = index; i < app->window_count - 1; ++i) {
        app->windows[i] = app->windows[i + 1];
        app->windows[i].app = app;
    }
    app->window_count -= 1;
}

static Uint32 app_state_get_event_window_id(const SDL_Event *event)
{
    switch (event->type) {
        case SDL_WINDOWEVENT:
            return event->window.windowID;
        case SDL_MOUSEMOTION:
            return event->motion.windowID;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            return event->button.windowID;
        case SDL_MOUSEWHEEL:
            return event->wheel.windowID;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            return event->key.windowID;
        case SDL_TEXTINPUT:
            return event->text.windowID;
        default:
            break;
    }

    return 0;
}

static int app_state_find_window_index(const app_state_t *app, Uint32 window_id)
{
	if (app == NULL || window_id == 0) {
		return -1;
	}

	for (int i = 0; i < app->window_count; ++i) {
		if (app->windows[i].window_id == window_id) {
			return i;
		}
	}

	return -1;
}

int app_state_add_window(app_state_t *app, const char *title, int width, int height, int x, int y,
    int use_position, const tab_state_t *tabs, int tab_count, int active_tab)
{
    if (app == NULL || app->window_count >= MAX_WINDOWS) {
        return 0;
    }

    window_state_t *window = &app->windows[app->window_count];
    if (window_manager_init(window, title, width, height, x, y, use_position) != SKRONTCH_OK) {
        return 0;
    }
    window->app = app;

    if (tabs != NULL && tab_count > 0) {
        window_manager_set_tabs(window, tabs, tab_count, active_tab);
    }

    app->window_count += 1;
    return 1;
}

skrontch_error_t app_state_init(app_state_t *app, const char *title, int width, int height)
{
    if (app == NULL) {
        return SKRONTCH_ERROR;
    }

    app_state_clear(app);
    workspace_manager_init(&app->workspace);
    if (!workspace_manager_load(&app->workspace, app, title, width, height)) {
		window_manager_reset_pane_counter();
        if (!app_state_add_window(app, title, width, height, 0, 0, 0, NULL, 0, 0)) {
            return SKRONTCH_ERROR;
        }
        workspace_manager_mark_dirty(&app->workspace);
    }

    return SKRONTCH_OK;
}

void app_state_shutdown(app_state_t *app)
{
    if (app == NULL) {
        return;
    }

    for (int i = 0; i < app->window_count; ++i) {
        window_manager_shutdown(&app->windows[i]);
    }
    app->window_count = 0;
}

int app_state_handle_event(app_state_t *app, const SDL_Event *event)
{
    if (app == NULL || event == NULL) {
        return 0;
    }

	if (event->type == SDL_KEYDOWN &&
		(event->key.keysym.mod & KMOD_CTRL) != 0 &&
		(event->key.keysym.mod & KMOD_SHIFT) != 0 &&
		(event->key.keysym.sym == SDLK_LEFT || event->key.keysym.sym == SDLK_RIGHT)) {
		Uint32 window_id = app_state_get_event_window_id(event);
		int current_index = app_state_find_window_index(app, window_id);
		if (current_index >= 0 && app->window_count > 1) {
			int direction = event->key.keysym.sym == SDLK_LEFT ? -1 : 1;
			int next_index = current_index + direction;
			if (next_index < 0) {
				next_index = app->window_count - 1;
			} else if (next_index >= app->window_count) {
				next_index = 0;
			}
			SDL_Window *target = app->windows[next_index].window;
			if (target != NULL) {
				Uint32 flags = SDL_GetWindowFlags(target);
				if ((flags & SDL_WINDOW_MINIMIZED) != 0) {
					SDL_RestoreWindow(target);
				}
				SDL_RaiseWindow(target);
				SDL_SetWindowInputFocus(target);
				window_manager_sync_mouse_inside(&app->windows[next_index]);
			}
			app->suppress_workspace_save = 1;
			return 1;
		}
	}

    if (event->type == SDL_KEYDOWN &&
        event->key.keysym.sym == SDLK_q &&
        (event->key.keysym.mod & KMOD_CTRL) != 0) {
        app->is_running = 0;
        return 1;
    }

    if (event->type == SDL_QUIT) {
        app->is_running = 0;
        return 1;
    }

    Uint32 window_id = app_state_get_event_window_id(event);
    if (window_id == 0) {
        return 0;
    }

    for (int i = 0; i < app->window_count; ++i) {
        if (app->windows[i].window_id != window_id) {
            continue;
        }

        if (event->type == SDL_WINDOWEVENT &&
            event->window.event == SDL_WINDOWEVENT_CLOSE) {
            app->windows[i].should_close = 1;
            return 1;
        }

        return window_manager_handle_event(&app->windows[i], event);
    }

    return 0;
}

void app_state_update(app_state_t *app, float delta_seconds)
{
    if (app == NULL) {
        return;
    }

    int state_changed = 0;
    for (int i = 0; i < app->window_count; ++i) {
        window_manager_update(&app->windows[i], delta_seconds);

        tab_state_t detached_tab;
		if (window_manager_extract_detached_tab(&app->windows[i], &detached_tab)) {
			int win_x = 0;
			int win_y = 0;
			SDL_GetWindowPosition(app->windows[i].window, &win_x, &win_y);
			if (app_state_add_window(app, "skrontch", app->windows[i].width,
				app->windows[i].height, win_x, win_y, 1, &detached_tab, 1, 0)) {
                state_changed = 1;
            }
        }
    }

    for (int i = 0; i < app->window_count; ) {
        if (app->windows[i].should_close) {
            app_state_remove_window(app, i);
            state_changed = 1;
            continue;
        }
        i += 1;
    }

    if (app->window_count == 0) {
        app->is_running = 0;
    }

    if (state_changed) {
        workspace_manager_mark_dirty(&app->workspace);
    }

    workspace_manager_update(&app->workspace, app);
    app->suppress_workspace_save = 0;
}

void app_state_render(app_state_t *app)
{
    if (app == NULL) {
        return;
    }

    for (int i = 0; i < app->window_count; ++i) {
        window_manager_render(&app->windows[i]);
    }
}
