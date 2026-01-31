 #include "cursor_manager.h"
 
#include <SDL.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "image_loader.h"
 
 static int build_asset_path(char *buffer, size_t buffer_size, const char *filename)
 {
     char *base_path = SDL_GetBasePath();
     const char *fallback = ".";
    const char *root = base_path != NULL ? base_path : fallback;
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    char root_buffer[512];
    snprintf(root_buffer, sizeof(root_buffer), "%s", root);

    size_t length = strlen(root_buffer);
    while (length > 0 &&
        (root_buffer[length - 1] == '/' || root_buffer[length - 1] == '\\')) {
        root_buffer[length - 1] = '\0';
        length--;
    }

    if (length >= 5) {
        const char *tail = root_buffer + length - 5;
        if (strcmp(tail, "build") == 0) {
            root_buffer[length - 5] = '\0';
            length -= 5;
            while (length > 0 &&
                (root_buffer[length - 1] == '/' || root_buffer[length - 1] == '\\')) {
                root_buffer[length - 1] = '\0';
                length--;
            }
        }
    }

    int wrote = snprintf(buffer, buffer_size, "%s%cassets%c%s",
        root_buffer, separator, separator, filename);
     if (base_path != NULL) {
         SDL_free(base_path);
     }
     return wrote > 0 && (size_t)wrote < buffer_size;
 }
 
static SDL_Surface *load_surface_for_cursor(const char *path)
{
    const char *ext = strrchr(path, '.');
    if (ext == NULL) {
        return image_loader_load_rgba(path);
    }

    if (strcmp(ext, ".png") == 0 || strcmp(ext, ".jpg") == 0 ||
        strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".bmp") == 0) {
        SDL_Log("cursor_manager: loading image %s", path);
        return image_loader_load_rgba(path);
    }

    SDL_Log("cursor_manager: unsupported image extension %s", path);
    return NULL;
}

static int load_cursor_texture(SDL_Renderer *renderer, const char *path, int hot_x, int hot_y,
    custom_cursor_t *cursor)
{
    SDL_Log("cursor_manager: load_cursor_texture %s", path);
    SDL_Surface *surface = load_surface_for_cursor(path);
    if (surface == NULL) {
        SDL_Log("cursor_manager: failed to load %s", path);
        return 0;
    }

    SDL_Log("cursor_manager: CreateTexture %s", path);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL) {
        SDL_Log("cursor_manager: SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_FreeSurface(surface);
        return 0;
    }

    SDL_SetTextureScaleMode(texture, SDL_ScaleModeNearest);

    cursor->texture = texture;
    cursor->width = surface->w;
    cursor->height = surface->h;
    cursor->hot_x = hot_x;
    cursor->hot_y = hot_y;

    SDL_FreeSurface(surface);
    return 1;
}

