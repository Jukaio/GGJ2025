#include <SDL_mixer.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#include "assets.h"
#include "Bouncee.h"
#include "core.h"
#include "ui.h"
typedef uint64_t milliseconds;

constexpr SDL_Color background_color = SDL_Color{ 129, 191, 183, 255 };
constexpr SDL_Color bubble_pink_bright = SDL_Color{ 255, 212, 222, 255 };
constexpr SDL_Color bubble_pink = SDL_Color{ 243, 162, 190, 255 };
constexpr SDL_Color bubble_pink_dark = SDL_Color{ 212, 93, 135, 255 };
constexpr SDL_Color bubble_blue_bright = SDL_Color{ 198, 231, 228, 255 };
constexpr SDL_Color bubble_blue = SDL_Color{ 129, 191, 183, 255 };
constexpr SDL_Color bubble_blue_dark = SDL_Color{ 34, 81, 90, 255 };
constexpr SDL_Color bubble_white = SDL_Color{ 214, 250, 249, 255 };

struct SinglePlayer
{
	uint64_t previous_money;
	uint64_t current_money;

	uint64_t previous_base;
	uint64_t current_base;

	uint64_t previous_multiplier;
	uint64_t current_multiplier;
};

struct SinglePlayerUI
{
	char buffer[64];

	TTF_Text* score;
	TTF_Text* base;
	TTF_Text* multiplier;
};


struct Bubble
{
	float x;
	float y;

	float radius;
	float paddding_ratio;

	float click_scale;

	float duration_click;
	float time_point_last_clicked;

	uint32_t consecutive_clicks;

	SDL_Color color;
};

struct BubbleAnimation
{
	Sprite sprites[16];
	Sprite current;

	uint8_t count;
};

enum PlayerBubbleBase : uint8_t
{
	PlayerBubbleBaseBasic = 0,
	PlayerBubbleBaseCat,
	PlayerBubbleBaseGhost,
	PlayerBubbleBaseGhostCat,
};

struct PlayerBubble
{
	Bubble bubble;

