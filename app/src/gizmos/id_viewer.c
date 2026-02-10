#include "id_viewer.h"

#include <SDL.h>
#include <stdlib.h>

typedef struct id_viewer_state_t {
    int width;
    int height;
} id_viewer_state_t;

static void draw_segment(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    SDL_RenderFillRect(renderer, rect);
}

static void draw_digit(SDL_Renderer *renderer, int digit, const SDL_Rect *bounds)
{
    static const unsigned char masks[10] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
    };

    if (digit < 0 || digit > 9 || renderer == NULL || bounds == NULL) {
        return;
    }

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
        { x + thickness, y, long_w, thickness },
        { x + w - thickness, y + thickness, thickness, long_h },
        { x + w - thickness, y + thickness * 2 + long_h, thickness, long_h },
        { x + thickness, y + h - thickness, long_w, thickness },
        { x, y + thickness * 2 + long_h, thickness, long_h },
        { x, y + thickness, thickness, long_h },
        { x + thickness, y + thickness + long_h, long_w, thickness }
    };

    unsigned char mask = masks[digit];
    for (int i = 0; i < 7; ++i) {
        if (mask & (1u << i)) {
            draw_segment(renderer, &segments[i]);
        }
    }
}

static void draw_number(SDL_Renderer *renderer, int value, const SDL_Rect *bounds)
{
    int digits[8];
    int count = 0;

    if (value < 0) {
        value = 0;
    }
    if (value == 0) {
        digits[count++] = 0;
    } else {
        while (value > 0 && count < 8) {
            digits[count++] = value % 10;
            value /= 10;
        }
        for (int i = 0; i < count / 2; ++i) {
            int temp = digits[i];
            digits[i] = digits[count - 1 - i];
            digits[count - 1 - i] = temp;
        }
    }

    int spacing = bounds->w / 6;
    if (spacing < 2) {
        spacing = 2;
    }
    int total_width = bounds->w * count + spacing * (count - 1);
    int start_x = bounds->x - total_width / 2;

    for (int i = 0; i < count; ++i) {
        SDL_Rect digit_bounds = {
            start_x + i * (bounds->w + spacing),
            bounds->y,
            bounds->w,
            bounds->h
        };
        draw_digit(renderer, digits[i], &digit_bounds);
    }
}

static int id_viewer_init(gizmo_instance_t *gizmo)
{
    if (gizmo == NULL) {
        return 0;
    }
    id_viewer_state_t *state = (id_viewer_state_t *)SDL_calloc(1, sizeof(*state));
    if (state == NULL) {
        return 0;
    }
    gizmo->gizmo_state = state;
    return 1;
}

static void id_viewer_on_resize(gizmo_instance_t *gizmo, int width, int height)
{
    if (gizmo == NULL || gizmo->gizmo_state == NULL) {
        return;
    }
    id_viewer_state_t *state = (id_viewer_state_t *)gizmo->gizmo_state;
    state->width = width;
    state->height = height;
}

static void id_viewer_update_and_render(gizmo_instance_t *gizmo, float dt,
    const gizmo_input_state_t *input)
{
    (void)dt;
    if (gizmo == NULL || gizmo->gui == NULL || gizmo->gui->renderer == NULL) {
        return;
    }

    SDL_Renderer *renderer = gizmo->gui->renderer;
    SDL_Rect bounds = gizmo->bounds;
    if (bounds.w <= 0 || bounds.h <= 0) {
        return;
    }

    SDL_Color bg = { 0x22, 0x23, 0x23, 0xFF };
    SDL_Color border = { 0x5F, 0x61, 0x62, 0xFF };
    SDL_Color fg = { 0xF0, 0xF6, 0xF0, 0xFF };
    if (input != NULL && input->is_focused) {
        border.r = 0xA4;
        border.g = 0xA9;
        border.b = 0xA5;
    }

    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(renderer, &bounds);

    SDL_SetRenderDrawColor(renderer, border.r, border.g, border.b, border.a);
    SDL_RenderDrawRect(renderer, &bounds);

    int digit_w = bounds.w / 10;
    int digit_h = bounds.h / 4;
    if (digit_w < 14) {
        digit_w = 14;
    }
    if (digit_h < 24) {
        digit_h = 24;
    }

    SDL_Rect number_bounds = {
        bounds.x + bounds.w / 2,
        bounds.y + bounds.h / 2 - digit_h / 2,
        digit_w,
        digit_h
    };

    SDL_SetRenderDrawColor(renderer, fg.r, fg.g, fg.b, fg.a);
    draw_number(renderer, gizmo->pane_id, &number_bounds);
}

static void id_viewer_shutdown(gizmo_instance_t *gizmo)
{
    if (gizmo == NULL) {
        return;
    }
    if (gizmo->gizmo_state != NULL) {
        SDL_free(gizmo->gizmo_state);
        gizmo->gizmo_state = NULL;
    }
}

static int id_viewer_clone_state(const gizmo_instance_t *source, gizmo_instance_t *dest)
{
    (void)source;
    (void)dest;
    return 1;
}

const gizmo_api_t id_viewer_gizmo_api = {
    "id_viewer",
    "ID Viewer",
    id_viewer_init,
    id_viewer_on_resize,
    id_viewer_update_and_render,
    id_viewer_shutdown,
    id_viewer_clone_state
};
