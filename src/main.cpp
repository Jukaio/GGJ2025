#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>

#include "assets.h"
#include "core.h"
typedef uint64_t milliseconds;

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
	SDL_Window* window;
	SDL_Renderer* renderer;

	InputDevice input;

	float tick_frequency;
	float tick_accumulator;

	float now;
	float delta_time;
};

struct SinglePlayer
{
	uint64_t previous_money;
	uint64_t current_money;

	uint64_t base;
	uint64_t multiplier;
};

struct Bubble
{
	float x;
	float y;

	float radius;
	float paddding_ratio;

	float click_scale;

	float duration_click;
	float time_since_clicked;

	SDL_Color color;
};

struct PlayerBubble
{
	Bubble bubble;

	union
	{
		uint64_t archetype;
		struct
		{
			uint8_t is_basic : 1;
			uint8_t is_cat : 1;
			uint8_t has_sun_glasses : 1;
			uint8_t has_glasses : 1;
			uint8_t e04 : 1;
			uint8_t e05 : 1;
			uint8_t e06 : 1;
			uint8_t e07 : 1;
			uint8_t e08 : 1;
			uint8_t e09 : 1;
			uint8_t e10 : 1;
			uint8_t e11 : 1;
			uint8_t e12 : 1;
			uint8_t e13 : 1;
			uint8_t e14 : 1;
			uint8_t e15 : 1;
			uint8_t e16 : 1;
			uint8_t e17 : 1;
			uint8_t e18 : 1;
			uint8_t e19 : 1;
			uint8_t e20 : 1;
			uint8_t e21 : 1;
			uint8_t e22 : 1;
			uint8_t e23 : 1;
			uint8_t e24 : 1;
			uint8_t e25 : 1;
			uint8_t e26 : 1;
			uint8_t e27 : 1;
			uint8_t e28 : 1;
			uint8_t e29 : 1;
			uint8_t e30 : 1;
			uint8_t e31 : 1;
			uint8_t e32 : 1;
			uint8_t e33 : 1;
			uint8_t e34 : 1;
			uint8_t e35 : 1;
			uint8_t e36 : 1;
			uint8_t e37 : 1;
			uint8_t e38 : 1;
			uint8_t e39 : 1;
			uint8_t e40 : 1;
			uint8_t e41 : 1;
			uint8_t e42 : 1;
			uint8_t e43 : 1;
			uint8_t e44 : 1;
			uint8_t e45 : 1;
			uint8_t e46 : 1;
			uint8_t e47 : 1;
			uint8_t e48 : 1;
			uint8_t e49 : 1;
			uint8_t e50 : 1;
			uint8_t e51 : 1;
			uint8_t e52 : 1;
			uint8_t e53 : 1;
			uint8_t e54 : 1;
			uint8_t e55 : 1;
			uint8_t e56 : 1;
			uint8_t e57 : 1;
			uint8_t e58 : 1;
			uint8_t e59 : 1;
			uint8_t e60 : 1;
			uint8_t e61 : 1;
			uint8_t e62 : 1;
			uint8_t e63 : 1;
		};
	};
};


struct AutoBubbleIncremental
{
	// Config
	uint64_t cost;
	uint64_t gain;
	uint32_t cooldown;

	// Dynamic
	uint32_t accumulator;
	uint32_t amount;
};

struct AutoBubble
{
	Bubble bubble;

	AutoBubbleIncremental inc;

	SDL_Color color;

	float x;
	float y;
	float width;
	float height;
};


struct UpgradeBubbleIncremental
{
	// Config
	uint64_t cost;

	uint64_t base_bonus;
	uint64_t multiplier_bonus;
};

struct UpgradeBubble
{
	Bubble bubble;

	UpgradeBubbleIncremental inc;

	float x;
	float y;
	float width;
	float height;

	SDL_Color color;
};

float lerp(float a, float b, float t) { return a + (b - a) * t; }


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
	ASSERT(frame_index < MOUSE_STATE_FRAME_COUNT,
		"Cannot go that much back in time - Should we fallback to previous?");

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