	union
	{
		uint64_t archetype;
		struct
		{
			uint8_t is_cat : 1;
			uint8_t is_ghost : 1;
			uint8_t has_dead_eyes : 1;
			uint8_t has_ghost_eyes : 1;
			uint8_t has_sun_glasses : 1;
			uint8_t has_halo : 1;
			uint8_t has_glasses : 1;
			uint8_t has_bow : 1;
			uint8_t has_tie : 1;
			uint8_t has_devil_horns : 1;
			uint8_t has_has_glare : 1;
			uint8_t has_has_weird_mouth : 1;
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

struct Particle
{
	Bubble bubble;
	float vx;
	float vy;
	float lifetime;
	Sprite sprite;
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
//
// void animation_add(BubbleAnimation* animation, )
//{
//
//}

inline float get_legal_radius(Bubble* bubble)
{
	float legal_ratio = 1.0f - bubble->paddding_ratio;
	return bubble->radius * legal_ratio;
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
	float time_difference = app->now - bubble->time_point_last_clicked;
	float frac = time_difference / bubble->duration_click;
	frac = SDL_clamp(frac, 0.0f, 1.0f);

	float scale = lerp(bubble->click_scale, 1.0f, frac);
	float half = bubble->radius * scale;
	float size = half * 2;
	return SDL_FRect{ bubble->x - half, bubble->y - half, size, size };
}

float get_wobble(float dt)
{
	float time = (dt - 0.0f) / 1000.0f;
	float scale = 1.0f + 0.1f * sinf(time * 5.0f);
	return scale;
}

void emit_particles(const App* app, Particle* particles, int x, int y, SDL_Color color, int count)
{
	for (int i = 0; i < count; ++i)
	{
		float angle = (float)(rand()) / RAND_MAX * 2.0f * PI;
		float speed = (float)(rand()) / RAND_MAX * 220.0f + 125.0f;
		float radius = (float)(rand()) / RAND_MAX * 30 + 8.0f;

		Particle* particle = &particles[i];
		particle->bubble.x = (float)x;
		particle->bubble.y = (float)y;
		particle->bubble.radius = radius;
		particle->vx = cosf(angle) * speed;
		particle->vy = sinf(angle) * speed;
		particle->lifetime = 1.0f;
		particle->bubble.color = color;

		int num = (rand() % 10 + 1);
		switch (num)
		{
		case 1:
			particle->sprite = Sprite::ParticleKot;
			break;
		case 2:
			particle->sprite = Sprite::ParticleGhost;
			break;
		case 3:
			particle->sprite = Sprite::ParticleGhostCat;
			break;
		case 4:
			particle->sprite = Sprite::ParticleClick;
			break;
		default:
			particle->sprite = Sprite::ParticleBasic;
			break;
		}
	}
}

void update(const App* app,
	Particle* particles,
	size_t* particle_count,
	size_t particle_capacity,
	PlayerBubble* player_bubbles,
	size_t player_count)
{
	const uint32_t emit_count = 4;

	for (int i = 0; i < *particle_count; ++i)
	{
		Particle* particle = &particles[i];
		particle->bubble.x += particle->vx * app->delta_time;
		particle->bubble.y += particle->vy * app->delta_time;
		particle->lifetime -= app->delta_time;
	}

	float mx = app->input.mouse.current.x;
	float my = app->input.mouse.current.y;
	for (size_t index = 0; index < player_count; index++)
	{
		PlayerBubble* player_bubble = &player_bubbles[index];

		float time_difference = app->now - player_bubble->bubble.time_point_last_clicked;
		if (time_difference > 1.0f)
		{
			player_bubble->bubble.consecutive_clicks = 0;
		}

		float distance = math_distance(mx, my, player_bubble->bubble.x, player_bubble->bubble.y);
		if (distance < get_legal_radius(&player_bubble->bubble))
		{
			bool is_player_clicking = button_just_down(&app->input.mouse, 1);

			if (is_player_clicking)
			{
				player_bubble->bubble.consecutive_clicks = player_bubble->bubble.consecutive_clicks + 1;

				uint32_t total_emit_count = player_bubble->bubble.consecutive_clicks + emit_count;
				if (*particle_count + total_emit_count > particle_capacity)
				{
					uint32_t difference = particle_capacity - *particle_count;
					emit_particles(app, particles + difference, mx, my, bubble_blue_bright, difference);
					*particle_count = particle_capacity;
				}
				else
				{
					emit_particles(app, particles + *particle_count, mx, my, bubble_blue_bright, total_emit_count);
					*particle_count = *particle_count + total_emit_count;
				}
			}
		}
	}
	for (int i = 0; i < *particle_count; ++i)
	{
		Particle* particle = &particles[i];
		if (particle->lifetime < 0.0f)
		{
			// Remove swap back
			particles[i] = particles[*particle_count - 1];
			*particle_count = *particle_count - 1;
			// Repeat current
			i--;
		}
	}
}

void update(App* app, SinglePlayer* player, PlayerBubble* player_bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);
	{
		SDL_Scancode scancode_begin = SDL_SCANCODE_1;
		SDL_Scancode scancode_end = SDL_SCANCODE_9;
		for (int current = scancode_begin; current <= scancode_end; current++)
		{
			if (key_just_down(&app->input.keyboard, (SDL_Scancode)current))
			{
				uint64_t bit_index = current - scancode_begin;
				uint64_t mask = 1ull << bit_index;
				for (size_t index = 0; index < count; index++)
				{
					PlayerBubble* bubble = &player_bubbles[index];
					bubble->archetype = mask ^ bubble->archetype;
				}
			}
		}
	}

