#include "window_manager.h"
#include "palette_manager.h"

#include <SDL.h>
 
enum {
    HEADER_HEIGHT = 28,
    MENU_HEIGHT = 26,
    MENU_BUTTON_SIZE = 18,
    MENU_BUTTON_PADDING = 6,
    MENU_BORDER_THICKNESS = 2,
    HEADER_BUTTON_SIZE = 14,
    HEADER_BUTTON_PADDING = 6,
    SPLIT_THICKNESS = 12,
    MIN_PANE_SIZE = 120,
    DROP_ZONE_MARGIN_DIVISOR = 4
};


 static void window_manager_clear(window_manager_t *manager)
 {
     manager->window = NULL;
     manager->renderer = NULL;
     manager->width = 0;
     manager->height = 0;
    manager->node_count = 0;
    manager->root_node = -1;
    manager->is_dragging_split = 0;
    manager->is_dragging_header = 0;
    manager->dragged_pane_node = -1;
    manager->dragged_split_node = -1;
    manager->focused_pane_node = -1;
    manager->is_creating_pane = 0;
    manager->pending_pane_id = 0;
    manager->hover_pane_index = -1;
    manager->hover_header = 0;
    manager->hover_header_close = 0;
    manager->hover_menu_button = 0;
    manager->hover_split_node = -1;
    manager->hover_drop_zone = DROP_ZONE_NONE;
    manager->esc_cancel_active = 0;
    manager->is_running = 1;
    manager->is_mouse_pressed = 0;
    manager->drag_offset_x = 0;
    manager->drag_offset_y = 0;
    manager->mouse_x = 0;
    manager->mouse_y = 0;
    cursor_manager_init(&manager->cursor_manager, NULL);
 }
 
 static void window_manager_set_size(window_manager_t *manager, int width, int height)
 {
     if (manager == NULL) {
         return;
     }
 
     manager->width = width;
     manager->height = height;
 }
 
