#ifndef INPUT_STATE_H
#define INPUT_STATE_H

#include <SDL.h>

typedef struct input_state_t {
    int key_is_down[SDL_NUM_SCANCODES];
    Uint16 keys_down[SDL_NUM_SCANCODES];
    int mouse_x;
    int mouse_y;
    int mouse_left_down;
    int mouse_right_down;
    int mouse_middle_down;
    Uint16 mouse_left_frames;
    Uint16 mouse_right_frames;
    Uint16 mouse_middle_frames;
    int wheel_x;
    int wheel_y;
} input_state_t;

void input_state_init(input_state_t *input);
void input_state_begin_frame(input_state_t *input);
void input_state_handle_event(input_state_t *input, const SDL_Event *event);

#endif