void update(InputDevice* input)
{
	update(&input->keyboard);
	update(&input->mouse);
}


void print(const Bubble* bubble, const char* prefix = "")
{
	SDL_Log("%sBubble Pos:(%.2f, %.2f) - Radius: %.2f - Color: R: %d G: %d B: %d A: %d", prefix, bubble->x, bubble->y,
		bubble->radius, bubble->color.r, bubble->color.g, bubble->color.b, bubble->color.a);
}


void print(const PlayerBubble* bubble)
{
	SDL_Log("== PlayerBubble ==");
	print(&bubble->bubble, "PlayerBubble.");
	SDL_Log("-%sArchetype: %llu", "PlayerBubble.", bubble->archetype);
}

void print(const AutoBubbleIncremental* inc, const char* prefix = "")
{
	// Only show config, runtime data is not relevant
	SDL_Log("%sAutoBubbleIncremental Cost: %llu - Gain: %llu - Cooldown: %u", prefix, inc->cost, inc->gain,
		inc->cooldown);
}

void print(const AutoBubble* bubble, const char* prefix = "")
{
	// Only show config, runtime data is not relevant
	SDL_Log("== AutoBubble ==");
	print(&bubble->bubble, "AutoBubble.");
	print(&bubble->inc, "AutoBubble.");
	SDL_Log("%sAutoBubble Rect:(%.2f, %.2f, %.2f, %.2f) Color:(R: %d G: %d B: %d A: %d)", "AutoBubble.", bubble->x,
		bubble->y, bubble->width, bubble->height, bubble->color.r, bubble->color.g, bubble->color.b,
		bubble->color.a);
}

float math_distance(float ax, float ay, float bx, float by)
{
	float dx = bx - ax;
	float dy = by - ay;
	float magnitude = (dx * dx) + (dy * dy);
	if (magnitude > 0.0f)
	{
		return SDL_sqrtf(magnitude);
	}
	return 0.0f;
}

SDL_Rect get_rect(const Bubble* bubble)
{
	int half = int(bubble->radius);
	int size = int(bubble->radius * 2);
	return SDL_Rect{ int(bubble->x) - half, int(bubble->y) - half, size, size };
}

SDL_FRect get_frect(const App* app, const Bubble* bubble)
{
	float time_difference = app->now - bubble->time_since_clicked;
	float frac = time_difference / bubble->duration_click;
	frac = SDL_clamp(frac, 0.0f, 1.0f);

	float scale = lerp(bubble->click_scale, 1.0f, frac);
	float half = bubble->radius * scale;
	float size = half * 2;
	return SDL_FRect{ bubble->x - half, bubble->y - half, size, size };
}

void update(App* app, SinglePlayer* player, PlayerBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);
	{
		SDL_Scancode scancode_begin = SDL_SCANCODE_1;
		SDL_Scancode scancode_end = SDL_SCANCODE_9;
		for (uint32_t current = scancode_begin; current <= scancode_end; current++)
		{
			if (key_just_down(&app->input.keyboard, SDL_Scancode(current)))
			{
				uint64_t multiplier = current - scancode_begin + 1;
				player->multiplier = multiplier;
			}
		}
	}

	for (size_t index = 0; index < count; index++)
	{
		PlayerBubble* bubble = &bubbles[index];

		MouseState const* state = &app->input.mouse.current;
		float distance = math_distance(state->x, state->y, bubble->bubble.x, bubble->bubble.y);

		float legal_ratop = 1.0f - bubble->bubble.paddding_ratio;
		if (distance < bubble->bubble.radius * legal_ratop)
		{
			bubble->bubble.color = SDL_Color{ 255, 0, 0, 255 };

			if (is_player_clicking)
			{
				player->current_money = player->current_money + (player->base * player->multiplier);
				bubble->bubble.color = SDL_Color{ 255, 255, 255, 255 };
				bubble->bubble.time_since_clicked = app->now;
			}
		}
		else
		{
			bubble->bubble.color = SDL_Color{ 0, 0, 255, 255 };
		}
	}
}

