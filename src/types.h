#pragma once

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "Bouncee.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>

typedef uint64_t milliseconds;

constexpr SDL_Color background_color = SDL_Color{ 255, 212, 222, 255 };
constexpr SDL_Color bubble_pink_bright = SDL_Color{ 255, 212, 222, 255 };
constexpr SDL_Color bubble_pink = SDL_Color{ 243, 162, 190, 255 };
constexpr SDL_Color bubble_pink_dark = SDL_Color{ 212, 93, 135, 255 };
constexpr SDL_Color bubble_blue_bright = SDL_Color{ 198, 231, 228, 255 };
constexpr SDL_Color bubble_blue = SDL_Color{ 129, 191, 183, 255 };
constexpr SDL_Color bubble_blue_dark = SDL_Color{ 34, 81, 90, 255 };
constexpr SDL_Color bubble_white = SDL_Color{ 214, 250, 249, 255 };
constexpr SDL_Color bubble_white_bright = SDL_Color{ 255, 255, 255, 255 };


struct SinglePlayer
{
	int previous_upgrades_levels[(int)Upgrade::Count];
	int current_upgrade_levels[(int)Upgrade::Count];

	uint64_t addon_multiplier;
	uint64_t addon_base;

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

	struct TTF_Text* x;

	struct TTF_Text* money;
	struct TTF_Text* base;
	struct TTF_Text* multiplier;
};


struct Bubble
{
	float x;
	float y;

	float xMin;
	float xMax;
	float yMin;
	float yMax;

	float radius;
	float paddding_ratio;

	float click_scale;

	float duration_click;
	float time_point_last_clicked;

	uint32_t consecutive_clicks;

	uint32_t burst_cap;

	SDL_Color color;
};

enum BubbleAnimationState
{
	BubbleAnimationStateStop,
	BubbleAnimationStatePlaying,
};


struct BubbleAnimation
{
	Sprite sprites[32];

	BubbleAnimationState state;

	float accumulator;
	float duration;

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

	BubbleAnimation pop_animation;

	float min_radius;
	float max_radius;

	bool owned_cosmetics[16];

	union
	{
		uint64_t archetype;
		struct
		{
			uint8_t has_has_glare : 1;
			uint8_t has_halo : 1;
			uint8_t has_devil_horns : 1;
			uint8_t has_ghost_eyes : 1;
			uint8_t has_glasses : 1;
			uint8_t has_has_weird_mouth : 1;
			uint8_t is_ghost : 1;
			uint8_t has_dead_eyes : 1;
			uint8_t has_sun_glasses : 1;
			uint8_t has_bow : 1;
			uint8_t has_tie : 1;
			uint8_t is_cat : 1;
			uint8_t has_crown : 1;
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
};

struct AutoBubble
{
	Bubble bubble;

	AutoBubbleIncremental inc;

	SDL_Color color;

	bool is_dead;

	float min_radius;
	float max_radius;
	float spawned_at;
	float width;
	float height;

	BubbleAnimation pop_animation;
	float pop_countdown;

	union
	{
		uint8_t archetype;
		struct
		{
			uint8_t is_cat : 1;
			uint8_t is_ghost : 1;
			uint8_t has_dead_eyes : 1;
			uint8_t has_sun_glasses : 1;
			uint8_t has_bow : 1;
			uint8_t has_tie : 1;
			uint8_t has_has_glare : 1;
			uint8_t has_has_weird_mouth : 1;
		};
	};
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

struct SimpleDuck
{
	float duration;
	float accumulator;
	uint32_t amount;
};

void animation_create(BubbleAnimation* animation, float duration, size_t sprite_count, ...);
void animation_start(BubbleAnimation* animation);
bool animation_try_get_current(const BubbleAnimation* animation, Sprite* sprite);
bool animation_update(App* app, BubbleAnimation* animation);
bool animation_render(App* app, const BubbleAnimation* animation, const Bubble* bubble);

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline void play(Audio audio) { Mix_PlayChannel(-1, sounds[(u64)audio], 0); }
inline void playLoop(Audio audio) {
	sounds[(u64)audio]->volume = sounds[(u64)audio]->volume * 0.8f;
	Mix_PlayChannel(-1, sounds[(u64)audio], -1);
}

inline float get_legal_radius(const Bubble* bubble)
{
	float legal_ratio = 1.0f - bubble->paddding_ratio;
	return bubble->radius * legal_ratio;
}

inline SDL_Rect get_rect(const Bubble* bubble)
{
	int half = int(bubble->radius);
	int size = int(bubble->radius * 2);
	return SDL_Rect{ int(bubble->x) - half, int(bubble->y) - half, size, size };
}

inline SDL_FRect get_frect(const App* app, const Bubble* bubble)
{
	float time_difference = app->now - bubble->time_point_last_clicked;
	float frac = time_difference / bubble->duration_click;
	frac = SDL_clamp(frac, 0.0f, 1.0f);

	float scale = lerp(bubble->click_scale, 1.0f, frac);
	float half = bubble->radius * scale;
	float size = half * 2;
	return SDL_FRect{ bubble->x - half, bubble->y - half, size, size };
}

inline float get_wobble(float dt)
{
	float time = (dt - 0.0f) / 1000.0f;
	float scale = 1.0f + 0.1f * SDL_sinf(time * 5.0f);
	return scale;
}