static int file_exists(const char *path)
{
    SDL_RWops *file = SDL_RWFromFile(path, "rb");
    if (file == NULL) {
        return 0;
    }
    SDL_RWclose(file);
    return 1;
}

 static void destroy_cursor_texture(custom_cursor_t *cursor)
 {
     if (cursor->texture != NULL) {
         SDL_DestroyTexture(cursor->texture);
         cursor->texture = NULL;
     }
     cursor->width = 0;
     cursor->height = 0;
     cursor->hot_x = 0;
     cursor->hot_y = 0;
 }
 
 static custom_cursor_t *select_custom_cursor(cursor_manager_t *manager)
 {
     custom_cursor_t *cursor = &manager->cursor_pointer;
     switch (manager->active_kind) {
         case CURSOR_KIND_HAND_OPEN:
             cursor = &manager->cursor_hand_open;
             break;
         case CURSOR_KIND_HAND_CLOSED:
             cursor = &manager->cursor_hand_closed;
             break;
        case CURSOR_KIND_HAND_ONE_FINGER:
            cursor = manager->cursor_hand_one_finger.texture != NULL
                ? &manager->cursor_hand_one_finger
                : &manager->cursor_hand_open;
            break;
         case CURSOR_KIND_RESIZE_WE:
             cursor = manager->cursor_resize_we.texture != NULL
                 ? &manager->cursor_resize_we
                 : &manager->cursor_pointer;
             break;
         case CURSOR_KIND_RESIZE_NS:
             cursor = manager->cursor_resize_ns.texture != NULL
                 ? &manager->cursor_resize_ns
                 : &manager->cursor_pointer;
             break;
         case CURSOR_KIND_POINTER:
         default:
             cursor = &manager->cursor_pointer;
             break;
     }
 
     if (cursor->texture == NULL) {
         cursor = &manager->cursor_pointer;
     }
 
     return cursor->texture != NULL ? cursor : NULL;
 }
 
 static void update_system_cursor(cursor_manager_t *manager)
 {
     SDL_Cursor *cursor = manager->cursor_arrow;
     if (manager->active_kind == CURSOR_KIND_HAND_OPEN) {
         cursor = manager->cursor_hand;
     } else if (manager->active_kind == CURSOR_KIND_HAND_CLOSED) {
         cursor = manager->cursor_move;
    } else if (manager->active_kind == CURSOR_KIND_HAND_ONE_FINGER) {
        cursor = manager->cursor_hand;
     } else if (manager->active_kind == CURSOR_KIND_RESIZE_WE) {
         cursor = manager->cursor_size_we;
     } else if (manager->active_kind == CURSOR_KIND_RESIZE_NS) {
         cursor = manager->cursor_size_ns;
     }
 
     if (cursor != NULL) {
         SDL_SetCursor(cursor);
     }
 }
 
 void cursor_manager_init(cursor_manager_t *manager, SDL_Renderer *renderer)
 {
     if (manager == NULL) {
         return;
     }
 
     manager->renderer = renderer;
     manager->mouse_inside = 0;
     manager->custom_cursor_enabled = 0;
     manager->active_kind = CURSOR_KIND_POINTER;
     manager->mouse_x = 0;
     manager->mouse_y = 0;
 
     manager->cursor_pointer.texture = NULL;
     manager->cursor_hand_open.texture = NULL;
     manager->cursor_hand_closed.texture = NULL;
    manager->cursor_hand_one_finger.texture = NULL;
     manager->cursor_resize_we.texture = NULL;
     manager->cursor_resize_ns.texture = NULL;
 
    SDL_Log("cursor_manager_init start");
    if (renderer == NULL) {
        SDL_Log("cursor_manager_init: renderer is NULL, skipping custom cursors");
        return;
    }

     manager->cursor_arrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
     manager->cursor_hand = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
     manager->cursor_move = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
     manager->cursor_size_we = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
     manager->cursor_size_ns = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
 
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    char cursor_path[512];
    if (build_asset_path(cursor_path, sizeof(cursor_path), "cursor_pointer.png") &&
        file_exists(cursor_path) &&
        load_cursor_texture(renderer, cursor_path, 0, 0, &manager->cursor_pointer) &&
        build_asset_path(cursor_path, sizeof(cursor_path), "cursor_hand_open.png") &&
        file_exists(cursor_path) &&
        load_cursor_texture(renderer, cursor_path, 6, 2, &manager->cursor_hand_open) &&
        build_asset_path(cursor_path, sizeof(cursor_path), "cursor_hand_closed.png") &&
        file_exists(cursor_path) &&
        load_cursor_texture(renderer, cursor_path, 6, 2, &manager->cursor_hand_closed)) {
        manager->custom_cursor_enabled = 1;
        SDL_ShowCursor(SDL_DISABLE);
    }
    if (build_asset_path(cursor_path, sizeof(cursor_path), "cursor_hand_1_finger.png") &&
        file_exists(cursor_path)) {
        load_cursor_texture(renderer, cursor_path, 6, 2, &manager->cursor_hand_one_finger);
    }

    if (build_asset_path(cursor_path, sizeof(cursor_path), "cursor_resize_we.png") &&
        file_exists(cursor_path)) {
        load_cursor_texture(renderer, cursor_path, 8, 8, &manager->cursor_resize_we);
    }
    if (build_asset_path(cursor_path, sizeof(cursor_path), "cursor_resize_ns.png") &&
        file_exists(cursor_path)) {
        load_cursor_texture(renderer, cursor_path, 8, 8, &manager->cursor_resize_ns);
    }

    update_system_cursor(manager);
    SDL_Log("cursor_manager_init done (custom=%d)", manager->custom_cursor_enabled);
 }
 
 void cursor_manager_shutdown(cursor_manager_t *manager)
 {
     if (manager == NULL) {
         return;
     }
 
     destroy_cursor_texture(&manager->cursor_pointer);
     destroy_cursor_texture(&manager->cursor_hand_open);
     destroy_cursor_texture(&manager->cursor_hand_closed);
    destroy_cursor_texture(&manager->cursor_hand_one_finger);
     destroy_cursor_texture(&manager->cursor_resize_we);
     destroy_cursor_texture(&manager->cursor_resize_ns);
 
     if (manager->cursor_arrow != NULL) {
         SDL_FreeCursor(manager->cursor_arrow);
         manager->cursor_arrow = NULL;
     }
     if (manager->cursor_hand != NULL) {
         SDL_FreeCursor(manager->cursor_hand);
         manager->cursor_hand = NULL;
     }
     if (manager->cursor_move != NULL) {
         SDL_FreeCursor(manager->cursor_move);
         manager->cursor_move = NULL;
     }
     if (manager->cursor_size_we != NULL) {
         SDL_FreeCursor(manager->cursor_size_we);
         manager->cursor_size_we = NULL;
     }
     if (manager->cursor_size_ns != NULL) {
         SDL_FreeCursor(manager->cursor_size_ns);
         manager->cursor_size_ns = NULL;
     }
 
 }
 
 void cursor_manager_set_mouse_inside(cursor_manager_t *manager, int inside)
 {
     if (manager == NULL) {
         return;
     }
 
     manager->mouse_inside = inside;
     if (manager->custom_cursor_enabled) {
         SDL_ShowCursor(inside ? SDL_DISABLE : SDL_ENABLE);
     }
 }
 
 void cursor_manager_set_mouse_position(cursor_manager_t *manager, int x, int y)
 {
     if (manager == NULL) {
         return;
     }
 
     manager->mouse_x = x;
     manager->mouse_y = y;
 }
 
 void cursor_manager_set_active(cursor_manager_t *manager, cursor_kind_t kind)
 {
     if (manager == NULL) {
         return;
     }
 
     manager->active_kind = kind;
     if (!manager->custom_cursor_enabled) {
         update_system_cursor(manager);
     }
 }
 
 void cursor_manager_render(cursor_manager_t *manager)
 {
     if (manager == NULL) {
         return;
     }
 
     if (!manager->custom_cursor_enabled || !manager->mouse_inside) {
         return;
     }
 
     custom_cursor_t *cursor = select_custom_cursor(manager);
     if (cursor == NULL) {
         return;
     }
 
     SDL_Rect dst = {
        manager->mouse_x - cursor->hot_x * 2,
        manager->mouse_y - cursor->hot_y * 2,
        cursor->width * 2,
        cursor->height * 2
     };
     SDL_RenderCopy(manager->renderer, cursor->texture, NULL, &dst);
 }
