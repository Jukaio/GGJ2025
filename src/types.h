#pragma once

#include <SDL_mixer.h>
#include <SDL3/SDL.h>

#include "Bouncee.h"
#include "core.h"
#include <stdio.h>
#include <stdlib.h>


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

	struct TTF_Text* score;
	struct TTF_Text* base;
	struct TTF_Text* multiplier;
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

void animation_create(BubbleAnimation* animation, float duration, size_t sprite_count, ...);
void animation_start(BubbleAnimation* animation);
bool animation_try_get_current(const BubbleAnimation* animation, Sprite* sprite);
void animation_update(App* app, BubbleAnimation* animation);
bool animation_render(App* app, const BubbleAnimation* animation, const Bubble* bubble);

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline void play(Audio audio) { Mix_PlayChannel(-1, sounds[(u64)audio], 0); }

inline float get_legal_radius(Bubble* bubble)
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
	float scale = 1.0f + 0.1f * sinf(time * 5.0f);
	return scale;
}