	for (size_t index = 0; index < count; index++)
	{
		PlayerBubble* bubble = &player_bubbles[index];

		MouseState const* state = &app->input.mouse.current;
		float distance = math_distance(state->x, state->y, bubble->bubble.x, bubble->bubble.y);

		if (distance < get_legal_radius(&bubble->bubble))
		{
			bubble->bubble.color = bubble_blue;

			if (is_player_clicking)
			{
				player->current_money = player->current_money + (player->current_base * player->current_multiplier);
				bubble->bubble.time_point_last_clicked = app->now;
			}
		}
		else
		{
			bubble->bubble.color = bubble_blue_bright;
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
			bubble->bubble.color = bubble_pink_bright;
			continue;
		}

		if (distance < get_legal_radius(&bubble->bubble))
		{
			bubble->bubble.color = bubble_blue;

			if (is_player_clicking)
			{
				player->current_money = player->current_money - bubble->inc.cost;
				bubble->inc.amount = bubble->inc.amount + 1;
				bubble->bubble.time_point_last_clicked = app->now;
			}
		}
		else
		{
			bubble->bubble.color = bubble_blue_bright;
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
			bubble->bubble.color = bubble_pink_bright;
			continue;
		}

		if (distance < get_legal_radius(&bubble->bubble))
		{
			bubble->bubble.color = bubble_blue;

			if (is_player_clicking)
			{
				player->current_money = player->current_money - bubble->inc.cost;
				player->current_base = player->current_base + bubble->inc.base_bonus;
				player->current_multiplier = player->current_multiplier + bubble->inc.multiplier_bonus;

				bubble->bubble.time_point_last_clicked = app->now;
			}
		}
		else
		{
			bubble->bubble.color = bubble_blue_bright;
		}
	}
}

void update(App* app, SinglePlayer* player, SinglePlayerUI* ui, bool force = false)
{
	if (force || player->previous_money != player->current_money)
	{
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "Money: %llu", player->current_money);
		if (length >= 0)
		{
			TTF_SetTextString(ui->score, ui->buffer, length);
		}
	}
	if (force || player->previous_base != player->current_base)
	{
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "Base: %llu", player->current_base);
		if (length >= 0)
		{
			TTF_SetTextString(ui->base, ui->buffer, length);
		}
	}
	if (force || player->previous_multiplier != player->current_multiplier)
	{
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "Multiplier: %llu", player->current_multiplier);
		if (length >= 0)
		{
			TTF_SetTextString(ui->multiplier, ui->buffer, length);
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

void post_render_update(SinglePlayer* player)
{
	player->previous_money = player->current_money;
	player->previous_base = player->current_base;
	player->previous_multiplier = player->current_multiplier;
}

void render(App* app, const Bubble* bubble, Sprite sprite, float offset_x = 0.0f, float offset_y = 0.0f)
{
	SDL_Color c = bubble->color;

	SDL_Texture* texture = tex[(uint64_t)sprite];
	SDL_SetTextureColorMod(texture, c.r, c.g, c.b);
	SDL_SetTextureAlphaMod(texture, c.a);

	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };

	SDL_FRect dst = get_frect(app, bubble);
	dst.x = dst.x + offset_x;
	dst.y = dst.y + offset_y;
	SDL_RenderTexture(app->renderer, texture, &src, &dst);
}

void render(App* app, PlayerBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const PlayerBubble* player_bubble = &bubbles[index];
		const Bubble* bubble = &player_bubble->bubble;
		{
			SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };
			SDL_FRect dst = SDL_FRect{ bubble->x, bubble->y, bubble->radius * 4.0f, bubble->radius * 4.0f };
			dst.x -= bubble->radius * 2.0f;
			dst.y -= bubble->radius * 2.0f;
			SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}

		uint64_t base_mask = 0b11 & player_bubble->archetype;
		if (base_mask == 0)
		{
			render(app, bubble, Sprite::BubbleBase);
		}

		else if (player_bubble->is_cat && player_bubble->is_ghost)
		{
			render(app, bubble, Sprite::BubbleGhostCat);
		}
		else if (player_bubble->is_cat)
		{
			render(app, bubble, Sprite::BubbleKot);
		}
		else if (player_bubble->is_ghost)
		{
			render(app, bubble, Sprite::BubbleGhost);
		}

		if (player_bubble->has_halo)
		{
			render(app, bubble, Sprite::BubbleAngel);
		}
		if (player_bubble->has_dead_eyes)
		{
			render(app, bubble, Sprite::BubbleDead);
		}
		if (player_bubble->has_ghost_eyes)
		{
			render(app, bubble, Sprite::BubbleGhostEyes);
		}
		if (player_bubble->has_has_weird_mouth)
		{
			render(app, bubble, Sprite::BubbleWeirdMouth);
		}

		if (player_bubble->has_glasses)
		{
			render(app, bubble, Sprite::BubbleGlasses);
		}
		if (player_bubble->has_sun_glasses)
		{
			render(app, bubble, Sprite::BubbleSunglasses);
		}
		if (player_bubble->has_devil_horns)
		{
			render(app, bubble, Sprite::BubbleDevil);
		}
		if (player_bubble->has_bow)
		{
			render(app, bubble, Sprite::BubbleBow);
		}
		if (player_bubble->has_tie)
		{
			render(app, bubble, Sprite::BubblesTie);
		}
		if (player_bubble->has_has_glare)
		{
			render(app, bubble, Sprite::BubbleGlare);
		}
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

			SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };
			SDL_FRect dst = SDL_FRect{ auto_bubble->x, auto_bubble->y, auto_bubble->width, auto_bubble->height };

			SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}

		render(app, bubble, Sprite::BubbleKot, auto_bubble->x, auto_bubble->y);
	}
}

void render(App* app, Particle* particles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const Particle* particle = &particles[index];
		const Bubble* bubble = &particle->bubble;
		render(app, bubble, particle->sprite);
	}
}

void render(App* app, UpgradeBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		const UpgradeBubble* upgrade_bubble = &bubbles[index];
		const Bubble* bubble = &upgrade_bubble->bubble;

		{
			SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };
			SDL_FRect dst =
				SDL_FRect{ upgrade_bubble->x, upgrade_bubble->y, upgrade_bubble->width, upgrade_bubble->height };

			SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}

		render(app, bubble, Sprite::BubbleGhost, upgrade_bubble->x, upgrade_bubble->y);
	}
}


