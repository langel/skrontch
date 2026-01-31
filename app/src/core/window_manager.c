#include "window_manager.h"
#include "palette_manager.h"

#include <SDL.h>
#include <string.h>
 
enum {
    HEADER_HEIGHT = 28,
    MENU_HEIGHT = 26,
    MENU_BUTTON_SIZE = 18,
    MENU_BUTTON_PADDING = 6,
    MENU_BORDER_THICKNESS = 2,
    HEADER_BUTTON_SIZE = 14,
    HEADER_BUTTON_PADDING = 6,
    HEADER_DETACH_SIZE = 14,
    HEADER_DETACH_PADDING = 6,
    TAB_HEIGHT = 20,
    TAB_WIDTH = 140,
    TAB_PADDING = 6,
    TAB_SPACING = 4,
    TAB_ADD_SIZE = 16,
    SPLIT_THICKNESS = 12,
    MIN_PANE_SIZE = 120,
    DROP_ZONE_MARGIN_DIVISOR = 4
};


static void window_manager_clear(window_state_t *window)
 {
    window->window = NULL;
    window->renderer = NULL;
    window->window_id = 0;
    window->width = 0;
    window->height = 0;
    window->is_dragging_split = 0;
    window->is_dragging_header = 0;
    window->dragged_pane_node = -1;
    window->dragged_split_node = -1;
    window->is_creating_pane = 0;
    window->pending_pane_id = 0;
    window->hover_pane_index = -1;
    window->hover_header = 0;
    window->hover_header_close = 0;
    window->hover_menu_button = 0;
    window->hover_split_node = -1;
    window->hover_drop_zone = DROP_ZONE_NONE;
    window->esc_cancel_active = 0;
    window->is_mouse_pressed = 0;
    window->drag_offset_x = 0;
    window->drag_offset_y = 0;
    window->mouse_x = 0;
    window->mouse_y = 0;
    window->hover_tab_index = -1;
    window->hover_tab_add = 0;
    window->is_dragging_tab = 0;
    window->dragged_tab_index = -1;
    window->dragged_tab_offset_x = 0;
    window->hover_tab_insert = -1;
    window->hover_header_detach = 0;
    window->pending_detach = 0;
    window->pending_detach_tab = -1;
    window->pending_detach_node = -1;
    window->pending_detach_pane_id = 0;
    window->should_close = 0;
    window->tab_count = 0;
    window->active_tab = 0;
    memset(&window->cursor_manager, 0, sizeof(window->cursor_manager));
 }
 
