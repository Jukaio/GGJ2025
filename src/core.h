#pragma once

#include <SDL3/SDL_Keyboard.h>
#include <SDL3/SDL_Mouse.h>
#include <SDL3/SDL_video.h>

using i32 = int;
using i64 = long long;
using u32 = unsigned int;
using u64 = unsigned long long;

struct AssetRef
{
	uint64_t offset;
	uint64_t size;
};

#if _WIN32
void message_box(const char* title, const char* body);
#define ASSERT(condition, msg)                                                  \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char buffer[1024];                                                  \
            SDL_snprintf(buffer, sizeof(buffer),                                    \
                     "Assertion failed: (%s)\nFile: %s\nLine: %d\nMessage: ",   \
                     #condition, __FILE__, __LINE__);                           \
            SDL_snprintf(buffer + SDL_strlen(buffer),                                   \
                     sizeof(buffer) - SDL_strlen(buffer),                           \
                     "%s", msg);                                                \
            message_box("Assertion Failed", buffer);                            \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)

#define ASSERT_FMT(condition, format, ...)                                      \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char buffer[1024];                                                  \
            SDL_snprintf(buffer, sizeof(buffer),                                    \
                     "Assertion failed: (%s)\nFile: %s\nLine: %d\nMessage: ",   \
                     #condition, __FILE__, __LINE__);                           \
            SDL_snprintf(buffer + SDL_strlen(buffer),                                   \
                     sizeof(buffer) - SDL_strlen(buffer),                           \
                     format, ##__VA_ARGS__);                                    \
            message_box("Assertion Failed", buffer);                            \
            __debugbreak();                                                     \
        }                                                                       \
    } while (0)
#else
#define ASSERT(...)
#define ASSERT_FMT(...)
#endif

struct KeyboardState
{
	bool state[SDL_SCANCODE_COUNT];
};

enum
{
	KEYBOARD_STATE_FRAME_COUNT = 2
};

struct KeyboardDevice
{
	union
	{
		KeyboardState state[KEYBOARD_STATE_FRAME_COUNT];

		struct
		{
			KeyboardState current;
			KeyboardState previous;
			// ...
		};
	};
};

struct MouseState
{
	float x;
	float y;

	Uint32 buttons;
};

enum
{
	MOUSE_STATE_FRAME_COUNT = 2
};

struct MouseDevice
{
	// 0 = current; 1 = previous; ...
	union
	{
		MouseState frames[MOUSE_STATE_FRAME_COUNT];

		struct
		{
			MouseState current;
			MouseState previous;
			// ...
		};
	};
};

struct InputDevice
{
	MouseDevice mouse;
	KeyboardDevice keyboard;
};

struct App
{
	struct SDL_Window* window;
	struct SDL_Renderer* renderer;
	class Upgrades* upgrades;

	InputDevice input;

	float tick_frequency;
	float tick_accumulator;

	float now;
	float delta_time;

	class UiState* ui;
};

enum class Upgrade
{
	BubbleDoubbler1,
	BubbleDoubler2,
	BubbleTriple2,
	BubbleTripler1,

	AutoBubble1,
	AutoBubble3,
	AutoBubble2,
	AutoBubble4,

	Count
};

inline void update(KeyboardDevice* keyboard_state)
{
	int num_keys = 0;
	const bool* current_state = SDL_GetKeyboardState(&num_keys);

	const size_t size = num_keys * sizeof(*keyboard_state->current.state);

	SDL_memcpy((bool*)keyboard_state->previous.state, (bool*)keyboard_state->current.state, size);

	SDL_memcpy((bool*)keyboard_state->current.state, (bool*)current_state, size);
}

inline bool key_down(KeyboardDevice const* keyboard_state, SDL_Scancode scancode, int frame_index = 0)
{
	return keyboard_state->state[frame_index].state[(size_t)scancode];
}

inline bool key_up(KeyboardDevice const* keyboard_state, SDL_Scancode scancode, int frame_index = 0)
{
	return !key_down(keyboard_state, scancode, frame_index);
}

inline bool key_just_down(KeyboardDevice const* keyboard_state, SDL_Scancode scancode)
{
	return key_down(keyboard_state, scancode, 0) && key_up(keyboard_state, scancode, 1);
}

inline void update(MouseDevice* mouse_state)
{
	mouse_state->previous = mouse_state->current;

	float* x = &mouse_state->current.x;
	float* y = &mouse_state->current.y;
	mouse_state->current.buttons = SDL_GetMouseState(x, y);
}

inline bool button_down(const MouseDevice* mouse_state, int button_index, int frame_index = 0)
{
	ASSERT(frame_index < MOUSE_STATE_FRAME_COUNT, "Cannot go that much back in time - Should we fallback to previous?");

	int button_mask = SDL_BUTTON_MASK(button_index);
	const MouseState* state = &mouse_state->frames[frame_index];
	return (state->buttons & button_mask) == button_mask;
}

inline bool button_up(const MouseDevice* mouse_state, int button_index, int frame_index = 0)
{
	return !button_down(mouse_state, button_index, frame_index);
}

inline bool button_just_down(const MouseDevice* mouse_state, int button_index)
{
	return button_down(mouse_state, button_index, 0) && !button_down(mouse_state, button_index, 1);
}

inline void update(InputDevice* input)
{
	update(&input->keyboard);
	update(&input->mouse);
}

inline bool GetWindowSizeI(SDL_Window* wnd, int* w, int* h)
{
	return SDL_GetWindowSizeInPixels(wnd, w, h);
}