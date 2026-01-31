 #ifndef WINDOW_MANAGER_H
 #define WINDOW_MANAGER_H
 
#include <SDL.h>

#include "cursor_manager.h"
#include "skrontch_types.h"
 
#define MAX_SPLIT_NODES 31
#define MAX_TABS 12

 typedef enum {
     SPLIT_ORIENTATION_NONE = 0,
     SPLIT_ORIENTATION_VERTICAL = 1,
     SPLIT_ORIENTATION_HORIZONTAL = 2
 } split_orientation_t;

 typedef enum {
     DROP_ZONE_NONE = 0,
     DROP_ZONE_CENTER = 1,
     DROP_ZONE_LEFT = 2,
     DROP_ZONE_RIGHT = 3,
     DROP_ZONE_TOP = 4,
     DROP_ZONE_BOTTOM = 5
 } drop_zone_t;

 typedef struct split_node_t {
     int is_leaf;
     int pane_id;
     split_orientation_t orientation;
     float ratio;
     int first;
     int second;
     int parent;
 } split_node_t;

typedef struct app_state_t app_state_t;

typedef struct tab_state_t {
    split_node_t nodes[MAX_SPLIT_NODES];
    int node_count;
    int root_node;
    int focused_pane_node;
} tab_state_t;

typedef struct window_state_t {
    app_state_t *app;
     SDL_Window *window;
     SDL_Renderer *renderer;
    Uint32 window_id;
     int width;
     int height;
     int is_dragging_split;
     int is_dragging_header;
     int dragged_pane_node;
     int dragged_split_node;
    int is_creating_pane;
    int pending_pane_id;
     int hover_pane_index;
     int hover_header;
    int hover_header_close;
    int hover_menu_button;
     int hover_split_node;
     drop_zone_t hover_drop_zone;
    int esc_cancel_active;
     int drag_offset_x;
     int drag_offset_y;
     int mouse_x;
     int mouse_y;
    int hover_tab_index;
    int hover_tab_add;
    int is_dragging_tab;
    int dragged_tab_index;
    int dragged_tab_offset_x;
    int hover_tab_insert;
    int hover_header_detach;
    int pending_detach;
    int pending_detach_tab;
    int pending_detach_node;
    int pending_detach_pane_id;
    int should_close;
    tab_state_t tabs[MAX_TABS];
    int tab_count;
    int active_tab;
    cursor_manager_t cursor_manager;
    int is_mouse_pressed;
} window_state_t;
 
skrontch_error_t window_manager_init(window_state_t *window, const char *title, int width, int height,
    int x, int y, int use_position);
void window_manager_shutdown(window_state_t *window);
int window_manager_handle_event(window_state_t *window, const SDL_Event *event);
void window_manager_update(window_state_t *window, float delta_seconds);
void window_manager_render(window_state_t *window);
void window_manager_set_tabs(window_state_t *window, const tab_state_t *tabs, int tab_count,
    int active_tab);
int window_manager_extract_detached_tab(window_state_t *window, tab_state_t *tab_out);
void window_manager_get_window_rect(window_state_t *window, int *x, int *y, int *w, int *h);
void tab_state_init_default(tab_state_t *tab);
 
 #endif