void render(App* app, SinglePlayerUI* ui)
{
	if (!TTF_DrawRendererText(ui->score, 64.0f, 64.0f))
	{
		SDL_Log(SDL_GetError());
	};
	if (!TTF_DrawRendererText(ui->base, 64.0f, 128.0f))
	{
		SDL_Log(SDL_GetError());
	};
	if (!TTF_DrawRendererText(ui->multiplier, 64.0f, 192.0f))
	{
		SDL_Log(SDL_GetError());
	};
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

		auto_bubble->color = bubble_pink_dark;

		bubble->x = radius;
		bubble->y = radius;
		bubble->radius = radius;
		bubble->color = bubble_pink;

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

		upgrade_bubble->color = bubble_blue_dark;

		bubble->x = radius;
		bubble->y = radius;
		bubble->radius = radius;
		bubble->color = bubble_blue;

		UpgradeBubbleIncremental config = {};
		config.cost = (32 + (index * 2) * 32);
		config.base_bonus = 1 + (index * 2);
		config.multiplier_bonus = 1 + (index * 2);

		bubble->click_scale = bubble_click_scale;
		bubble->duration_click = bubble_click_duration;

		upgrade_bubble->inc = config;

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
	app.ui = (UiState*)SDL_malloc(sizeof(UiState));
	SDL_zero(*app.ui);
	app.upgrades = (Upgrades*)SDL_malloc(sizeof(Upgrades));
	SDL_zero(*app.upgrades);

	app.tick_frequency = 0.25f;
	app.window = SDL_CreateWindow("Bubble Clicker", window_width, window_height, 0);
	if (app.window == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return -1;
	}
	app.renderer = SDL_CreateRenderer(app.window, nullptr);
	SDL_SetRenderVSync(app.renderer, 1);
	if (app.renderer == nullptr)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
		return -1;
	}

	load_assets(app.renderer);

	constexpr SDL_Color white = SDL_Color{ 255, 255, 255, 255 };
	TTF_TextEngine* text_engine = TTF_CreateRendererTextEngine(app.renderer);

	SinglePlayer player{};
	player.current_base = 1;
	player.current_multiplier = 1;

	SinglePlayerUI player_ui{};
	player_ui.score = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);
	TTF_SetTextColor(player_ui.score, 255, 255, 255, 255);

	for (int index = 0; index < (int)Upgrade::Count; index++)
	{
		app.upgrades->labels[index] = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);
	}

	player_ui.base = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);
	player_ui.multiplier = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);

	// Only one bubble for now
	constexpr size_t player_bubble_count = 1;
	PlayerBubble player_bubbles[player_bubble_count]{};

	constexpr size_t auto_bubble_count = 9;
	AutoBubble auto_bubbles[auto_bubble_count]{};

	constexpr size_t upgrade_bubble_count = 4;
	UpgradeBubble upgrade_bubbles[upgrade_bubble_count]{};

	// MAKE IT INSANE
	constexpr size_t particle_capacity = 1024;
	size_t particle_count = 0;
	Particle particles[particle_capacity]{};

	setup(player_bubbles, player_bubble_count);
	setup(auto_bubbles, auto_bubble_count);
	setup(upgrade_bubbles, upgrade_bubble_count);

	bool is_running = true;

	update(&app, &player, &player_ui, true);

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
		update(&app, particles, &particle_count, particle_capacity, player_bubbles, player_bubble_count);
		update(&app, &player, &player_ui);

		// Render

		SDL_SetRenderDrawColor(app.renderer, background_color.r, background_color.g, background_color.b,
			background_color.a);

		SDL_RenderClear(app.renderer);

		render(&app, auto_bubbles, auto_bubble_count);
		render(&app, upgrade_bubbles, upgrade_bubble_count);
		render(&app, player_bubbles, player_bubble_count);
		render(&app, particles, particle_count);

		render(&app, &player_ui);

		SDL_FRect canvas;
		int w, h;
		SDL_GetWindowSize(app.window, &w, &h);
		canvas.w = w;
		canvas.h = h;
		draw_tab_bottom_button(&app, &canvas);
		draw_tabs(&app, &canvas);


		SDL_RenderPresent(app.renderer);

		post_render_update(&player);
	}

	destroy_assets();

	TTF_DestroyRendererTextEngine(text_engine);

	SDL_DestroyRenderer(app.renderer);

	SDL_DestroyWindow(app.window);

	TTF_Quit();

	SDL_Quit();

	platform_destroy();
	return 0;
}