void update(App* app, SinglePlayer* player, AutoBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);

	for (size_t index = 0; index < count; index++)
	{
		AutoBubble* bubble = &bubbles[index];

		MouseState const* state = &app->input.mouse.current;
		float global_x = bubble->bubble.x + bubble->x;
		float global_y = bubble->bubble.y + bubble->y;
		float distance = math_distance(state->x, state->y, global_x, global_y);
		if (player->current_money < bubble->inc.cost)
		{
			bubble->bubble.color = SDL_Color{ 128, 128, 128, 255 };
			continue;
		}

		if (distance < bubble->bubble.radius)
		{
			bubble->bubble.color = SDL_Color{ 255, 0, 0, 255 };

			if (is_player_clicking)
			{
				player->current_money = player->current_money - bubble->inc.cost;
				bubble->inc.amount = bubble->inc.amount + 1;
				bubble->bubble.time_since_clicked = app->now;
				bubble->bubble.color = SDL_Color{ 255, 255, 255, 255 };
			}
		}
		else
		{
			bubble->bubble.color = SDL_Color{ 255, 0, 255, 255 };
		}
	}
}

void update(App* app, SinglePlayer* player, UpgradeBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);

	for (size_t index = 0; index < count; index++)
	{
		UpgradeBubble* bubble = &bubbles[index];

		MouseState const* state = &app->input.mouse.current;
		float global_x = bubble->bubble.x + bubble->x;
		float global_y = bubble->bubble.y + bubble->y;
		float distance = math_distance(state->x, state->y, global_x, global_y);
		if (player->current_money < bubble->inc.cost)
		{
			bubble->bubble.color = SDL_Color{ 128, 128, 128, 255 };
			continue;
		}

		if (distance < bubble->bubble.radius)
		{
			bubble->bubble.color = SDL_Color{ 255, 0, 0, 255 };

			if (is_player_clicking)
			{
				player->current_money = player->current_money - bubble->inc.cost;
				player->base = player->base + bubble->inc.base_bonus;
				player->multiplier = player->multiplier + bubble->inc.multiplier_bonus;

				bubble->bubble.time_since_clicked = app->now;
				bubble->bubble.color = SDL_Color{ 255, 255, 255, 255 };
			}
		}
		else
		{
			bubble->bubble.color = SDL_Color{ 255, 0, 255, 255 };
		}
	}
}


void fixed_update(App* app, SinglePlayer* player, AutoBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);

	for (size_t index = 0; index < count; index++)
	{
		AutoBubble* bubble = &bubbles[index];
		AutoBubbleIncremental* inc = &bubble->inc;

		inc->accumulator = inc->accumulator + 1;
		while (inc->accumulator >= inc->cooldown)
		{
			inc->accumulator = inc->accumulator - inc->cooldown;
			player->current_money = player->current_money + inc->gain * inc->amount;
		}
	}
}

void post_render_update(SinglePlayer* player) { player->previous_money = player->current_money; }

void render(App* app, PlayerBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const Bubble* bubble = &bubbles[index].bubble;

		SDL_Color c = bubble->color;

		SDL_Texture* texture = tex[(uint64_t)Sprite::BubbleBase];
		float w, h;
		SDL_GetTextureSize(texture, &w, &h);
		SDL_FRect src = SDL_FRect{ 0, 0, w, h };

		SDL_SetTextureColorMod(texture, c.r, c.g, c.b);

		SDL_FRect dst = get_frect(app, bubble);
		SDL_RenderTexture(app->renderer, texture, &src, &dst);
	}
}

void render(App* app, AutoBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const AutoBubble* auto_bubble = &bubbles[index];
		const Bubble* bubble = &auto_bubble->bubble;

		{
			SDL_Color c = auto_bubble->color;
			SDL_SetRenderDrawColor(app->renderer, c.r, c.g, c.b, c.a);

			SDL_FRect dst = SDL_FRect{ auto_bubble->x, auto_bubble->y, auto_bubble->width, auto_bubble->height };
			SDL_RenderRect(app->renderer, &dst);
		}

		{
			SDL_Color c = bubble->color;

			SDL_Texture* texture = tex[(uint64_t)Sprite::BubbleKot];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };

			SDL_SetTextureColorMod(texture, c.r, c.g, c.b);

			SDL_FRect dst = get_frect(app, bubble);
			dst.x = dst.x + auto_bubble->x;
			dst.y = dst.y + auto_bubble->y;
			SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}
	}
}

