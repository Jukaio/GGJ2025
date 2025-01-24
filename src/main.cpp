#include <stdio.h>

#include <SDL3\SDL.h>

#include "core.h";

typedef uint64_t milliseconds;

struct keyboard_state_frame {
  bool state[SDL_SCANCODE_COUNT];
};

enum { KEYBOARD_STATE_FRAME_COUNT = 2 };

struct keyboard_device {
  union {
    keyboard_state_frame state[KEYBOARD_STATE_FRAME_COUNT];

    struct {
      keyboard_state_frame current;
      keyboard_state_frame previous;
      // ...
    };
  };
};

struct mouse_state_frame {
  float x;
  float y;

  Uint32 state;
};

enum { MOUSE_STATE_FRAME_COUNT = 2 };

struct mouse_device {
  // 0 = current; 1 = previous; ...
  union {
    mouse_state_frame frames[MOUSE_STATE_FRAME_COUNT];

    struct {
      mouse_state_frame current;
      mouse_state_frame previous;
      // ...
    };
  };
};

inline void update(keyboard_device *keyboard_state) {
  int num_keys = 0;
  bool const *current_state = SDL_GetKeyboardState(&num_keys);

  const size_t size = num_keys * sizeof(*keyboard_state->current.state);

  SDL_memcpy((bool *)keyboard_state->previous.state,
             (bool *)keyboard_state->current.state, size);

  SDL_memcpy((bool *)keyboard_state->current.state, (bool *)current_state,
             size);
}

inline bool key_down(keyboard_device const *keyboard_state,
                     SDL_Scancode scancode, int frame_index = 0) {
  return keyboard_state->state[frame_index].state[(size_t)scancode];
}

inline bool key_up(keyboard_device const *keyboard_state, SDL_Scancode scancode,
                   int frame_index = 0) {
  return !key_down(keyboard_state, scancode, frame_index);
}

inline bool key_just_down(keyboard_device const *keyboard_state,
                          SDL_Scancode scancode) {
  return key_down(keyboard_state, scancode, 0) &&
         key_up(keyboard_state, scancode, 1);
}

inline void update(mouse_device *mouse_state) {
  mouse_state->previous = mouse_state->current;

  float *x = &mouse_state->current.x;
  float *y = &mouse_state->current.y;
  mouse_state->current.state = SDL_GetMouseState(x, y);
}

inline bool button_down(mouse_device const *mouse_state, int button_index,
                        int frame_index = 0) {
  SDL_assert(
      frame_index < MOUSE_STATE_FRAME_COUNT &&
      "Cannot go that much back in time - Should we fallback to previous?");

  int button_mask = SDL_BUTTON_MASK(button_index);
  return (mouse_state->frames[frame_index].state & button_mask) == button_mask;
}

inline bool button_up(mouse_device const *mouse_state, int button_index,
                      int frame_index = 0) {
  return !button_down(mouse_state, button_index, frame_index);
}

inline bool button_just_down(mouse_device const *mouse_state,
                             int button_index) {
  return button_down(mouse_state, button_index, 0) &&
         !button_down(mouse_state, button_index, 1);
}

struct input_device {
  mouse_device mouse;
  keyboard_device keyboard;
};

struct application {
  SDL_Window *window;
  input_device input;
};

void update(input_device *input) {
  update(&input->keyboard);
  update(&input->mouse);
}

struct bubble {
  float x;
  float y;

  float vx;
  float vy;

  float radius;

  SDL_Color primary_color;
};

float math_multiply_float2(float ax, float ay, float bx, float by) {
  return (ax * bx) + (ay * by);
}

float math_distance(float ax, float ay, float bx, float by) {
  float dx = bx - ax;
  float dy = by - ay;
  float magnitude = math_multiply_float2(ax, ay, bx, by);
  if (magnitude > 0.0f) {
    return SDL_sqrtf(magnitude);
  }
  return 0.0f;
}

