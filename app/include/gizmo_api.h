#ifndef GIZMO_API_H
#define GIZMO_API_H

#include <SDL.h>

#define GIZMO_TYPE_ID_MAX 64
#define GIZMO_DISPLAY_NAME_MAX 64
#define GIZMO_ACTION_TEXT_MAX 96
#define GIZMO_MAX_ACTIONS_PER_FRAME 64

struct gizmo_instance_t;

typedef enum gizmo_action_kind_t {
    GIZMO_ACTION_NONE = 0,
    GIZMO_ACTION_REPLACE_GIZMO_BY_TYPE = 1,
    GIZMO_ACTION_UPDATE_PANE_TITLE = 2
} gizmo_action_kind_t;

typedef struct gizmo_action_request_t {
    gizmo_action_kind_t kind;
    int pane_id;
    char value[GIZMO_ACTION_TEXT_MAX];
} gizmo_action_request_t;

typedef struct gizmo_action_queue_t {
    gizmo_action_request_t items[GIZMO_MAX_ACTIONS_PER_FRAME];
    int count;
} gizmo_action_queue_t;

typedef struct gizmo_input_state_t {
    int is_focused;
    int mouse_x;
    int mouse_y;
    Uint16 mouse_left_frames;
    Uint16 mouse_right_frames;
    Uint16 mouse_middle_frames;
    int wheel_x;
    int wheel_y;
    const Uint16 *keys_down;
    int key_count;
} gizmo_input_state_t;

typedef struct gizmo_gui_context_t {
    SDL_Renderer *renderer;
    int (*enqueue_request)(void *userdata, const gizmo_action_request_t *request);
    void *enqueue_userdata;
} gizmo_gui_context_t;

typedef struct gizmo_core_context_t {
    void *userdata;
} gizmo_core_context_t;

typedef struct gizmo_api_t {
    const char *type_id;
    const char *display_name;
    int (*init)(struct gizmo_instance_t *gizmo);
    void (*on_resize)(struct gizmo_instance_t *gizmo, int width, int height);
    void (*update_and_render)(struct gizmo_instance_t *gizmo, float dt,
        const gizmo_input_state_t *input);
    void (*shutdown)(struct gizmo_instance_t *gizmo);
    int (*clone_state)(const struct gizmo_instance_t *source, struct gizmo_instance_t *dest);
} gizmo_api_t;

typedef struct gizmo_instance_t {
    const gizmo_api_t *api;
    gizmo_gui_context_t *gui;
    gizmo_core_context_t *core;
    int pane_id;
    SDL_Rect bounds;
    void *gizmo_state;
    char type_id[GIZMO_TYPE_ID_MAX];
    char display_name[GIZMO_DISPLAY_NAME_MAX];
} gizmo_instance_t;

#endif