void render(App* app, UpgradeBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const UpgradeBubble* auto_bubble = &bubbles[index];
		const Bubble* bubble = &auto_bubble->bubble;

		{
			SDL_Color c = auto_bubble->color;
			SDL_SetRenderDrawColor(app->renderer, c.r, c.g, c.b, c.a);

			SDL_FRect dst = SDL_FRect{ auto_bubble->x, auto_bubble->y, auto_bubble->width, auto_bubble->height };
			SDL_RenderRect(app->renderer, &dst);
		}

		{
			SDL_Color c = bubble->color;

			SDL_Texture* texture = tex[(uint64_t)Sprite::BubbleGhost];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };

			SDL_SetTextureColorMod(texture, c.r, c.g, c.b);

			SDL_FRect dst = get_frect(app, bubble);
			dst.x = dst.x + auto_bubble->x;
			dst.y = dst.y + auto_bubble->y;
			SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}
	}
}


void render(App* app, SinglePlayer* player)
{
	if (player->previous_money != player->current_money)
	{
		SDL_Log("Player Score: %llu", player->current_money);
	}
}


constexpr int window_width = 1920;
constexpr int window_height = 1080;

constexpr int window_width_half = window_width / 2;
constexpr int window_height_half = window_height / 2;

constexpr float bubble_click_duration = 0.25f;
constexpr float bubble_click_scale = 0.60f;

void setup(PlayerBubble* player_bubbles, size_t count)
{
	SDL_assert(count == 1 && "We only handle one player bubble for now");

	player_bubbles->bubble.x = window_width_half;
	player_bubbles->bubble.y = window_height_half;
	player_bubbles->bubble.radius = 256.0f;
	player_bubbles->bubble.paddding_ratio = 0.32f;
	player_bubbles->bubble.click_scale = bubble_click_scale;
	player_bubbles->bubble.duration_click = bubble_click_duration;

	print(player_bubbles);
}

void setup(AutoBubble* auto_bubbles, size_t auto_bubble_count)
{
	constexpr float radius = 48.0f;
	constexpr float width = 128.0f;
	constexpr float height = 96.0f;
	constexpr float origin_x = 32.0f;
	constexpr float origin_y = 32.0f;
	constexpr float offset_y = 112.0f;
	for (size_t index = 0; index < auto_bubble_count; index++)
	{
		AutoBubble* auto_bubble = &auto_bubbles[index];
		Bubble* bubble = &auto_bubble->bubble;

		auto_bubble->x = origin_x;
		auto_bubble->y = origin_y + (index * offset_y);

		auto_bubble->width = width;
		auto_bubble->height = height;

		auto_bubble->color = SDL_Color{ 255, 255, 0, 255 };

		bubble->x = radius;
		bubble->y = radius;
		bubble->radius = radius;
		bubble->color = SDL_Color{ 255, 0, 255, 255 };

		AutoBubbleIncremental config = {};
		config.cooldown = 1;
		config.cost = (32 + (index * 2) * 32);
		config.gain = 1 + (index * 2);
		auto_bubble->inc = config;

		bubble->click_scale = bubble_click_scale;
		bubble->duration_click = bubble_click_duration;

		print(auto_bubble);
		SDL_Log("");
	}
}

