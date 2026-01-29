 #ifndef CURSOR_MANAGER_H
 #define CURSOR_MANAGER_H
 
 #include <SDL.h>
 
 typedef enum {
     CURSOR_KIND_POINTER = 0,
     CURSOR_KIND_HAND_OPEN = 1,
     CURSOR_KIND_HAND_CLOSED = 2,
    CURSOR_KIND_HAND_ONE_FINGER = 3,
    CURSOR_KIND_RESIZE_WE = 4,
    CURSOR_KIND_RESIZE_NS = 5
 } cursor_kind_t;
 
 typedef struct custom_cursor_t {
     SDL_Texture *texture;
     int width;
     int height;
     int hot_x;
     int hot_y;
 } custom_cursor_t;
 
 typedef struct cursor_manager_t {
     SDL_Renderer *renderer;
     int mouse_inside;
     int custom_cursor_enabled;
     cursor_kind_t active_kind;
     int mouse_x;
     int mouse_y;
     custom_cursor_t cursor_pointer;
     custom_cursor_t cursor_hand_open;
     custom_cursor_t cursor_hand_closed;
    custom_cursor_t cursor_hand_one_finger;
     custom_cursor_t cursor_resize_we;
     custom_cursor_t cursor_resize_ns;
     SDL_Cursor *cursor_arrow;
     SDL_Cursor *cursor_hand;
     SDL_Cursor *cursor_move;
     SDL_Cursor *cursor_size_we;
     SDL_Cursor *cursor_size_ns;
 } cursor_manager_t;
 
 void cursor_manager_init(cursor_manager_t *manager, SDL_Renderer *renderer);
 void cursor_manager_shutdown(cursor_manager_t *manager);
 void cursor_manager_set_mouse_inside(cursor_manager_t *manager, int inside);
 void cursor_manager_set_mouse_position(cursor_manager_t *manager, int x, int y);
 void cursor_manager_set_active(cursor_manager_t *manager, cursor_kind_t kind);
 void cursor_manager_render(cursor_manager_t *manager);
 
 #endif
