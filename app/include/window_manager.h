 #ifndef WINDOW_MANAGER_H
 #define WINDOW_MANAGER_H
 
#include <SDL.h>

#include "cursor_manager.h"
#include "skrontch_types.h"
 
 #define MAX_SPLIT_NODES 31

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

 typedef struct window_manager_t {
     SDL_Window *window;
     SDL_Renderer *renderer;
     int width;
     int height;
     split_node_t nodes[MAX_SPLIT_NODES];
     int node_count;
     int root_node;
     int is_dragging_split;
     int is_dragging_header;
     int dragged_pane_node;
     int dragged_split_node;
    int focused_pane_node;
     int hover_pane_index;
     int hover_header;
     int hover_split_node;
     drop_zone_t hover_drop_zone;
    int esc_cancel_active;
    int is_running;
     int drag_offset_x;
     int drag_offset_y;
     int mouse_x;
     int mouse_y;
    cursor_manager_t cursor_manager;
    int is_mouse_pressed;
 } window_manager_t;
 
 skrontch_error_t window_manager_init(window_manager_t *manager, const char *title, int width, int height);
 void window_manager_shutdown(window_manager_t *manager);
 void window_manager_handle_event(window_manager_t *manager, const SDL_Event *event);
 void window_manager_update(window_manager_t *manager, float delta_seconds);
 void window_manager_render(window_manager_t *manager);
 
 #endif