void setup(UpgradeBubble* upgrade_bubbles, size_t auto_bubble_count)
{
	constexpr float radius = 48.0f;
	constexpr float width = 128.0f;
	constexpr float height = 96.0f;
	constexpr float origin_x = window_width - (32.0f + width);
	constexpr float origin_y = 32.0f;
	constexpr float offset_y = 112.0f;
	for (size_t index = 0; index < auto_bubble_count; index++)
	{
		UpgradeBubble* upgrade_bubble = &upgrade_bubbles[index];
		Bubble* bubble = &upgrade_bubble->bubble;

		upgrade_bubble->x = origin_x;
		upgrade_bubble->y = origin_y + (index * offset_y);

		upgrade_bubble->width = width;
		upgrade_bubble->height = height;

		upgrade_bubble->color = SDL_Color{ 255, 255, 0, 255 };

		bubble->x = radius;
		bubble->y = radius;
		bubble->radius = radius;
		bubble->color = SDL_Color{ 255, 0, 255, 255 };

		UpgradeBubbleIncremental config = {};
		config.cost = (32 + (index * 2) * 32);
		config.base_bonus = 1 + (index * 2);
		config.multiplier_bonus = 1 + (index * 2);

		bubble->click_scale = bubble_click_scale;
		bubble->duration_click = bubble_click_duration;

		SDL_Log("");
	}
}


int main(int argc, char* argv[])
{
	platform_init();
	SDL_Init(SDL_INIT_VIDEO);

	if (!TTF_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not init TTF: %s\n", SDL_GetError());
		return -1;
	}


	App app{};
	app.tick_frequency = 1.0f;
	app.window = SDL_CreateWindow("Bubble Clicker", window_width, window_height, 0);
	if (app.window == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return -1;
	}
	app.renderer = SDL_CreateRenderer(app.window, nullptr);
	if (app.renderer == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return -1;
	}

	constexpr SDL_Color white = SDL_Color{ 255, 255, 255, 255 };
	TTF_Font* font = TTF_OpenFont("./fonts/cheeseburger/CHEESEBU.TTF", 48.0f);
	TTF_TextEngine* text_engine = TTF_CreateRendererTextEngine(app.renderer);
	TTF_Text* text = TTF_CreateText(text_engine, font, "0000000", 7);
	TTF_SetTextColor(text, 255, 255, 255, 255);

	load_assets(app.renderer);

	SinglePlayer player{};
	player.base = 1;
	player.multiplier = 1;

	// Only one bubble for now
	constexpr size_t player_bubble_count = 1;
	PlayerBubble player_bubbles[player_bubble_count];

	constexpr size_t auto_bubble_count = 9;
	AutoBubble auto_bubbles[auto_bubble_count]{};

	constexpr size_t upgrade_bubble_count = 4;
	UpgradeBubble upgrade_bubbles[upgrade_bubble_count]{};

	setup(player_bubbles, player_bubble_count);
	setup(auto_bubbles, auto_bubble_count);
	setup(upgrade_bubbles, upgrade_bubble_count);

	bool is_running = true;

	// milliseconds current = SDL_GetTicks();
	milliseconds tp = SDL_GetTicks();

	while (is_running)
	{
		milliseconds now = SDL_GetTicks();
		milliseconds delta_time = now - tp;
		tp = now;

		SDL_Event e{};
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_EVENT_QUIT)
			{
				is_running = false;
			}
		}

		// Input
		update(&app.input);

		// Fixed Update
		app.now = now / 1000.0f;
		app.delta_time = delta_time / 1000.0f;
		app.tick_accumulator = app.tick_accumulator + app.delta_time;
		while (app.tick_accumulator >= app.tick_frequency)
		{
			app.tick_accumulator = app.tick_accumulator - app.tick_frequency;
			fixed_update(&app, &player, auto_bubbles, auto_bubble_count);
		}

		// Update
		update(&app, &player, player_bubbles, player_bubble_count);
		update(&app, &player, auto_bubbles, auto_bubble_count);
		update(&app, &player, upgrade_bubbles, upgrade_bubble_count);

		// Render
		SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 0);

		SDL_RenderClear(app.renderer);

		render(&app, auto_bubbles, auto_bubble_count);
		render(&app, upgrade_bubbles, upgrade_bubble_count);
		render(&app, player_bubbles, player_bubble_count);

		render(&app, &player);

		TTF_DrawRendererText(text, 0.0f, 0.0f);

		SDL_RenderPresent(app.renderer);

		post_render_update(&player);
	}

	SDL_DestroyWindow(app.window);

	TTF_Quit();

	SDL_Quit();

	platform_destroy();
	return 0;
}