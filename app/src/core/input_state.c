#include "input_state.h"

#include <string.h>

static Uint16 bump_frame_counter(Uint16 counter, int is_down)
{
    if (!is_down) {
        return 0;
    }
    if (counter == 0) {
        return 1;
    }
    if (counter == 0xFFFFu) {
        return counter;
    }
    return (Uint16)(counter + 1u);
}

void input_state_init(input_state_t *input)
{
    if (input == NULL) {
        return;
    }
    memset(input, 0, sizeof(*input));
}

void input_state_begin_frame(input_state_t *input)
{
    if (input == NULL) {
        return;
    }

    for (int i = 0; i < SDL_NUM_SCANCODES; ++i) {
        input->keys_down[i] = bump_frame_counter(input->keys_down[i], input->key_is_down[i]);
    }

    input->mouse_left_frames = bump_frame_counter(input->mouse_left_frames, input->mouse_left_down);
    input->mouse_right_frames = bump_frame_counter(input->mouse_right_frames, input->mouse_right_down);
    input->mouse_middle_frames = bump_frame_counter(input->mouse_middle_frames, input->mouse_middle_down);
    input->wheel_x = 0;
    input->wheel_y = 0;
}

void input_state_handle_event(input_state_t *input, const SDL_Event *event)
{
    if (input == NULL || event == NULL) {
        return;
    }

    switch (event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            int scancode = (int)event->key.keysym.scancode;
            if (scancode >= 0 && scancode < SDL_NUM_SCANCODES) {
                input->key_is_down[scancode] = (event->type == SDL_KEYDOWN) ? 1 : 0;
            }
            break;
        }
        case SDL_MOUSEMOTION:
            input->mouse_x = event->motion.x;
            input->mouse_y = event->motion.y;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            int is_down = (event->type == SDL_MOUSEBUTTONDOWN) ? 1 : 0;
            input->mouse_x = event->button.x;
            input->mouse_y = event->button.y;
            if (event->button.button == SDL_BUTTON_LEFT) {
                input->mouse_left_down = is_down;
            } else if (event->button.button == SDL_BUTTON_RIGHT) {
                input->mouse_right_down = is_down;
            } else if (event->button.button == SDL_BUTTON_MIDDLE) {
                input->mouse_middle_down = is_down;
            }
            break;
        }
        case SDL_MOUSEWHEEL:
            input->wheel_x += event->wheel.x;
            input->wheel_y += event->wheel.y;
            break;
        default:
            break;
    }
}