static void window_manager_set_size(window_state_t *window, int width, int height)
 {
    if (window == NULL) {
         return;
     }
 
    window->width = width;
    window->height = height;
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

static int get_next_pane_id(tab_state_t *tab)
{
    int max_id = 0;
    if (tab == NULL) {
        return 1;
    }

    for (int i = 0; i < tab->node_count; ++i) {
        if (tab->nodes[i].is_leaf && tab->nodes[i].pane_id > max_id) {
            max_id = tab->nodes[i].pane_id;
        }
    }

    return max_id + 1;
}

static SDL_Rect get_menu_bar_rect(window_state_t *window)
{
    int width = window != NULL ? window->width : 0;
    return make_rect(0, 0, width, MENU_HEIGHT);
}

static SDL_Rect get_menu_close_rect(window_state_t *window)
{
    int width = window != NULL ? window->width : 0;
    int x = width - MENU_BUTTON_PADDING - MENU_BUTTON_SIZE;
    int y = (MENU_HEIGHT - MENU_BUTTON_SIZE) / 2;
    return make_rect(x, y, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE);
}

static SDL_Rect get_menu_add_rect(window_state_t *window)
{
    int width = window != NULL ? window->width : 0;
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

static SDL_Rect get_header_detach_rect(const SDL_Rect *header_rect)
{
    if (header_rect == NULL) {
        return make_rect(0, 0, 0, 0);
    }
    int x = header_rect->x + header_rect->w - HEADER_BUTTON_PADDING - HEADER_BUTTON_SIZE -
        HEADER_DETACH_PADDING - HEADER_DETACH_SIZE;
    int y = header_rect->y + (HEADER_HEIGHT - HEADER_DETACH_SIZE) / 2;
    return make_rect(x, y, HEADER_DETACH_SIZE, HEADER_DETACH_SIZE);
}

static SDL_Rect get_tab_bar_rect(window_state_t *window)
{
    int width = window != NULL ? window->width : 0;
    int right_reserved = MENU_BUTTON_PADDING * 2 + MENU_BUTTON_SIZE * 2;
    int x = TAB_PADDING;
    int w = width - right_reserved - TAB_PADDING * 2;
    if (w < 0) {
        w = 0;
    }
    return make_rect(x, 0, w, MENU_HEIGHT);
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

static void draw_up_icon(SDL_Renderer *renderer, const SDL_Rect *rect, int thickness)
{
    if (renderer == NULL || rect == NULL) {
        return;
    }

    int mid_x = rect->x + rect->w / 2;
    int top_y = rect->y + 3;
    int bottom_y = rect->y + rect->h - 3;
    for (int i = 0; i < thickness; ++i) {
        SDL_RenderDrawLine(renderer, mid_x + i, bottom_y, mid_x + i, top_y);
        SDL_RenderDrawLine(renderer, mid_x + i, top_y, rect->x + 3, rect->y + rect->h / 2);
        SDL_RenderDrawLine(renderer, mid_x + i, top_y, rect->x + rect->w - 3, rect->y + rect->h / 2);
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

typedef struct tab_info_t {
    SDL_Rect rect;
    int tab_index;
} tab_info_t;

static int tab_state_is_valid(const tab_state_t *tab)
{
    if (tab == NULL || tab->node_count <= 0 || tab->node_count > MAX_SPLIT_NODES) {
        return 0;
    }
    if (tab->root_node < 0 || tab->root_node >= tab->node_count) {
        return 0;
    }
    for (int i = 0; i < tab->node_count; ++i) {
        const split_node_t *node = &tab->nodes[i];
        if (node->first < -1 || node->first >= tab->node_count) {
            return 0;
        }
        if (node->second < -1 || node->second >= tab->node_count) {
            return 0;
        }
        if (node->parent < -1 || node->parent >= tab->node_count) {
            return 0;
        }
    }
    return 1;
}

static int split_tree_add_leaf(tab_state_t *tab, int pane_id)
{
    if (tab->node_count >= MAX_SPLIT_NODES) {
        SDL_Log("split_tree_add_leaf: node limit reached");
        return -1;
    }

    int index = tab->node_count++;
    split_node_t *node = &tab->nodes[index];
    node->is_leaf = 1;
    node->pane_id = pane_id;
    node->orientation = SPLIT_ORIENTATION_NONE;
    node->ratio = 0.0f;
    node->first = -1;
    node->second = -1;
    node->parent = -1;
    return index;
}

static int split_tree_add_split(tab_state_t *tab, split_orientation_t orientation,
    float ratio, int first, int second)
{
    if (tab->node_count >= MAX_SPLIT_NODES) {
        SDL_Log("split_tree_add_split: node limit reached");
        return -1;
    }

    int index = tab->node_count++;
    split_node_t *node = &tab->nodes[index];
    node->is_leaf = 0;
    node->pane_id = 0;
    node->orientation = orientation;
    node->ratio = ratio;
    node->first = first;
    node->second = second;
    node->parent = -1;

    if (first >= 0) {
        tab->nodes[first].parent = index;
    }
    if (second >= 0) {
        tab->nodes[second].parent = index;
    }

    return index;
}

void tab_state_init_default(tab_state_t *tab)
{
    if (tab == NULL) {
        return;
    }

    tab->node_count = 0;

    int left_top = split_tree_add_leaf(tab, 1);
    int left_bottom = split_tree_add_leaf(tab, 3);
    int right_top = split_tree_add_leaf(tab, 2);
    int right_bottom = split_tree_add_leaf(tab, 4);

    int left_split = split_tree_add_split(tab, SPLIT_ORIENTATION_HORIZONTAL, 0.5f,
        left_top, left_bottom);
    int right_split = split_tree_add_split(tab, SPLIT_ORIENTATION_HORIZONTAL, 0.5f,
        right_top, right_bottom);
    int root_split = split_tree_add_split(tab, SPLIT_ORIENTATION_VERTICAL, 0.55f,
        left_split, right_split);

    tab->root_node = root_split;
    tab->focused_pane_node = left_top;
}

static void split_tree_replace_child(tab_state_t *tab, int parent_index,
    int old_child, int new_child)
{
    split_node_t *parent = &tab->nodes[parent_index];
    if (parent->first == old_child) {
        parent->first = new_child;
    } else if (parent->second == old_child) {
        parent->second = new_child;
    }

    if (new_child >= 0) {
        tab->nodes[new_child].parent = parent_index;
    }
}

static void split_tree_detach_leaf(tab_state_t *tab, int leaf_index)
{
    int parent_index = tab->nodes[leaf_index].parent;
    if (parent_index < 0) {
        tab->root_node = leaf_index;
        return;
    }

    int sibling_index = tab->nodes[parent_index].first == leaf_index
        ? tab->nodes[parent_index].second
        : tab->nodes[parent_index].first;

    int grand_index = tab->nodes[parent_index].parent;
    if (grand_index >= 0) {
        split_tree_replace_child(tab, grand_index, parent_index, sibling_index);
    } else {
        tab->root_node = sibling_index;
        if (sibling_index >= 0) {
            tab->nodes[sibling_index].parent = -1;
        }
    }
}

static int split_tree_insert_new_pane(tab_state_t *tab, int target_node,
    int new_pane_id, drop_zone_t zone, const SDL_Rect *target_rect)
{
    if (tab == NULL || target_node < 0 || zone == DROP_ZONE_NONE) {
        return -1;
    }

    drop_zone_t effective_zone = zone;
    if (zone == DROP_ZONE_CENTER && target_rect != NULL) {
        effective_zone = (target_rect->w >= target_rect->h) ? DROP_ZONE_RIGHT : DROP_ZONE_BOTTOM;
    }

    int existing_leaf = split_tree_add_leaf(tab, tab->nodes[target_node].pane_id);
    int new_leaf = split_tree_add_leaf(tab, new_pane_id);
    if (existing_leaf < 0 || new_leaf < 0) {
        SDL_Log("split_tree_insert_new_pane: node limit reached");
        return -1;
    }

    split_node_t *target = &tab->nodes[target_node];
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

    tab->nodes[target->first].parent = target_node;
    tab->nodes[target->second].parent = target_node;
    return new_leaf;
}

static void split_tree_layout_node(tab_state_t *tab, int node_index, SDL_Rect rect,
    int depth, leaf_info_t *leaves, int *leaf_count, split_bar_t *bars, int *bar_count)
{
    if (tab == NULL || node_index < 0 || node_index >= tab->node_count) {
        return;
    }

    split_node_t *node = &tab->nodes[node_index];
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

        split_tree_layout_node(tab, node->first, left_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
        split_tree_layout_node(tab, node->second, right_rect, depth + 1,
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

        split_tree_layout_node(tab, node->first, top_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
        split_tree_layout_node(tab, node->second, bottom_rect, depth + 1,
            leaves, leaf_count, bars, bar_count);
    }
}

static int split_tree_layout(window_state_t *window, tab_state_t *tab, leaf_info_t *leaves,
    int *leaf_count, split_bar_t *bars, int *bar_count)
{
    if (window == NULL || tab == NULL || tab->root_node < 0 || tab->node_count <= 0) {
        return 0;
    }

    *leaf_count = 0;
    *bar_count = 0;
    int content_height = window->height - MENU_HEIGHT - MENU_BORDER_THICKNESS;
    if (content_height < 0) {
        content_height = 0;
    }
    SDL_Rect root_rect = make_rect(0, MENU_HEIGHT + MENU_BORDER_THICKNESS,
        window->width, content_height);
    split_tree_layout_node(tab, tab->root_node, root_rect, 0,
        leaves, leaf_count, bars, bar_count);
    return 1;
}

static void tab_state_init_single(tab_state_t *tab, int pane_id)
{
    if (tab == NULL) {
        return;
    }
    tab->node_count = 0;
    int leaf = split_tree_add_leaf(tab, pane_id);
    tab->root_node = leaf;
    tab->focused_pane_node = leaf;
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

static tab_state_t *window_get_active_tab(window_state_t *window)
{
    if (window == NULL || window->tab_count <= 0) {
        return NULL;
    }
    int index = window->active_tab;
    if (index < 0 || index >= window->tab_count) {
        index = 0;
    }
    if (!tab_state_is_valid(&window->tabs[index])) {
        tab_state_init_default(&window->tabs[index]);
    }
    return &window->tabs[index];
}

static int build_tab_layout(window_state_t *window, tab_info_t *tabs, int *tab_count,
    SDL_Rect *add_rect)
{
    if (window == NULL || tab_count == NULL) {
        return 0;
    }

    SDL_Rect bar = get_tab_bar_rect(window);
    int count = 0;
    int x = bar.x;
    int y = (MENU_HEIGHT - TAB_HEIGHT) / 2;

    for (int i = 0; i < window->tab_count; ++i) {
        if (x + TAB_WIDTH > bar.x + bar.w) {
            break;
        }
        if (tabs != NULL) {
            tabs[count].rect = make_rect(x, y, TAB_WIDTH, TAB_HEIGHT);
            tabs[count].tab_index = i;
        }
        x += TAB_WIDTH + TAB_SPACING;
        count += 1;
    }

    if (add_rect != NULL) {
        int add_x = x;
        int add_y = (MENU_HEIGHT - TAB_ADD_SIZE) / 2;
        if (add_x + TAB_ADD_SIZE > bar.x + bar.w) {
            add_x = bar.x + bar.w - TAB_ADD_SIZE;
        }
        if (add_x < bar.x) {
            add_x = bar.x;
        }
        *add_rect = make_rect(add_x, add_y, TAB_ADD_SIZE, TAB_ADD_SIZE);
    }

    *tab_count = count;
    return 1;
}

static int hit_test_tabs(int x, int y, const tab_info_t *tabs, int tab_count)
{
    for (int i = 0; i < tab_count; ++i) {
        if (point_in_rect(x, y, &tabs[i].rect)) {
            return i;
        }
    }
    return -1;
}

static int compute_tab_insert_index(int x, const tab_info_t *tabs, int tab_count)
{
    if (tab_count == 0) {
        return 0;
    }

    for (int i = 0; i < tab_count; ++i) {
        int mid = tabs[i].rect.x + tabs[i].rect.w / 2;
        if (x < mid) {
            return i;
        }
    }
    return tab_count;
}

static void window_manager_update_cursor(window_state_t *window);

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

static int window_manager_handle_mouse_down(window_state_t *window, int mouse_x, int mouse_y)
{
    tab_state_t *tab = window_get_active_tab(window);
    if (tab == NULL) {
        return 0;
    }

    leaf_info_t leaves[MAX_SPLIT_NODES];
    split_bar_t bars[MAX_SPLIT_NODES];
    int leaf_count = 0;
    int bar_count = 0;
    split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);

    window->mouse_x = mouse_x;
    window->mouse_y = mouse_y;
    cursor_manager_set_mouse_position(&window->cursor_manager, mouse_x, mouse_y);

    SDL_Rect menu_rect = get_menu_bar_rect(window);
    SDL_Rect menu_close = get_menu_close_rect(window);
    SDL_Rect menu_add = get_menu_add_rect(window);
    tab_info_t tabs[MAX_TABS];
    int tab_count = 0;
    SDL_Rect tab_add = make_rect(0, 0, 0, 0);
    build_tab_layout(window, tabs, &tab_count, &tab_add);

    if (point_in_rect(mouse_x, mouse_y, &menu_rect)) {
        int hit_tab = hit_test_tabs(mouse_x, mouse_y, tabs, tab_count);
        if (hit_tab >= 0) {
            int tab_index = tabs[hit_tab].tab_index;
            if (window->active_tab != tab_index) {
                window->active_tab = tab_index;
            }
            window->is_dragging_tab = 1;
            window->dragged_tab_index = tab_index;
            window->dragged_tab_offset_x = mouse_x - tabs[hit_tab].rect.x;
            window_manager_update_cursor(window);
            return 1;
        }
        if (point_in_rect(mouse_x, mouse_y, &tab_add)) {
            if (window->tab_count < MAX_TABS) {
                tab_state_init_default(&window->tabs[window->tab_count]);
                window->active_tab = window->tab_count;
                window->tab_count += 1;
                window_manager_update_cursor(window);
                return 1;
            }
            window_manager_update_cursor(window);
            return 0;
        }

        if (point_in_rect(mouse_x, mouse_y, &menu_close)) {
            window->should_close = 1;
            return 1;
        }
        if (point_in_rect(mouse_x, mouse_y, &menu_add)) {
            window->is_creating_pane = 1;
            window->pending_pane_id = get_next_pane_id(tab);
            window->esc_cancel_active = 0;
            window->hover_drop_zone = DROP_ZONE_NONE;
            window->is_dragging_header = 0;
            window->is_dragging_split = 0;
            window->dragged_pane_node = -1;
            window->dragged_split_node = -1;
        }
        window_manager_update_cursor(window);
        return 0;
    }

    if (window->is_creating_pane) {
        int leaf_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
        if (leaf_index >= 0) {
            drop_zone_t drop_zone = compute_drop_zone(&leaves[leaf_index].rect, mouse_x, mouse_y);
            int new_leaf = split_tree_insert_new_pane(
                tab,
                leaves[leaf_index].node_index,
                window->pending_pane_id,
                drop_zone,
                &leaves[leaf_index].rect
            );
            if (new_leaf >= 0) {
                tab->focused_pane_node = new_leaf;
                window->is_creating_pane = 0;
                window->pending_pane_id = 0;
                window->hover_drop_zone = DROP_ZONE_NONE;
                window_manager_update_cursor(window);
                return 1;
            }
        }
        window_manager_update_cursor(window);
        return 0;
    }

    int bar_index = hit_test_split_bar(mouse_x, mouse_y, bars, bar_count);
    if (bar_index >= 0) {
        window->is_dragging_split = 1;
        window->hover_split_node = bars[bar_index].node_index;
        window->dragged_split_node = bars[bar_index].node_index;
        window_manager_update_cursor(window);
        return 0;
    }

    int leaf_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
    if (leaf_index >= 0) {
        tab->focused_pane_node = leaves[leaf_index].node_index;
        SDL_Rect header_rect = make_rect(
            leaves[leaf_index].rect.x,
            leaves[leaf_index].rect.y,
            leaves[leaf_index].rect.w,
            HEADER_HEIGHT
        );
        SDL_Rect close_rect = get_header_close_rect(&header_rect);
        SDL_Rect detach_rect = get_header_detach_rect(&header_rect);
        if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2 &&
            point_in_rect(mouse_x, mouse_y, &close_rect)) {
            if (leaf_count > 1) {
                int closed_node = leaves[leaf_index].node_index;
                split_tree_detach_leaf(tab, closed_node);
                split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);
                if (leaf_count > 0) {
                    tab->focused_pane_node = leaves[0].node_index;
                } else {
                    tab->focused_pane_node = -1;
                }
                window_manager_update_cursor(window);
                return 1;
            }
            window_manager_update_cursor(window);
            return 0;
        }
        if (header_rect.w >= HEADER_DETACH_SIZE + HEADER_DETACH_PADDING * 2 &&
            point_in_rect(mouse_x, mouse_y, &detach_rect)) {
            window->pending_detach = 1;
            window->pending_detach_tab = window->active_tab;
            window->pending_detach_node = leaves[leaf_index].node_index;
            window->pending_detach_pane_id = leaves[leaf_index].pane_id;
            window_manager_update_cursor(window);
            return 1;
        }
        if (point_in_rect(mouse_x, mouse_y, &header_rect)) {
            window->is_dragging_header = 1;
            window->dragged_pane_node = leaves[leaf_index].node_index;
            window->drag_offset_x = mouse_x - leaves[leaf_index].rect.x;
            window->drag_offset_y = mouse_y - leaves[leaf_index].rect.y;
            window->hover_header = 1;
            tab->focused_pane_node = window->dragged_pane_node;
            window_manager_update_cursor(window);
            return 1;
        }
    }

    return 0;
}


static void window_manager_update_cursor(window_state_t *window)
{
    cursor_kind_t kind = CURSOR_KIND_POINTER;

    if (window->is_creating_pane) {
        kind = CURSOR_KIND_HAND_CLOSED;
    } else if (window->is_dragging_header) {
        kind = CURSOR_KIND_HAND_CLOSED;
    } else if (window->is_dragging_split || window->hover_split_node >= 0) {
        tab_state_t *tab = window_get_active_tab(window);
        int split_index = window->is_dragging_split ? window->dragged_split_node
            : window->hover_split_node;
        split_orientation_t orientation = SPLIT_ORIENTATION_NONE;
        if (tab != NULL && split_index >= 0 && split_index < tab->node_count) {
            orientation = tab->nodes[split_index].orientation;
        }
        kind = (orientation == SPLIT_ORIENTATION_VERTICAL)
            ? CURSOR_KIND_RESIZE_WE
            : CURSOR_KIND_RESIZE_NS;
    } else if (window->hover_menu_button || window->hover_header_close ||
        window->hover_header_detach || window->hover_tab_add || window->hover_tab_index >= 0) {
        kind = CURSOR_KIND_HAND_ONE_FINGER;
    } else if (window->hover_header) {
        kind = CURSOR_KIND_HAND_OPEN;
    }

    cursor_manager_set_active(&window->cursor_manager, kind);
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
 
skrontch_error_t window_manager_init(window_state_t *window, const char *title, int width, int height,
    int x, int y, int use_position)
 {
    if (window == NULL) {
         return SKRONTCH_ERROR;
     }
 
    window_manager_clear(window);
 
    int window_x = use_position ? x : (int)SDL_WINDOWPOS_CENTERED;
    int window_y = use_position ? y : (int)SDL_WINDOWPOS_CENTERED;
    window->window = SDL_CreateWindow(
         title,
        window_x,
        window_y,
         width,
         height,
         SDL_WINDOW_RESIZABLE
     );
 
    if (window->window == NULL) {
         SDL_Log("window_manager_init: SDL_CreateWindow failed: %s", SDL_GetError());
        window_manager_clear(window);
         return SKRONTCH_ERROR;
     }
 
    window->renderer = SDL_CreateRenderer(
        window->window,
         -1,
         SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
     );
 
    if (window->renderer == NULL) {
         SDL_Log("window_manager_init: SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window->window);
        window_manager_clear(window);
         return SKRONTCH_ERROR;
     }
 
    window->window_id = SDL_GetWindowID(window->window);
    cursor_manager_init(&window->cursor_manager, window->renderer);
    window_manager_update_cursor(window);

    window_manager_set_size(window, width, height);
    tab_state_init_default(&window->tabs[0]);
    window->tab_count = 1;
    window->active_tab = 0;
 
     return SKRONTCH_OK;
 }
 
void window_manager_shutdown(window_state_t *window)
 {
    if (window == NULL) {
         return;
     }
 
    if (window->renderer != NULL) {
        SDL_DestroyRenderer(window->renderer);
        window->renderer = NULL;
     }
 
    if (window->window != NULL) {
        SDL_DestroyWindow(window->window);
        window->window = NULL;
     }

    cursor_manager_shutdown(&window->cursor_manager);
 }
 
int window_manager_handle_event(window_state_t *window, const SDL_Event *event)
{
    if (window == NULL || event == NULL) {
        return 0;
    }

    int state_changed = 0;
    tab_state_t *tab = window_get_active_tab(window);

    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
            event->window.event == SDL_WINDOWEVENT_RESIZED) {
            window_manager_set_size(window, event->window.data1, event->window.data2);
            state_changed = 1;
        }
        if (event->window.event == SDL_WINDOWEVENT_MOVED) {
            state_changed = 1;
        }
        if (event->window.event == SDL_WINDOWEVENT_ENTER) {
            cursor_manager_set_mouse_inside(&window->cursor_manager, 1);
        }
        if (event->window.event == SDL_WINDOWEVENT_LEAVE) {
            window->hover_header = 0;
            window->hover_header_close = 0;
            window->hover_menu_button = 0;
            window->hover_split_node = -1;
            window->hover_drop_zone = DROP_ZONE_NONE;
            window->hover_tab_index = -1;
            window->hover_tab_add = 0;
            window->hover_header_detach = 0;
            cursor_manager_set_mouse_inside(&window->cursor_manager, 0);
        }
        if (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
            int mouse_x = 0;
            int mouse_y = 0;
            Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
            if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 && !window->is_mouse_pressed) {
                window->is_mouse_pressed = 1;
                state_changed |= window_manager_handle_mouse_down(window, mouse_x, mouse_y);
            }
        }
    }

    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        window->is_mouse_pressed = 1;
        state_changed |= window_manager_handle_mouse_down(window, event->button.x, event->button.y);
    }

    if (event->type == SDL_MOUSEBUTTONUP && event->button.button == SDL_BUTTON_LEFT) {
        window->is_mouse_pressed = 0;

        if (window->is_dragging_tab) {
            tab_info_t tabs[MAX_TABS];
            int tab_count = 0;
            SDL_Rect tab_add = make_rect(0, 0, 0, 0);
            build_tab_layout(window, tabs, &tab_count, &tab_add);
            int insert_index = compute_tab_insert_index(event->button.x, tabs, tab_count);
            int from = window->dragged_tab_index;
            int to = insert_index;
            if (to > from) {
                to -= 1;
            }
            if (to < 0) {
                to = 0;
            }
            if (to >= window->tab_count) {
                to = window->tab_count - 1;
            }
            if (from >= 0 && from < window->tab_count && to != from) {
                tab_state_t temp = window->tabs[from];
                if (to < from) {
                    memmove(&window->tabs[to + 1], &window->tabs[to],
                        sizeof(tab_state_t) * (from - to));
                } else {
                    memmove(&window->tabs[from], &window->tabs[from + 1],
                        sizeof(tab_state_t) * (to - from));
                }
                window->tabs[to] = temp;

                int active = window->active_tab;
                if (active == from) {
                    active = to;
                } else if (from < active && to >= active) {
                    active -= 1;
                } else if (from > active && to <= active) {
                    active += 1;
                }
                window->active_tab = active;
                state_changed = 1;
            }
        }

        if (tab != NULL && window->is_dragging_header) {
            leaf_info_t leaves[MAX_SPLIT_NODES];
            split_bar_t bars[MAX_SPLIT_NODES];
            int leaf_count = 0;
            int bar_count = 0;
            split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);

            if (!window->esc_cancel_active &&
                window->hover_pane_index >= 0 &&
                window->dragged_pane_node >= 0 &&
                window->hover_pane_index < leaf_count) {
                int target_node = leaves[window->hover_pane_index].node_index;
                int dragged_node = window->dragged_pane_node;

                if (window->hover_drop_zone == DROP_ZONE_CENTER) {
                    int temp_id = tab->nodes[target_node].pane_id;
                    tab->nodes[target_node].pane_id = tab->nodes[dragged_node].pane_id;
                    tab->nodes[dragged_node].pane_id = temp_id;
                    tab->focused_pane_node = target_node;
                    state_changed = 1;
                } else if (window->hover_drop_zone != DROP_ZONE_NONE &&
                    target_node != dragged_node) {
                    int dragged_pane_id = tab->nodes[dragged_node].pane_id;
                    int target_pane_id = tab->nodes[target_node].pane_id;

                    split_tree_detach_leaf(tab, dragged_node);

                    int existing_leaf = split_tree_add_leaf(tab, target_pane_id);
                    if (existing_leaf >= 0) {
                        split_node_t *target = &tab->nodes[target_node];
                        target->is_leaf = 0;
                        target->pane_id = 0;
                        target->ratio = 0.5f;
                        target->orientation = (window->hover_drop_zone == DROP_ZONE_LEFT ||
                                               window->hover_drop_zone == DROP_ZONE_RIGHT)
                            ? SPLIT_ORIENTATION_VERTICAL
                            : SPLIT_ORIENTATION_HORIZONTAL;

                        tab->nodes[dragged_node].is_leaf = 1;
                        tab->nodes[dragged_node].pane_id = dragged_pane_id;
                        tab->nodes[dragged_node].orientation = SPLIT_ORIENTATION_NONE;
                        tab->nodes[dragged_node].ratio = 0.0f;
                        tab->nodes[dragged_node].first = -1;
                        tab->nodes[dragged_node].second = -1;

                        if (window->hover_drop_zone == DROP_ZONE_LEFT ||
                            window->hover_drop_zone == DROP_ZONE_TOP) {
                            target->first = dragged_node;
                            target->second = existing_leaf;
                        } else {
                            target->first = existing_leaf;
                            target->second = dragged_node;
                        }

                        tab->nodes[target->first].parent = target_node;
                        tab->nodes[target->second].parent = target_node;
                        state_changed = 1;
                    }
                }
            }
        }

        window->is_dragging_split = 0;
        window->is_dragging_header = 0;
        window->is_dragging_tab = 0;
        window->dragged_pane_node = -1;
        window->dragged_split_node = -1;
        window->dragged_tab_index = -1;
        window->hover_tab_insert = -1;
        window->hover_drop_zone = DROP_ZONE_NONE;
        window_manager_update_cursor(window);
    }

    if (event->type == SDL_MOUSEMOTION) {
        int mouse_x = event->motion.x;
        int mouse_y = event->motion.y;
        window->mouse_x = mouse_x;
        window->mouse_y = mouse_y;
        cursor_manager_set_mouse_position(&window->cursor_manager, mouse_x, mouse_y);

        if (tab != NULL) {
            leaf_info_t leaves[MAX_SPLIT_NODES];
            split_bar_t bars[MAX_SPLIT_NODES];
            int leaf_count = 0;
            int bar_count = 0;
            split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);
            window->hover_pane_index = hit_test_leaf(mouse_x, mouse_y, leaves, leaf_count);
            window->hover_split_node = -1;
            window->hover_header = 0;
            window->hover_header_close = 0;
            window->hover_header_detach = 0;
            window->hover_menu_button = 0;
            window->hover_tab_index = -1;
            window->hover_tab_add = 0;
            window->hover_drop_zone = DROP_ZONE_NONE;

            int bar_index = hit_test_split_bar(mouse_x, mouse_y, bars, bar_count);
            if (bar_index >= 0) {
                window->hover_split_node = bars[bar_index].node_index;
            }

            SDL_Rect menu_rect = get_menu_bar_rect(window);
            SDL_Rect menu_close = get_menu_close_rect(window);
            SDL_Rect menu_add = get_menu_add_rect(window);
            if (point_in_rect(mouse_x, mouse_y, &menu_rect) &&
                (point_in_rect(mouse_x, mouse_y, &menu_close) ||
                 point_in_rect(mouse_x, mouse_y, &menu_add))) {
                window->hover_menu_button = 1;
            }

            tab_info_t tabs[MAX_TABS];
            int tab_count = 0;
            SDL_Rect tab_add = make_rect(0, 0, 0, 0);
            build_tab_layout(window, tabs, &tab_count, &tab_add);
            int hit_tab = hit_test_tabs(mouse_x, mouse_y, tabs, tab_count);
            if (hit_tab >= 0) {
                window->hover_tab_index = tabs[hit_tab].tab_index;
            }
            if (point_in_rect(mouse_x, mouse_y, &tab_add)) {
                window->hover_tab_add = 1;
            }

            if (window->is_dragging_tab) {
                window->hover_tab_insert = compute_tab_insert_index(mouse_x, tabs, tab_count);
            }

            if (window->hover_pane_index >= 0) {
                SDL_Rect header_rect = make_rect(
                    leaves[window->hover_pane_index].rect.x,
                    leaves[window->hover_pane_index].rect.y,
                    leaves[window->hover_pane_index].rect.w,
                    HEADER_HEIGHT
                );
                SDL_Rect close_rect = get_header_close_rect(&header_rect);
                SDL_Rect detach_rect = get_header_detach_rect(&header_rect);
                if (point_in_rect(mouse_x, mouse_y, &header_rect)) {
                    if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2 &&
                        point_in_rect(mouse_x, mouse_y, &close_rect)) {
                        window->hover_header_close = 1;
                    } else if (header_rect.w >= HEADER_DETACH_SIZE + HEADER_DETACH_PADDING * 2 &&
                        point_in_rect(mouse_x, mouse_y, &detach_rect)) {
                        window->hover_header_detach = 1;
                    } else {
                        window->hover_header = 1;
                    }
                }

                if ((window->is_dragging_header || window->is_creating_pane) &&
                    !window->esc_cancel_active) {
                    window->hover_drop_zone = compute_drop_zone(
                        &leaves[window->hover_pane_index].rect,
                        mouse_x,
                        mouse_y
                    );
                }
            }

            if (window->is_dragging_split && window->dragged_split_node >= 0) {
                for (int i = 0; i < bar_count; ++i) {
                    if (bars[i].node_index == window->dragged_split_node) {
                        split_node_t *node = &tab->nodes[window->dragged_split_node];
                        if (node->orientation == SPLIT_ORIENTATION_VERTICAL) {
                            float ratio = (float)(mouse_x - bars[i].container.x) /
                                (float)bars[i].container.w;
                            float min_ratio = (float)MIN_PANE_SIZE / (float)bars[i].container.w;
                            float max_ratio = 1.0f - min_ratio;
                            node->ratio = clamp_float(ratio, min_ratio, max_ratio);
                            state_changed = 1;
                        } else if (node->orientation == SPLIT_ORIENTATION_HORIZONTAL) {
                            float ratio = (float)(mouse_y - bars[i].container.y) /
                                (float)bars[i].container.h;
                            float min_ratio = (float)MIN_PANE_SIZE / (float)bars[i].container.h;
                            float max_ratio = 1.0f - min_ratio;
                            node->ratio = clamp_float(ratio, min_ratio, max_ratio);
                            state_changed = 1;
                        }
                        break;
                    }
                }
            }
        }

        window_manager_update_cursor(window);
    }

    if (event->type == SDL_KEYDOWN && tab != NULL) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            window->esc_cancel_active = 1;
            window->hover_drop_zone = DROP_ZONE_NONE;
            window->is_creating_pane = 0;
            window->pending_pane_id = 0;
        }
        if (event->key.keysym.sym == SDLK_TAB &&
            (event->key.keysym.mod & KMOD_CTRL) != 0) {
            leaf_info_t leaves[MAX_SPLIT_NODES];
            split_bar_t bars[MAX_SPLIT_NODES];
            int leaf_count = 0;
            int bar_count = 0;
            split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);
            sort_leaves_by_position(leaves, leaf_count);

            if (leaf_count > 0) {
                int current_index = 0;
                for (int i = 0; i < leaf_count; ++i) {
                    if (leaves[i].node_index == tab->focused_pane_node) {
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
                tab->focused_pane_node = leaves[next_index].node_index;
                state_changed = 1;
            }
        }
        if (event->key.keysym.sym == SDLK_q &&
            (event->key.keysym.mod & KMOD_CTRL) != 0) {
            window->should_close = 1;
            state_changed = 1;
        }
    }

    if (event->type == SDL_KEYUP) {
        if (event->key.keysym.sym == SDLK_ESCAPE) {
            window->esc_cancel_active = 0;
        }
    }

    return state_changed;
}
 