SDL_Rect get_rect(bubble *bubble) {
  int half = int(bubble->radius);
  int size = int(bubble->radius * 2);
  return SDL_Rect{int(bubble->x) - half, int(bubble->y) - half, size, size};
}

SDL_FRect get_frect(bubble *bubble) {
  float half = bubble->radius;
  float size = bubble->radius * 2;
  return SDL_FRect{bubble->x - half, bubble->y - half, size, size};
}

void update(application *app, bubble *bubbles, size_t count, float dt) {

  int width;
  int height;
  if (!SDL_GetWindowSizeInPixels(app->window, &width, &height)) {
    // TODO:...
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
    return;
  }

  for (size_t index = 0; index < count; index++) {
    bubble *bubble = &bubbles[index];

    float half = bubble->radius;
    float size = bubble->radius * 2;
    SDL_Rect dst = get_rect(bubble);

    float nx = 0.0f;
    float ny = 0.0f;
    if (bubble->x - half <= 0) {
      nx = nx + 1.0f;
    }
    if (bubble->x + half >= width) {
      nx = nx - 1;
    }
    if (bubble->y - half <= 0) {
      ny = ny + 1;
    }
    if (bubble->y + half >= height) {
      ny = ny - 1;
    }
    float dot = math_multiply_float2(bubble->vx, nx, bubble->vy, ny);
    bubble->vx = bubble->vx - 2.0f * dot * nx;
    bubble->vy = bubble->vy - 2.0f * dot * ny;
  }

  for (size_t index = 0; index < count; index++) {
    bubble *bubble = &bubbles[index];

    bubble->x = bubble->x + (bubble->vx * dt);
    bubble->y = bubble->y + (bubble->vy * dt);
  }

  for (size_t index = 0; index < count; index++) {
    bubble *bubble = &bubbles[index];

    mouse_state_frame const *state = &app->input.mouse.current;
    float distance = math_distance(state->x, state->y, bubble->x, bubble->y);
    if (distance < bubble->radius) {
      bubble->primary_color = SDL_Color{255, 0, 0, 255};
    } else {
      bubble->primary_color = SDL_Color{0, 0, 255, 255};
    }
  }
}

void render(SDL_Renderer *renderer, bubble *bubbles, size_t count) {
  for (size_t index = 0; index < count; index++) {
    bubble bubble = bubbles[index];

    SDL_Color c = bubble.primary_color;
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);

    SDL_FRect dst = get_frect(&bubble);

    SDL_RenderFillRect(renderer, &dst);
  }
}

int main(int argc, char *argv[]) {
  platform_init();

  application app{};

  SDL_Init(SDL_INIT_VIDEO);

  app.window = SDL_CreateWindow("huge game", 1800, 1200, 0);

  constexpr size_t bubble_count = 4;
  bubble bubbles[bubble_count];

  for (size_t index = 0; index < bubble_count; index++) {
    float x = (128.0f * index) + 128;
    bubbles[index] = bubble{x, 64.0f, 80.0f, 0.0f, 32.0f};
  }

  if (app.window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n",
                 SDL_GetError());
    return 1;
  }

  SDL_Renderer *renderer = SDL_CreateRenderer(app.window, nullptr);

  bool is_running = true;

  // milliseconds current = SDL_GetTicks();
  milliseconds tp = SDL_GetTicks();

  while (is_running) {
    milliseconds now = SDL_GetTicks();
    milliseconds delta_time = now - tp;
    tp = now;

    SDL_Event e{};
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT) {
        is_running = false;
      }
    }

    // Input
    update(&app.input);

    // Update
    float dt_seconds = delta_time / 1000.0f;

    update(&app, bubbles, bubble_count, dt_seconds);

    // Render
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    SDL_RenderClear(renderer);

    render(renderer, bubbles, bubble_count);

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyWindow(app.window);

  SDL_Quit();

  platform_destroy();
  return 0;
}