static int clamp_int(int value, int min_value, int max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static float clamp_float(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static SDL_Rect make_rect(int x, int y, int w, int h)
{
    SDL_Rect rect = { x, y, w, h };
    return rect;
}

static int point_in_rect(int x, int y, const SDL_Rect *rect)
{
    if (rect == NULL) {
        return 0;
    }
    return x >= rect->x &&
           y >= rect->y &&
           x < rect->x + rect->w &&
           y < rect->y + rect->h;
}

static int get_next_pane_id(window_manager_t *manager)
{
    int max_id = 0;
    if (manager == NULL) {
        return 1;
    }

    for (int i = 0; i < manager->node_count; ++i) {
        if (manager->nodes[i].is_leaf && manager->nodes[i].pane_id > max_id) {
            max_id = manager->nodes[i].pane_id;
        }
    }

    return max_id + 1;
}

static SDL_Rect get_menu_bar_rect(window_manager_t *manager)
{
    int width = manager != NULL ? manager->width : 0;
    return make_rect(0, 0, width, MENU_HEIGHT);
}

static SDL_Rect get_menu_close_rect(window_manager_t *manager)
{
    int width = manager != NULL ? manager->width : 0;
    int x = width - MENU_BUTTON_PADDING - MENU_BUTTON_SIZE;
    int y = (MENU_HEIGHT - MENU_BUTTON_SIZE) / 2;
    return make_rect(x, y, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE);
}

static SDL_Rect get_menu_add_rect(window_manager_t *manager)
{
    int width = manager != NULL ? manager->width : 0;
    int x = width - MENU_BUTTON_PADDING * 2 - MENU_BUTTON_SIZE * 2;
    int y = (MENU_HEIGHT - MENU_BUTTON_SIZE) / 2;
    return make_rect(x, y, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE);
}

static SDL_Rect get_header_close_rect(const SDL_Rect *header_rect)
{
    if (header_rect == NULL) {
        return make_rect(0, 0, 0, 0);
    }
    int x = header_rect->x + header_rect->w - HEADER_BUTTON_PADDING - HEADER_BUTTON_SIZE;
    int y = header_rect->y + (HEADER_HEIGHT - HEADER_BUTTON_SIZE) / 2;
    return make_rect(x, y, HEADER_BUTTON_SIZE, HEADER_BUTTON_SIZE);
}

static void draw_border_inside(SDL_Renderer *renderer, const SDL_Rect *rect, int thickness)
{
    for (int i = 0; i < thickness; ++i) {
        SDL_Rect inset = {
            rect->x + i,
            rect->y + i,
            rect->w - i * 2,
            rect->h - i * 2
        };
        if (inset.w > 0 && inset.h > 0) {
            SDL_RenderDrawRect(renderer, &inset);
        }
    }
}

static void draw_border_outside(SDL_Renderer *renderer, const SDL_Rect *rect, int thickness)
{
    for (int i = 1; i <= thickness; ++i) {
        SDL_Rect inset = {
            rect->x - i,
            rect->y - i,
            rect->w + i * 2,
            rect->h + i * 2
        };
        if (inset.w > 0 && inset.h > 0) {
            SDL_RenderDrawRect(renderer, &inset);
        }
    }
}

static void draw_x_icon(SDL_Renderer *renderer, const SDL_Rect *rect, int thickness)
{
    if (renderer == NULL || rect == NULL) {
        return;
    }

    int x0 = rect->x + 2;
    int y0 = rect->y + 2;
    int x1 = rect->x + rect->w - 3;
    int y1 = rect->y + rect->h - 3;
    for (int i = 0; i < thickness; ++i) {
        SDL_RenderDrawLine(renderer, x0, y0 + i, x1, y1 + i);
        SDL_RenderDrawLine(renderer, x0, y1 - i, x1, y0 - i);
    }
}

static void draw_plus_icon(SDL_Renderer *renderer, const SDL_Rect *rect, int thickness)
{
    if (renderer == NULL || rect == NULL) {
        return;
    }

    int mid_x = rect->x + rect->w / 2;
    int mid_y = rect->y + rect->h / 2;
    int half = rect->w < rect->h ? rect->w / 2 : rect->h / 2;
    int length = half - 2;
    for (int i = 0; i < thickness; ++i) {
        SDL_RenderDrawLine(renderer, mid_x - length, mid_y + i, mid_x + length, mid_y + i);
        SDL_RenderDrawLine(renderer, mid_x + i, mid_y - length, mid_x + i, mid_y + length);
    }
}

typedef struct leaf_info_t {
    SDL_Rect rect;
    int node_index;
    int pane_id;
} leaf_info_t;

typedef struct split_bar_t {
    SDL_Rect rect;
    SDL_Rect container;
    int node_index;
    int depth;
    split_orientation_t orientation;
} split_bar_t;

static int split_tree_add_leaf(window_manager_t *manager, int pane_id)
{
    if (manager->node_count >= MAX_SPLIT_NODES) {
        SDL_Log("split_tree_add_leaf: node limit reached");
        return -1;
    }

    int index = manager->node_count++;
    split_node_t *node = &manager->nodes[index];
    node->is_leaf = 1;
    node->pane_id = pane_id;
    node->orientation = SPLIT_ORIENTATION_NONE;
    node->ratio = 0.0f;
    node->first = -1;
    node->second = -1;
    node->parent = -1;
    return index;
}

static int split_tree_add_split(window_manager_t *manager, split_orientation_t orientation,
    float ratio, int first, int second)
{
    if (manager->node_count >= MAX_SPLIT_NODES) {
        SDL_Log("split_tree_add_split: node limit reached");
        return -1;
    }

    int index = manager->node_count++;
    split_node_t *node = &manager->nodes[index];
    node->is_leaf = 0;
    node->pane_id = 0;
    node->orientation = orientation;
    node->ratio = ratio;
    node->first = first;
    node->second = second;
    node->parent = -1;

    if (first >= 0) {
        manager->nodes[first].parent = index;
    }
    if (second >= 0) {
        manager->nodes[second].parent = index;
    }

    return index;
}

static void split_tree_init(window_manager_t *manager)
{
    manager->node_count = 0;

    int left_top = split_tree_add_leaf(manager, 1);
    int left_bottom = split_tree_add_leaf(manager, 3);
    int right_top = split_tree_add_leaf(manager, 2);
    int right_bottom = split_tree_add_leaf(manager, 4);

    int left_split = split_tree_add_split(manager, SPLIT_ORIENTATION_HORIZONTAL, 0.5f,
        left_top, left_bottom);
    int right_split = split_tree_add_split(manager, SPLIT_ORIENTATION_HORIZONTAL, 0.5f,
        right_top, right_bottom);
    int root_split = split_tree_add_split(manager, SPLIT_ORIENTATION_VERTICAL, 0.55f,
        left_split, right_split);

    manager->root_node = root_split;
}

static void split_tree_replace_child(window_manager_t *manager, int parent_index,
    int old_child, int new_child)
{
    split_node_t *parent = &manager->nodes[parent_index];
    if (parent->first == old_child) {
        parent->first = new_child;
    } else if (parent->second == old_child) {
        parent->second = new_child;
    }

    if (new_child >= 0) {
        manager->nodes[new_child].parent = parent_index;
    }
}

static void split_tree_detach_leaf(window_manager_t *manager, int leaf_index)
{
    int parent_index = manager->nodes[leaf_index].parent;
    if (parent_index < 0) {
        manager->root_node = leaf_index;
        return;
    }

    int sibling_index = manager->nodes[parent_index].first == leaf_index
        ? manager->nodes[parent_index].second
        : manager->nodes[parent_index].first;

    int grand_index = manager->nodes[parent_index].parent;
    if (grand_index >= 0) {
        split_tree_replace_child(manager, grand_index, parent_index, sibling_index);
    } else {
        manager->root_node = sibling_index;
        if (sibling_index >= 0) {
            manager->nodes[sibling_index].parent = -1;
        }
    }
}

static int split_tree_insert_new_pane(window_manager_t *manager, int target_node,
    int new_pane_id, drop_zone_t zone, const SDL_Rect *target_rect)
{
    if (manager == NULL || target_node < 0 || zone == DROP_ZONE_NONE) {
        return -1;
    }

    drop_zone_t effective_zone = zone;
    if (zone == DROP_ZONE_CENTER && target_rect != NULL) {
        effective_zone = (target_rect->w >= target_rect->h) ? DROP_ZONE_RIGHT : DROP_ZONE_BOTTOM;
    }

    int existing_leaf = split_tree_add_leaf(manager, manager->nodes[target_node].pane_id);
    int new_leaf = split_tree_add_leaf(manager, new_pane_id);
    if (existing_leaf < 0 || new_leaf < 0) {
        SDL_Log("split_tree_insert_new_pane: node limit reached");
        return -1;
    }

    split_node_t *target = &manager->nodes[target_node];
    target->is_leaf = 0;
    target->pane_id = 0;
    target->ratio = 0.5f;
    target->orientation = (effective_zone == DROP_ZONE_LEFT || effective_zone == DROP_ZONE_RIGHT)
        ? SPLIT_ORIENTATION_VERTICAL
        : SPLIT_ORIENTATION_HORIZONTAL;

    if (effective_zone == DROP_ZONE_LEFT || effective_zone == DROP_ZONE_TOP) {
        target->first = new_leaf;
        target->second = existing_leaf;
    } else {
        target->first = existing_leaf;
        target->second = new_leaf;
    }

    manager->nodes[target->first].parent = target_node;
    manager->nodes[target->second].parent = target_node;
    return new_leaf;
}

static void split_tree_layout_node(window_manager_t *manager, int node_index, SDL_Rect rect,
    int depth, leaf_info_t *leaves, int *leaf_count, split_bar_t *bars, int *bar_count)
{
    if (node_index < 0) {
        return;
    }

    split_node_t *node = &manager->nodes[node_index];
    if (node->is_leaf) {
        leaves[*leaf_count].rect = rect;
        leaves[*leaf_count].node_index = node_index;
        leaves[*leaf_count].pane_id = node->pane_id;
        *leaf_count += 1;
        return;
    }

    if (node->orientation == SPLIT_ORIENTATION_VERTICAL) {
        int split_pos = (int)(node->ratio * (float)rect.w);
        int min_pos = MIN_PANE_SIZE;
        int max_pos = rect.w - MIN_PANE_SIZE;
        if (max_pos < min_pos) {
            split_pos = rect.w / 2;
        } else {
            split_pos = clamp_int(split_pos, min_pos, max_pos);
        }
        node->ratio = rect.w > 0 ? (float)split_pos / (float)rect.w : 0.5f;

        int left_w = split_pos - SPLIT_THICKNESS / 2;
        int right_w = rect.w - split_pos - SPLIT_THICKNESS / 2;
        if (left_w < 0) {
            left_w = 0;
        }
        if (right_w < 0) {
            right_w = 0;
        }

        SDL_Rect left_rect = make_rect(rect.x, rect.y, left_w, rect.h);
        SDL_Rect right_rect = make_rect(rect.x + split_pos + SPLIT_THICKNESS / 2, rect.y,
            right_w, rect.h);
        SDL_Rect bar_rect = make_rect(rect.x + split_pos - SPLIT_THICKNESS / 2, rect.y,
            SPLIT_THICKNESS, rect.h);

        bars[*bar_count].rect = bar_rect;
        bars[*bar_count].container = rect;
        bars[*bar_count].node_index = node_index;
        bars[*bar_count].depth = depth;
        bars[*bar_count].orientation = node->orientation;
        *bar_count += 1;

        split_tree_layout_node(manager, node->first, left_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
        split_tree_layout_node(manager, node->second, right_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
        return;
    }

    if (node->orientation == SPLIT_ORIENTATION_HORIZONTAL) {
        int split_pos = (int)(node->ratio * (float)rect.h);
        int min_pos = MIN_PANE_SIZE;
        int max_pos = rect.h - MIN_PANE_SIZE;
        if (max_pos < min_pos) {
            split_pos = rect.h / 2;
        } else {
            split_pos = clamp_int(split_pos, min_pos, max_pos);
        }
        node->ratio = rect.h > 0 ? (float)split_pos / (float)rect.h : 0.5f;

        int top_h = split_pos - SPLIT_THICKNESS / 2;
        int bottom_h = rect.h - split_pos - SPLIT_THICKNESS / 2;
        if (top_h < 0) {
            top_h = 0;
        }
        if (bottom_h < 0) {
            bottom_h = 0;
        }

        SDL_Rect top_rect = make_rect(rect.x, rect.y, rect.w, top_h);
        SDL_Rect bottom_rect = make_rect(rect.x, rect.y + split_pos + SPLIT_THICKNESS / 2,
            rect.w, bottom_h);
        SDL_Rect bar_rect = make_rect(rect.x, rect.y + split_pos - SPLIT_THICKNESS / 2,
            rect.w, SPLIT_THICKNESS);

        bars[*bar_count].rect = bar_rect;
        bars[*bar_count].container = rect;
        bars[*bar_count].node_index = node_index;
        bars[*bar_count].depth = depth;
        bars[*bar_count].orientation = node->orientation;
        *bar_count += 1;

        split_tree_layout_node(manager, node->first, top_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
        split_tree_layout_node(manager, node->second, bottom_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
    }
}

static int split_tree_layout(window_manager_t *manager, leaf_info_t *leaves, int *leaf_count,
    split_bar_t *bars, int *bar_count)
{
    if (manager->root_node < 0) {
        return 0;
    }

    *leaf_count = 0;
    *bar_count = 0;
    int content_height = manager->height - MENU_HEIGHT - MENU_BORDER_THICKNESS;
    if (content_height < 0) {
        content_height = 0;
    }
    SDL_Rect root_rect = make_rect(0, MENU_HEIGHT + MENU_BORDER_THICKNESS,
        manager->width, content_height);
    split_tree_layout_node(manager, manager->root_node, root_rect, 0,
        leaves, leaf_count, bars, bar_count);
    return 1;
}

static int hit_test_leaf(int x, int y, const leaf_info_t *leaves, int leaf_count)
{
    for (int i = 0; i < leaf_count; ++i) {
        if (point_in_rect(x, y, &leaves[i].rect)) {
            return i;
        }
    }
    return -1;
}

static int hit_test_split_bar(int x, int y, const split_bar_t *bars, int bar_count)
{
    for (int i = 0; i < bar_count; ++i) {
        if (point_in_rect(x, y, &bars[i].rect)) {
            return i;
        }
    }
    return -1;
}

static drop_zone_t compute_drop_zone(const SDL_Rect *rect, int x, int y)
{
    int margin_x = rect->w / DROP_ZONE_MARGIN_DIVISOR;
    int margin_y = rect->h / DROP_ZONE_MARGIN_DIVISOR;

    if (x <= rect->x + margin_x) {
        return DROP_ZONE_LEFT;
    }
    if (x >= rect->x + rect->w - margin_x) {
        return DROP_ZONE_RIGHT;
    }
    if (y <= rect->y + margin_y) {
        return DROP_ZONE_TOP;
    }
    if (y >= rect->y + rect->h - margin_y) {
        return DROP_ZONE_BOTTOM;
    }
    return DROP_ZONE_CENTER;
}

static void sort_leaves_by_position(leaf_info_t *leaves, int leaf_count)
{
    for (int i = 0; i < leaf_count - 1; ++i) {
        for (int j = i + 1; j < leaf_count; ++j) {
            int yi = leaves[i].rect.y;
            int yj = leaves[j].rect.y;
            int xi = leaves[i].rect.x;
            int xj = leaves[j].rect.x;
            if (yj < yi || (yj == yi && xj < xi)) {
                leaf_info_t temp = leaves[i];
                leaves[i] = leaves[j];
                leaves[j] = temp;
            }
        }
    }
}

static void window_manager_update_cursor(window_manager_t *manager);

static void draw_split_bar(SDL_Renderer *renderer, const split_bar_t *bar, int is_hovered)
{
    SDL_Color fill_color = palette_get_color("gray2");
    SDL_Color border_color = palette_get_color("black");

    if (is_hovered) {
        fill_color = palette_get_color("white");
        border_color = palette_get_color("gray2");
    }

    SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
    SDL_RenderFillRect(renderer, &bar->rect);
    SDL_SetRenderDrawColor(renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    if (bar->orientation == SPLIT_ORIENTATION_VERTICAL) {
        SDL_Rect left_edge = make_rect(bar->rect.x, bar->rect.y, 2, bar->rect.h);
        SDL_Rect right_edge = make_rect(
            bar->rect.x + bar->rect.w - 2,
            bar->rect.y,
            2,
            bar->rect.h
        );
        SDL_RenderFillRect(renderer, &left_edge);
        SDL_RenderFillRect(renderer, &right_edge);
    } else if (bar->orientation == SPLIT_ORIENTATION_HORIZONTAL) {
        SDL_Rect top_edge = make_rect(bar->rect.x, bar->rect.y, bar->rect.w, 2);
        SDL_Rect bottom_edge = make_rect(
            bar->rect.x,
            bar->rect.y + bar->rect.h - 2,
            bar->rect.w,
            2
        );
        SDL_RenderFillRect(renderer, &top_edge);
        SDL_RenderFillRect(renderer, &bottom_edge);
    }
}

static void window_manager_handle_mouse_down(window_manager_t *manager, int mouse_x, int mouse_y)
{
    leaf_info_t leaves[MAX_SPLIT_NODES];
    split_bar_t bars[MAX_SPLIT_NODES];
    int leaf_count = 0;
    int bar_count = 0;
    split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);

    manager->mouse_x = mouse_x;
    manager->mouse_y = mouse_y;
    cursor_manager_set_mouse_position(&manager->cursor_manager, mouse_x, mouse_y);

    SDL_Rect menu_rect = get_menu_bar_rect(manager);
    SDL_Rect menu_close = get_menu_close_rect(manager);
    SDL_Rect menu_add = get_menu_add_rect(manager);
    if (point_in_rect(mouse_x, mouse_y, &menu_rect)) {
        if (point_in_rect(mouse_x, mouse_y, &menu_close)) {
            manager->is_running = 0;
        } else if (point_in_rect(mouse_x, mouse_y, &menu_add)) {
            manager->is_creating_pane = 1;
            manager->pending_pane_id = get_next_pane_id(manager);
            manager->esc_cancel_active = 0;
            manager->hover_drop_zone = DROP_ZONE_NONE;
            manager->is_dragging_header = 0;
            manager->is_dragging_split = 0;
            manager->dragged_pane_node = -1;
            manager->dragged_split_node = -1;
        }
        window_manager_update_cursor(manager);
        return;
    }

    if (manager->is_creating_pane) {
        int leaf_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
        if (leaf_index >= 0) {
            drop_zone_t drop_zone = compute_drop_zone(&leaves[leaf_index].rect, mouse_x, mouse_y);
            int new_leaf = split_tree_insert_new_pane(
                manager,
                leaves[leaf_index].node_index,
                manager->pending_pane_id,
                drop_zone,
                &leaves[leaf_index].rect
            );
            if (new_leaf >= 0) {
                manager->focused_pane_node = new_leaf;
                manager->is_creating_pane = 0;
                manager->pending_pane_id = 0;
                manager->hover_drop_zone = DROP_ZONE_NONE;
            }
        }
        window_manager_update_cursor(manager);
        return;
    }

    int bar_index = hit_test_split_bar(mouse_x, mouse_y, bars, bar_count);
    if (bar_index >= 0) {
        manager->is_dragging_split = 1;
        manager->hover_split_node = bars[bar_index].node_index;
        manager->dragged_split_node = bars[bar_index].node_index;
        window_manager_update_cursor(manager);
        return;
    }

    int leaf_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
    if (leaf_index >= 0) {
        manager->focused_pane_node = leaves[leaf_index].node_index;
        SDL_Rect header_rect = make_rect(
            leaves[leaf_index].rect.x,
            leaves[leaf_index].rect.y,
            leaves[leaf_index].rect.w,
            HEADER_HEIGHT
        );
        SDL_Rect close_rect = get_header_close_rect(&header_rect);
        if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2 &&
            point_in_rect(mouse_x, mouse_y, &close_rect)) {
            if (leaf_count > 1) {
                int closed_node = leaves[leaf_index].node_index;
                split_tree_detach_leaf(manager, closed_node);
                split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);
                if (leaf_count > 0) {
                    manager->focused_pane_node = leaves[0].node_index;
                } else {
                    manager->focused_pane_node = -1;
                }
            }
            window_manager_update_cursor(manager);
            return;
        }
        if (point_in_rect(mouse_x, mouse_y, &header_rect)) {
            manager->is_dragging_header = 1;
            manager->dragged_pane_node = leaves[leaf_index].node_index;
            manager->drag_offset_x = mouse_x - leaves[leaf_index].rect.x;
            manager->drag_offset_y = mouse_y - leaves[leaf_index].rect.y;
            manager->hover_header = 1;
            manager->focused_pane_node = manager->dragged_pane_node;
            window_manager_update_cursor(manager);
        }
    }
}


static void window_manager_update_cursor(window_manager_t *manager)
{
    cursor_kind_t kind = CURSOR_KIND_POINTER;

    if (manager->is_creating_pane) {
        kind = CURSOR_KIND_HAND_CLOSED;
    } else if (manager->is_dragging_header) {
        kind = CURSOR_KIND_HAND_CLOSED;
    } else if (manager->is_dragging_split || manager->hover_split_node >= 0) {
        split_orientation_t orientation = manager->nodes[
            manager->is_dragging_split ? manager->dragged_split_node : manager->hover_split_node
        ].orientation;
        kind = (orientation == SPLIT_ORIENTATION_VERTICAL)
            ? CURSOR_KIND_RESIZE_WE
            : CURSOR_KIND_RESIZE_NS;
    } else if (manager->hover_menu_button || manager->hover_header_close) {
        kind = CURSOR_KIND_HAND_ONE_FINGER;
    } else if (manager->hover_header) {
        kind = CURSOR_KIND_HAND_OPEN;
    }

    cursor_manager_set_active(&manager->cursor_manager, kind);
}

 static void draw_segment(SDL_Renderer *renderer, const SDL_Rect *rect)
 {
     SDL_RenderFillRect(renderer, rect);
 }
 
 static void draw_digit(SDL_Renderer *renderer, int digit, const SDL_Rect *bounds)
 {
     static const unsigned char digit_masks[10] = {
         0x3F, /* 0 */
         0x06, /* 1 */
         0x5B, /* 2 */
         0x4F, /* 3 */
         0x66, /* 4 */
         0x6D, /* 5 */
         0x7D, /* 6 */
         0x07, /* 7 */
         0x7F, /* 8 */
         0x6F  /* 9 */
     };
 
     int thickness = bounds->w < bounds->h ? bounds->w / 6 : bounds->h / 6;
     if (thickness < 2) {
         thickness = 2;
     }
 
     int x = bounds->x;
     int y = bounds->y;
     int w = bounds->w;
     int h = bounds->h;
     int long_w = w - thickness * 2;
     int long_h = (h - thickness * 3) / 2;
 
     SDL_Rect segments[7] = {
         { x + thickness, y, long_w, thickness }, /* a */
         { x + w - thickness, y + thickness, thickness, long_h }, /* b */
         { x + w - thickness, y + thickness * 2 + long_h, thickness, long_h }, /* c */
         { x + thickness, y + h - thickness, long_w, thickness }, /* d */
         { x, y + thickness * 2 + long_h, thickness, long_h }, /* e */
         { x, y + thickness, thickness, long_h }, /* f */
         { x + thickness, y + thickness + long_h, long_w, thickness } /* g */
     };
 
     if (digit < 0 || digit > 9) {
         return;
     }
 
     unsigned char mask = digit_masks[digit];
     for (int i = 0; i < 7; ++i) {
         if (mask & (1 << i)) {
             draw_segment(renderer, &segments[i]);
         }
     }
 }
 
 skrontch_error_t window_manager_init(window_manager_t *manager, const char *title, int width, int height)
 {
     if (manager == NULL) {
         return SKRONTCH_ERROR;
     }
 
     window_manager_clear(manager);
    split_tree_init(manager);
 
     manager->window = SDL_CreateWindow(
         title,
         SDL_WINDOWPOS_CENTERED,
         SDL_WINDOWPOS_CENTERED,
         width,
         height,
         SDL_WINDOW_RESIZABLE
     );
 
     if (manager->window == NULL) {
         SDL_Log("window_manager_init: SDL_CreateWindow failed: %s", SDL_GetError());
         window_manager_clear(manager);
         return SKRONTCH_ERROR;
     }
 
     manager->renderer = SDL_CreateRenderer(
         manager->window,
         -1,
         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
     );
 
     if (manager->renderer == NULL) {
         SDL_Log("window_manager_init: SDL_CreateRenderer failed: %s", SDL_GetError());
         SDL_DestroyWindow(manager->window);
         window_manager_clear(manager);
         return SKRONTCH_ERROR;
     }
 
    cursor_manager_init(&manager->cursor_manager, manager->renderer);
    window_manager_update_cursor(manager);

     window_manager_set_size(manager, width, height);
 
     return SKRONTCH_OK;
 }
 
 void window_manager_shutdown(window_manager_t *manager)
 {
     if (manager == NULL) {
         return;
     }
 
     if (manager->renderer != NULL) {
         SDL_DestroyRenderer(manager->renderer);
         manager->renderer = NULL;
     }
 
     if (manager->window != NULL) {
         SDL_DestroyWindow(manager->window);
         manager->window = NULL;
     }

    cursor_manager_shutdown(&manager->cursor_manager);
 }
 
 void window_manager_handle_event(window_manager_t *manager, const SDL_Event *event)
 {
     if (manager == NULL || event == NULL) {
         return;
     }
 
     if (event->type == SDL_WINDOWEVENT) {
         if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
             event->window.event == SDL_WINDOWEVENT_RESIZED) {
             window_manager_set_size(manager, event->window.data1, event->window.data2);
         }
        if (event->window.event == SDL_WINDOWEVENT_ENTER) {
            cursor_manager_set_mouse_inside(&manager->cursor_manager, 1);
        }
        if (event->window.event == SDL_WINDOWEVENT_LEAVE) {
            manager->hover_header = 0;
            manager->hover_header_close = 0;
            manager->hover_menu_button = 0;
            manager->hover_split_node = -1;
            manager->hover_drop_zone = DROP_ZONE_NONE;
            cursor_manager_set_mouse_inside(&manager->cursor_manager, 0);
        }
        if (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
            int mouse_x = 0;
            int mouse_y = 0;
            Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
            if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 && !manager->is_mouse_pressed) {
                manager->is_mouse_pressed = 1;
                window_manager_handle_mouse_down(manager, mouse_x, mouse_y);
            }
        }
     }

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        manager->is_mouse_pressed = 1;
        window_manager_handle_mouse_down(manager, event->button.x, event->button.y);
    }

    if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        manager->is_mouse_pressed = 0;
        if (manager->is_dragging_header) {
            leaf_info_t leaves[MAX_SPLIT_NODES];
            split_bar_t bars[MAX_SPLIT_NODES];
            int leaf_count = 0;
            int bar_count = 0;
            split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);

            if (!manager->esc_cancel_active &&
                manager->hover_pane_index >= 0 &&
                manager->dragged_pane_node >= 0 &&
                manager->hover_pane_index < leaf_count) {
                int target_node = leaves[manager->hover_pane_index].node_index;
                int dragged_node = manager->dragged_pane_node;

                if (manager->hover_drop_zone == DROP_ZONE_CENTER) {
                    int temp_id = manager->nodes[target_node].pane_id;
                    manager->nodes[target_node].pane_id = manager->nodes[dragged_node].pane_id;
                    manager->nodes[dragged_node].pane_id = temp_id;
                    manager->focused_pane_node = target_node;
                } else if (manager->hover_drop_zone != DROP_ZONE_NONE &&
                    target_node != dragged_node) {
                    int dragged_pane_id = manager->nodes[dragged_node].pane_id;
                    int target_pane_id = manager->nodes[target_node].pane_id;

                    split_tree_detach_leaf(manager, dragged_node);

                    int existing_leaf = split_tree_add_leaf(manager, target_pane_id);
                    if (existing_leaf >= 0) {
                        split_node_t *target = &manager->nodes[target_node];
                        target->is_leaf = 0;
                        target->pane_id = 0;
                        target->ratio = 0.5f;
                        target->orientation = (manager->hover_drop_zone == DROP_ZONE_LEFT ||
                                               manager->hover_drop_zone == DROP_ZONE_RIGHT)
                            ? SPLIT_ORIENTATION_VERTICAL
                            : SPLIT_ORIENTATION_HORIZONTAL;

                        manager->nodes[dragged_node].is_leaf = 1;
                        manager->nodes[dragged_node].pane_id = dragged_pane_id;
                        manager->nodes[dragged_node].orientation = SPLIT_ORIENTATION_NONE;
                        manager->nodes[dragged_node].ratio = 0.0f;
                        manager->nodes[dragged_node].first = -1;
                        manager->nodes[dragged_node].second = -1;

                        if (manager->hover_drop_zone == DROP_ZONE_LEFT ||
                            manager->hover_drop_zone == DROP_ZONE_TOP) {
                            target->first = dragged_node;
                            target->second = existing_leaf;
                        } else {
                            target->first = existing_leaf;
                            target->second = dragged_node;
                        }

                        manager->nodes[target->first].parent = target_node;
                        manager->nodes[target->second].parent = target_node;
                    }
                }
            }
        }

        manager->is_dragging_split = 0;
        manager->is_dragging_header = 0;
        manager->dragged_pane_node = -1;
        manager->dragged_split_node = -1;
        manager->hover_drop_zone = DROP_ZONE_NONE;
        window_manager_update_cursor(manager);
    }

    if (event->type == SDL_MOUSEMOTION) {
        int mouse_x = event->motion.x;
        int mouse_y = event->motion.y;
        manager->mouse_x = mouse_x;
        manager->mouse_y = mouse_y;
        cursor_manager_set_mouse_position(&manager->cursor_manager, mouse_x, mouse_y);

        leaf_info_t leaves[MAX_SPLIT_NODES];
        split_bar_t bars[MAX_SPLIT_NODES];
        int leaf_count = 0;
        int bar_count = 0;
        split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);
        manager->hover_pane_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
        manager->hover_split_node = -1;
        manager->hover_header = 0;
        manager->hover_header_close = 0;
        manager->hover_menu_button = 0;
        manager->hover_drop_zone = DROP_ZONE_NONE;

        int bar_index = hit_test_split_bar(mouse_x, mouse_y, bars, bar_count);
        if (bar_index >= 0) {
            manager->hover_split_node = bars[bar_index].node_index;
        }

        SDL_Rect menu_rect = get_menu_bar_rect(manager);
        SDL_Rect menu_close = get_menu_close_rect(manager);
        SDL_Rect menu_add = get_menu_add_rect(manager);
        if (point_in_rect(mouse_x, mouse_y, &menu_rect) &&
            (point_in_rect(mouse_x, mouse_y, &menu_close) ||
             point_in_rect(mouse_x, mouse_y, &menu_add))) {
            manager->hover_menu_button = 1;
        }

        if (manager->hover_pane_index >= 0) {
            SDL_Rect header_rect = make_rect(
                leaves[manager->hover_pane_index].rect.x,
                leaves[manager->hover_pane_index].rect.y,
                leaves[manager->hover_pane_index].rect.w,
                HEADER_HEIGHT
            );
            SDL_Rect close_rect = get_header_close_rect(&header_rect);
            if (point_in_rect(mouse_x, mouse_y, &header_rect)) {
                if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2 &&
                    point_in_rect(mouse_x, mouse_y, &close_rect)) {
                    manager->hover_header_close = 1;
                } else {
                    manager->hover_header = 1;
                }
            }

            if ((manager->is_dragging_header || manager->is_creating_pane) &&
                !manager->esc_cancel_active) {
                manager->hover_drop_zone = compute_drop_zone(
                    &leaves[manager->hover_pane_index].rect,
                    mouse_x,
                    mouse_y
                );
            }
        }

        if (manager->is_dragging_split && manager->dragged_split_node >= 0) {
            for (int i = 0; i < bar_count; ++i) {
                if (bars[i].node_index == manager->dragged_split_node) {
                    split_node_t *node = &manager->nodes[manager->dragged_split_node];
                    if (node->orientation == SPLIT_ORIENTATION_VERTICAL) {
                        float ratio = (float)(mouse_x - bars[i].container.x) /
                            (float)bars[i].container.w;
                        float min_ratio = (float)MIN_PANE_SIZE / (float)bars[i].container.w;
                        float max_ratio = 1.0f - min_ratio;
                        node->ratio = clamp_float(ratio, min_ratio, max_ratio);
                    } else if (node->orientation == SPLIT_ORIENTATION_HORIZONTAL) {
                        float ratio = (float)(mouse_y - bars[i].container.y) /
                            (float)bars[i].container.h;
                        float min_ratio = (float)MIN_PANE_SIZE / (float)bars[i].container.h;
                        float max_ratio = 1.0f - min_ratio;
                        node->ratio = clamp_float(ratio, min_ratio, max_ratio);
                    }
                    break;
                }
            }
        }

        window_manager_update_cursor(manager);
    }

    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            manager->esc_cancel_active = 1;
            manager->hover_drop_zone = DROP_ZONE_NONE;
            manager->is_creating_pane = 0;
            manager->pending_pane_id = 0;
        }
        if (event->key.keysym.sym == SDLK_TAB &&
            (event->key.keysym.mod & KMOD_CTRL) != 0) {
            leaf_info_t leaves[MAX_SPLIT_NODES];
            split_bar_t bars[MAX_SPLIT_NODES];
            int leaf_count = 0;
            int bar_count = 0;
            split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);
            sort_leaves_by_position(leaves, leaf_count);

            if (leaf_count > 0) {
                int current_index = 0;
                for (int i = 0; i < leaf_count; ++i) {
                    if (leaves[i].node_index == manager->focused_pane_node) {
                        current_index = i;
                        break;
                    }
                }

                int direction = (event->key.keysym.mod & KMOD_SHIFT) != 0 ? -1 : 1;
                int next_index = current_index + direction;
                if (next_index < 0) {
                    next_index = leaf_count - 1;
                } else if (next_index >= leaf_count) {
                    next_index = 0;
                }
                manager->focused_pane_node = leaves[next_index].node_index;
            }
        }
        if (event->key.keysym.sym == SDLK_q &&
            (event->key.keysym.mod & KMOD_CTRL) != 0) {
            manager->is_running = 0;
        }
    }

    if (event->type == SDL_KEYUP) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            manager->esc_cancel_active = 0;
        }
    }
 }
 
 void window_manager_update(window_manager_t *manager, float delta_seconds)
 {
     if (manager == NULL) {
         return;
     }
 
     (void)delta_seconds;
 }
 
 void window_manager_render(window_manager_t *manager)
 {
     if (manager == NULL || manager->renderer == NULL) {
         return;
     }
 
     int render_width = 0;
     int render_height = 0;
     if (SDL_GetRendererOutputSize(manager->renderer, &render_width, &render_height) == 0) {
         window_manager_set_size(manager, render_width, render_height);
     }
 
    SDL_Color background = palette_get_color("black");
    SDL_SetRenderDrawColor(manager->renderer, background.r, background.g, background.b, background.a);
     SDL_RenderClear(manager->renderer);
 
    leaf_info_t leaves[MAX_SPLIT_NODES];
    split_bar_t bars[MAX_SPLIT_NODES];
    int leaf_count = 0;
    int bar_count = 0;
    split_tree_layout(manager, leaves, &leaf_count, bars, &bar_count);
    int has_focus_rect = 0;
    SDL_Rect focus_rect = { 0, 0, 0, 0 };
 
    for (int i = 0; i < leaf_count; ++i) {
        int pane_id = leaves[i].pane_id;
        SDL_Color pane_color = palette_get_color("black");
         SDL_SetRenderDrawColor(
             manager->renderer,
            pane_color.r,
            pane_color.g,
            pane_color.b,
            pane_color.a
         );
        SDL_RenderFillRect(manager->renderer, &leaves[i].rect);
 
        SDL_Rect header_rect = make_rect(
            leaves[i].rect.x,
            leaves[i].rect.y,
            leaves[i].rect.w,
            HEADER_HEIGHT
        );
        SDL_Color header_color = palette_get_color("gray1");
        SDL_SetRenderDrawColor(manager->renderer, header_color.r, header_color.g, header_color.b, header_color.a);
        SDL_RenderFillRect(manager->renderer, &header_rect);

        if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2) {
            SDL_Rect close_rect = get_header_close_rect(&header_rect);
            SDL_Color icon_color = palette_get_color("white");
            SDL_SetRenderDrawColor(manager->renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
            draw_x_icon(manager->renderer, &close_rect, 2);
        }

        if (pane_id > 0) {
            int min_dim = leaves[i].rect.w < leaves[i].rect.h ? leaves[i].rect.w : leaves[i].rect.h;
            int digit_size = min_dim / 3;
            if (digit_size < 16) {
                digit_size = 16;
            }

            SDL_Rect digit_bounds = {
                leaves[i].rect.x + (leaves[i].rect.w - digit_size) / 2,
                leaves[i].rect.y + (leaves[i].rect.h - digit_size) / 2,
                digit_size,
                digit_size
            };

            SDL_Color digit_color = palette_get_color("white");
            SDL_SetRenderDrawColor(manager->renderer, digit_color.r, digit_color.g, digit_color.b, digit_color.a);
            draw_digit(manager->renderer, pane_id, &digit_bounds);

            SDL_Rect header_digit = {
                leaves[i].rect.x + 8,
                leaves[i].rect.y + 6,
                14,
                16
            };
            draw_digit(manager->renderer, pane_id, &header_digit);
        }

        if (leaves[i].node_index == manager->focused_pane_node) {
            focus_rect = leaves[i].rect;
            has_focus_rect = 1;
        }
     }
 
    for (int i = 0; i < bar_count; ++i) {
        if (bars[i].node_index == manager->hover_split_node) {
            continue;
        }
        draw_split_bar(manager->renderer, &bars[i], 0);
    }

    if (has_focus_rect) {
        SDL_Color focus_color = palette_get_color("gray1");
        SDL_SetRenderDrawColor(manager->renderer, focus_color.r, focus_color.g, focus_color.b, focus_color.a);
        draw_border_outside(manager->renderer, &focus_rect, 4);
    }

    for (int i = 0; i < bar_count; ++i) {
        if (bars[i].node_index != manager->hover_split_node) {
            continue;
        }
        draw_split_bar(manager->renderer, &bars[i], 1);
    }

    if ((manager->is_dragging_header || manager->is_creating_pane) &&
        !manager->esc_cancel_active &&
        manager->hover_pane_index >= 0 &&
        manager->hover_pane_index < leaf_count &&
        manager->hover_drop_zone != DROP_ZONE_NONE &&
        !(manager->is_dragging_header &&
          leaves[manager->hover_pane_index].node_index == manager->dragged_pane_node)) {
        SDL_Rect target = leaves[manager->hover_pane_index].rect;
        int margin_x = target.w / DROP_ZONE_MARGIN_DIVISOR;
        int margin_y = target.h / DROP_ZONE_MARGIN_DIVISOR;
        SDL_Rect left_zone = make_rect(target.x, target.y, margin_x, target.h);
        SDL_Rect right_zone = make_rect(target.x + target.w - margin_x, target.y, margin_x, target.h);
        SDL_Rect top_zone = make_rect(target.x, target.y, target.w, margin_y);
        SDL_Rect bottom_zone = make_rect(target.x, target.y + target.h - margin_y, target.w, margin_y);
        SDL_Rect center_zone = make_rect(target.x + margin_x, target.y + margin_y,
            target.w - margin_x * 2, target.h - margin_y * 2);

        SDL_Color drop_color = palette_get_color("gray2");
        SDL_SetRenderDrawBlendMode(manager->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(manager->renderer, drop_color.r, drop_color.g, drop_color.b, 40);
        SDL_RenderFillRect(manager->renderer, &left_zone);
        SDL_RenderFillRect(manager->renderer, &right_zone);
        SDL_RenderFillRect(manager->renderer, &top_zone);
        SDL_RenderFillRect(manager->renderer, &bottom_zone);

        if (manager->hover_drop_zone == DROP_ZONE_CENTER) {
            SDL_SetRenderDrawColor(manager->renderer, drop_color.r, drop_color.g, drop_color.b, 60);
            SDL_RenderFillRect(manager->renderer, &center_zone);
            SDL_SetRenderDrawColor(manager->renderer, drop_color.r, drop_color.g, drop_color.b, 200);
            draw_border_inside(manager->renderer, &center_zone, 2);
        } else {
            SDL_Rect preview = target;
            if (manager->hover_drop_zone == DROP_ZONE_LEFT) {
                preview = left_zone;
            } else if (manager->hover_drop_zone == DROP_ZONE_RIGHT) {
                preview = right_zone;
            } else if (manager->hover_drop_zone == DROP_ZONE_TOP) {
                preview = top_zone;
            } else if (manager->hover_drop_zone == DROP_ZONE_BOTTOM) {
                preview = bottom_zone;
            }
            SDL_SetRenderDrawColor(manager->renderer, drop_color.r, drop_color.g, drop_color.b, 200);
            draw_border_inside(manager->renderer, &preview, 2);
        }

        SDL_SetRenderDrawBlendMode(manager->renderer, SDL_BLENDMODE_NONE);
    }

    if (manager->is_dragging_header && manager->dragged_pane_node >= 0) {
        SDL_Rect dragged_pane = { 0, 0, 0, 0 };
        for (int i = 0; i < leaf_count; ++i) {
            if (leaves[i].node_index == manager->dragged_pane_node) {
                dragged_pane = leaves[i].rect;
                break;
            }
        }
        int outline_x = manager->mouse_x - manager->drag_offset_x;
        int outline_y = manager->mouse_y - manager->drag_offset_y;

        SDL_Rect outline_rect = make_rect(outline_x, outline_y, dragged_pane.w, dragged_pane.h);
        SDL_Color drag_outline = palette_get_color("white");
        SDL_SetRenderDrawColor(manager->renderer, drag_outline.r, drag_outline.g, drag_outline.b, drag_outline.a);
        draw_border_inside(manager->renderer, &outline_rect, 2);
    }

    if (manager->is_creating_pane) {
        int preview_w = manager->width / 4;
        int preview_h = manager->height / 4;
        if (preview_w < 40) {
            preview_w = 40;
        }
        if (preview_h < 40) {
            preview_h = 40;
        }
        SDL_Rect preview_rect = make_rect(
            manager->mouse_x - preview_w / 2,
            manager->mouse_y - preview_h / 2,
            preview_w,
            preview_h
        );
        SDL_Color preview_color = palette_get_color("white");
        SDL_SetRenderDrawColor(manager->renderer, preview_color.r, preview_color.g,
            preview_color.b, preview_color.a);
        draw_border_inside(manager->renderer, &preview_rect, 2);
    }

    SDL_Rect menu_rect = get_menu_bar_rect(manager);
    SDL_Color menu_color = palette_get_color("gray1");
    SDL_SetRenderDrawColor(manager->renderer, menu_color.r, menu_color.g, menu_color.b, menu_color.a);
    SDL_RenderFillRect(manager->renderer, &menu_rect);

    SDL_Color border_color = palette_get_color("black");
    SDL_SetRenderDrawColor(manager->renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_Rect menu_border = make_rect(0, MENU_HEIGHT, manager->width, MENU_BORDER_THICKNESS);
    SDL_RenderFillRect(manager->renderer, &menu_border);

    SDL_Color icon_color = palette_get_color("white");
    SDL_SetRenderDrawColor(manager->renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_Rect menu_add = get_menu_add_rect(manager);
    SDL_Rect menu_close = get_menu_close_rect(manager);
    draw_plus_icon(manager->renderer, &menu_add, 2);
    draw_x_icon(manager->renderer, &menu_close, 2);

    cursor_manager_render(&manager->cursor_manager);
 
     SDL_RenderPresent(manager->renderer);
 }