void window_manager_update(window_state_t *window, float delta_seconds)
 {
    if (window == NULL) {
         return;
     }
 
     (void)delta_seconds;
 }
 
void window_manager_render(window_state_t *window)
 {
    if (window == NULL || window->renderer == NULL) {
         return;
     }
 
     int render_width = 0;
     int render_height = 0;
    if (SDL_GetRendererOutputSize(window->renderer, &render_width, &render_height) == 0) {
        window_manager_set_size(window, render_width, render_height);
     }
 
    SDL_Color background = palette_get_color("black");
    SDL_SetRenderDrawColor(window->renderer, background.r, background.g, background.b, background.a);
    SDL_RenderClear(window->renderer);
 
    tab_state_t *tab = window_get_active_tab(window);
    if (tab == NULL) {
        SDL_RenderPresent(window->renderer);
        return;
    }

    leaf_info_t leaves[MAX_SPLIT_NODES];
    split_bar_t bars[MAX_SPLIT_NODES];
    int leaf_count = 0;
    int bar_count = 0;
    split_tree_layout(window, tab, leaves, &leaf_count, bars, &bar_count);
    int has_focus_rect = 0;
    SDL_Rect focus_rect = { 0, 0, 0, 0 };
 
    for (int i = 0; i < leaf_count; ++i) {
        int pane_id = leaves[i].pane_id;
        SDL_Color pane_color = palette_get_color("black");
        SDL_SetRenderDrawColor(
            window->renderer,
            pane_color.r,
            pane_color.g,
            pane_color.b,
            pane_color.a
        );
        SDL_RenderFillRect(window->renderer, &leaves[i].rect);
 
        SDL_Rect header_rect = make_rect(
            leaves[i].rect.x,
            leaves[i].rect.y,
            leaves[i].rect.w,
            HEADER_HEIGHT
        );
        SDL_Color header_color = palette_get_color("gray1");
        SDL_SetRenderDrawColor(window->renderer, header_color.r, header_color.g, header_color.b, header_color.a);
        SDL_RenderFillRect(window->renderer, &header_rect);

        if (header_rect.w >= HEADER_BUTTON_SIZE + HEADER_BUTTON_PADDING * 2) {
            SDL_Rect close_rect = get_header_close_rect(&header_rect);
            SDL_Color icon_color = palette_get_color("white");
            SDL_SetRenderDrawColor(window->renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
            draw_x_icon(window->renderer, &close_rect, 2);
        }

        if (header_rect.w >= HEADER_DETACH_SIZE + HEADER_DETACH_PADDING * 2) {
            SDL_Rect detach_rect = get_header_detach_rect(&header_rect);
            SDL_Color icon_color = palette_get_color("white");
            SDL_SetRenderDrawColor(window->renderer, icon_color.r, icon_color.g, icon_color.b,
                icon_color.a);
            draw_up_icon(window->renderer, &detach_rect, 2);
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
            SDL_SetRenderDrawColor(window->renderer, digit_color.r, digit_color.g, digit_color.b, digit_color.a);
            draw_digit(window->renderer, pane_id, &digit_bounds);

            SDL_Rect header_digit = {
                leaves[i].rect.x + 8,
                leaves[i].rect.y + 6,
                14,
                16
            };
            draw_digit(window->renderer, pane_id, &header_digit);
        }

        if (leaves[i].node_index == tab->focused_pane_node) {
            focus_rect = leaves[i].rect;
            has_focus_rect = 1;
        }
     }
 
    for (int i = 0; i < bar_count; ++i) {
        if (bars[i].node_index == window->hover_split_node) {
            continue;
        }
        draw_split_bar(window->renderer, &bars[i], 0);
    }

    if (has_focus_rect) {
        SDL_Color focus_color = palette_get_color("gray1");
        SDL_SetRenderDrawColor(window->renderer, focus_color.r, focus_color.g, focus_color.b, focus_color.a);
        draw_border_outside(window->renderer, &focus_rect, 4);
    }

    for (int i = 0; i < bar_count; ++i) {
        if (bars[i].node_index != window->hover_split_node) {
            continue;
        }
        draw_split_bar(window->renderer, &bars[i], 1);
    }

    if ((window->is_dragging_header || window->is_creating_pane) &&
        !window->esc_cancel_active &&
        window->hover_pane_index >= 0 &&
        window->hover_pane_index < leaf_count &&
        window->hover_drop_zone != DROP_ZONE_NONE &&
        !(window->is_dragging_header &&
          leaves[window->hover_pane_index].node_index == window->dragged_pane_node)) {
        SDL_Rect target = leaves[window->hover_pane_index].rect;
        int margin_x = target.w / DROP_ZONE_MARGIN_DIVISOR;
        int margin_y = target.h / DROP_ZONE_MARGIN_DIVISOR;
        SDL_Rect left_zone = make_rect(target.x, target.y, margin_x, target.h);
        SDL_Rect right_zone = make_rect(target.x + target.w - margin_x, target.y, margin_x, target.h);
        SDL_Rect top_zone = make_rect(target.x, target.y, target.w, margin_y);
        SDL_Rect bottom_zone = make_rect(target.x, target.y + target.h - margin_y, target.w, margin_y);
        SDL_Rect center_zone = make_rect(target.x + margin_x, target.y + margin_y,
            target.w - margin_x * 2, target.h - margin_y * 2);

        SDL_Color drop_color = palette_get_color("gray2");
        SDL_SetRenderDrawBlendMode(window->renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(window->renderer, drop_color.r, drop_color.g, drop_color.b, 40);
        SDL_RenderFillRect(window->renderer, &left_zone);
        SDL_RenderFillRect(window->renderer, &right_zone);
        SDL_RenderFillRect(window->renderer, &top_zone);
        SDL_RenderFillRect(window->renderer, &bottom_zone);

        if (window->hover_drop_zone == DROP_ZONE_CENTER) {
            SDL_SetRenderDrawColor(window->renderer, drop_color.r, drop_color.g, drop_color.b, 60);
            SDL_RenderFillRect(window->renderer, &center_zone);
            SDL_SetRenderDrawColor(window->renderer, drop_color.r, drop_color.g, drop_color.b, 200);
            draw_border_inside(window->renderer, &center_zone, 2);
        } else {
            SDL_Rect preview = target;
            if (window->hover_drop_zone == DROP_ZONE_LEFT) {
                preview = left_zone;
            } else if (window->hover_drop_zone == DROP_ZONE_RIGHT) {
                preview = right_zone;
            } else if (window->hover_drop_zone == DROP_ZONE_TOP) {
                preview = top_zone;
            } else if (window->hover_drop_zone == DROP_ZONE_BOTTOM) {
                preview = bottom_zone;
            }
            SDL_SetRenderDrawColor(window->renderer, drop_color.r, drop_color.g, drop_color.b, 200);
            draw_border_inside(window->renderer, &preview, 2);
        }

        SDL_SetRenderDrawBlendMode(window->renderer, SDL_BLENDMODE_NONE);
    }

    if (window->is_dragging_header && window->dragged_pane_node >= 0) {
        SDL_Rect dragged_pane = { 0, 0, 0, 0 };
        for (int i = 0; i < leaf_count; ++i) {
            if (leaves[i].node_index == window->dragged_pane_node) {
                dragged_pane = leaves[i].rect;
                break;
            }
        }
        int outline_x = window->mouse_x - window->drag_offset_x;
        int outline_y = window->mouse_y - window->drag_offset_y;

        SDL_Rect outline_rect = make_rect(outline_x, outline_y, dragged_pane.w, dragged_pane.h);
        SDL_Color drag_outline = palette_get_color("white");
        SDL_SetRenderDrawColor(window->renderer, drag_outline.r, drag_outline.g, drag_outline.b, drag_outline.a);
        draw_border_inside(window->renderer, &outline_rect, 2);
    }

    if (window->is_creating_pane) {
        int preview_w = window->width / 4;
        int preview_h = window->height / 4;
        if (preview_w < 40) {
            preview_w = 40;
        }
        if (preview_h < 40) {
            preview_h = 40;
        }
        SDL_Rect preview_rect = make_rect(
            window->mouse_x - preview_w / 2,
            window->mouse_y - preview_h / 2,
            preview_w,
            preview_h
        );
        SDL_Color preview_color = palette_get_color("white");
        SDL_SetRenderDrawColor(window->renderer, preview_color.r, preview_color.g,
            preview_color.b, preview_color.a);
        draw_border_inside(window->renderer, &preview_rect, 2);
    }

    SDL_Rect menu_rect = get_menu_bar_rect(window);
    SDL_Color menu_color = palette_get_color("gray1");
    SDL_SetRenderDrawColor(window->renderer, menu_color.r, menu_color.g, menu_color.b, menu_color.a);
    SDL_RenderFillRect(window->renderer, &menu_rect);

    tab_info_t tabs[MAX_TABS];
    int tab_count = 0;
    SDL_Rect tab_add = make_rect(0, 0, 0, 0);
    build_tab_layout(window, tabs, &tab_count, &tab_add);
    for (int i = 0; i < tab_count; ++i) {
        SDL_Color tab_color = (tabs[i].tab_index == window->active_tab)
            ? palette_get_color("gray2")
            : palette_get_color("gray1");
        SDL_SetRenderDrawColor(window->renderer, tab_color.r, tab_color.g, tab_color.b, tab_color.a);
        SDL_RenderFillRect(window->renderer, &tabs[i].rect);
        SDL_SetRenderDrawColor(window->renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(window->renderer, &tabs[i].rect);
    }
    SDL_Color tab_icon = palette_get_color("white");
    SDL_SetRenderDrawColor(window->renderer, tab_icon.r, tab_icon.g, tab_icon.b, tab_icon.a);
    draw_plus_icon(window->renderer, &tab_add, 2);

    if (window->is_dragging_tab && window->hover_tab_insert >= 0) {
        int insert_index = window->hover_tab_insert;
        int line_x = tab_add.x;
        if (insert_index == 0 && tab_count > 0) {
            line_x = tabs[0].rect.x - 2;
        } else if (insert_index > 0 && insert_index <= tab_count) {
            int prev = insert_index - 1;
            line_x = tabs[prev].rect.x + tabs[prev].rect.w + TAB_SPACING / 2;
        }
        SDL_SetRenderDrawColor(window->renderer, tab_icon.r, tab_icon.g, tab_icon.b, tab_icon.a);
        SDL_RenderDrawLine(window->renderer, line_x, 4, line_x, MENU_HEIGHT - 4);
    }

    SDL_Color border_color = palette_get_color("black");
    SDL_SetRenderDrawColor(window->renderer, border_color.r, border_color.g, border_color.b, border_color.a);
    SDL_Rect menu_border = make_rect(0, MENU_HEIGHT, window->width, MENU_BORDER_THICKNESS);
    SDL_RenderFillRect(window->renderer, &menu_border);

    SDL_Color icon_color = palette_get_color("white");
    SDL_SetRenderDrawColor(window->renderer, icon_color.r, icon_color.g, icon_color.b, icon_color.a);
    SDL_Rect menu_add = get_menu_add_rect(window);
    SDL_Rect menu_close = get_menu_close_rect(window);
    draw_plus_icon(window->renderer, &menu_add, 2);
    draw_x_icon(window->renderer, &menu_close, 2);

    cursor_manager_render(&window->cursor_manager);

    SDL_RenderPresent(window->renderer);
 }

void window_manager_set_tabs(window_state_t *window, const tab_state_t *tabs, int tab_count,
    int active_tab)
{
    if (window == NULL || tabs == NULL || tab_count <= 0) {
        return;
    }

    if (tab_count > MAX_TABS) {
        tab_count = MAX_TABS;
    }

    for (int i = 0; i < tab_count; ++i) {
        window->tabs[i] = tabs[i];
        if (!tab_state_is_valid(&window->tabs[i])) {
            tab_state_init_default(&window->tabs[i]);
        }
    }

    window->tab_count = tab_count;
    if (active_tab < 0 || active_tab >= tab_count) {
        active_tab = 0;
    }
    window->active_tab = active_tab;
}

int window_manager_extract_detached_tab(window_state_t *window, tab_state_t *tab_out)
{
    if (window == NULL || tab_out == NULL || !window->pending_detach) {
        return 0;
    }

    int tab_index = window->pending_detach_tab;
    int node_index = window->pending_detach_node;
    if (tab_index < 0 || tab_index >= window->tab_count) {
        window->pending_detach = 0;
        return 0;
    }

    tab_state_t *source = &window->tabs[tab_index];
    if (node_index < 0 || node_index >= source->node_count) {
        window->pending_detach = 0;
        return 0;
    }
    if (!source->nodes[node_index].is_leaf) {
        window->pending_detach = 0;
        return 0;
    }

    leaf_info_t leaves[MAX_SPLIT_NODES];
    split_bar_t bars[MAX_SPLIT_NODES];
    int leaf_count = 0;
    int bar_count = 0;
    split_tree_layout(window, source, leaves, &leaf_count, bars, &bar_count);

    int pane_id = source->nodes[node_index].pane_id;
    tab_state_init_single(tab_out, pane_id);

    if (leaf_count <= 1) {
        window->should_close = 1;
    } else {
        split_tree_detach_leaf(source, node_index);
        if (source->focused_pane_node == node_index && leaf_count > 1) {
            source->focused_pane_node = leaves[0].node_index;
        }
    }

    window->pending_detach = 0;
    window->pending_detach_tab = -1;
    window->pending_detach_node = -1;
    window->pending_detach_pane_id = 0;
    return 1;
}

void window_manager_get_window_rect(window_state_t *window, int *x, int *y, int *w, int *h)
{
    if (window == NULL) {
        return;
    }

    int pos_x = 0;
    int pos_y = 0;
    int width = window->width;
    int height = window->height;

    if (window->window != NULL) {
        SDL_GetWindowPosition(window->window, &pos_x, &pos_y);
        SDL_GetWindowSize(window->window, &width, &height);
    }

    if (x != NULL) {
        *x = pos_x;
    }
    if (y != NULL) {
        *y = pos_y;
    }
    if (w != NULL) {
        *w = width;
    }
    if (h != NULL) {
        *h = height;
    }
}
