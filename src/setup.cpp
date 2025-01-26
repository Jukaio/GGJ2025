#include "assets.h"
#include "prototypes.h"
#include "types.h"

constexpr float bubble_click_duration = 0.25f;
constexpr float bubble_click_scale = 0.60f;

void setup(App* app, PlayerBubble* player_bubbles, size_t count)
{
	int window_width, window_height;
	GetWindowSizeI(app->window, &window_width, &window_height);
	int window_width_half = window_width / 2;
	int window_height_half = window_height / 2;

	SDL_assert(count == 1 && "We only handle one player bubble for now");

	animation_create(&player_bubbles->pop_animation, 1.0f, 24, Sprite::BubblePop1, Sprite::BubblePop2,
		Sprite::BubblePop3, Sprite::BubblePop4, Sprite::BubblePop5, Sprite::BubblePop6, Sprite::BubblePop7,
		Sprite::BubblePop8, Sprite::BubblePop9, Sprite::BubblePop10, Sprite::BubblePop11, NO_SPRITE,
		NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE,
		NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE);

	player_bubbles->bubble.x = window_width_half;
	player_bubbles->bubble.y = window_height_half;
	player_bubbles->bubble.xMax = window_width_half - 256.0f * 0.25f;
	player_bubbles->bubble.xMin = window_width_half + 256.0f * 0.25f;
	player_bubbles->bubble.yMax = window_height_half - 256.0f * 0.5f;
	player_bubbles->bubble.yMin = window_height_half + 256.0f * 0.5f;
	player_bubbles->bubble.radius = 256.0f;
	player_bubbles->max_radius = 256.0f;
	player_bubbles->min_radius = 228.0f;
	player_bubbles->bubble.paddding_ratio = 0.32f;
	player_bubbles->bubble.click_scale = bubble_click_scale;
	player_bubbles->bubble.duration_click = bubble_click_duration;
	player_bubbles->bubble.burst_cap = 32;
	//(player_bubbles);
}

void setup(App* app, AutoBubble* auto_bubbles, size_t auto_bubble_count)
{
	constexpr float radius = 48.0f;
	constexpr float width = 128.0f;
	constexpr float height = 96.0f;
	for (size_t index = 0; index < auto_bubble_count; index++)
	{
		AutoBubble* auto_bubble = &auto_bubbles[index];
		Bubble* bubble = &auto_bubble->bubble;

		animation_create(&auto_bubble->pop_animation, 1.0f, 24, Sprite::BubblePop1, Sprite::BubblePop2,
			Sprite::BubblePop3, Sprite::BubblePop4, Sprite::BubblePop5, Sprite::BubblePop6, Sprite::BubblePop7,
			Sprite::BubblePop8, Sprite::BubblePop9, Sprite::BubblePop10, Sprite::BubblePop11, NO_SPRITE,
			NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE,
			NO_SPRITE, NO_SPRITE, NO_SPRITE, NO_SPRITE);

		auto_bubble->width = width;
		auto_bubble->height = height;

		auto_bubble->color = bubble_pink_dark;

		auto_bubble->max_radius = 256.0f * 0.3f;
		auto_bubble->min_radius = 228.0f * 0.3f;
		auto_bubble->is_dead = true;

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

		// print(auto_bubble);
		// SDL_Log("");
	}
}

void setup(App* app, UpgradeBubble* upgrade_bubbles, size_t auto_bubble_count)
{
	int window_width, window_height;
	GetWindowSizeI(app->window, &window_width, &window_height);

	constexpr float radius = 48.0f;
	constexpr float width = 128.0f;
	constexpr float height = 96.0f;
	float origin_x = window_width - (32.0f + width);
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

		// SDL_Log("");
	}
}
