#include <SDL3_ttf/SDL_ttf.h>

#include "prototypes.h"
#include "types.h"

void animation_add(BubbleAnimation* animation, Sprite sprite)
{
	animation->sprites[animation->count] = sprite;
	animation->count = animation->count + 1;
}

void animation_create(BubbleAnimation* animation, float duration, size_t sprite_count, ...)
{
	ASSERT(animation != nullptr, "Animation must not be nullptr");
	SDL_zerop(animation);

	va_list argp;
	va_start(argp, sprite_count);
	for (int i = 0; i < sprite_count; i++)
	{
		Sprite sprite = va_arg(argp, Sprite);
		animation_add(animation, sprite);
	}
	va_end(argp);

	animation->duration = duration;
}

void animation_start(BubbleAnimation* animation)
{
	play(Audio::PopCrit);
	animation->state = BubbleAnimationStatePlaying;
	animation->accumulator = 0.0f;
}


bool animation_try_get_current(const BubbleAnimation* animation, Sprite* sprite)
{
	if (animation->count == 0)
	{
		return false;
	}
	float normalised_time = animation->accumulator / animation->duration;
	uint32_t current = (uint32_t)SDL_clamp(normalised_time * animation->count, 0, animation->count - 1);
	*sprite = animation->sprites[current];
	return true;
}

bool animation_update(App* app, BubbleAnimation* animation)
{
	if (animation->accumulator > animation->duration)
	{
		animation->state = BubbleAnimationStateStop;
		return false;
	}
	animation->accumulator = animation->accumulator + app->delta_time;
	return true;
}

static bool is_recolourable(Sprite sprite) {
	switch (sprite) {
	case Sprite::BubbleGhostEyes:
	case Sprite::BubbleDead:
	case Sprite::BubbleWeirdMouth:
	case Sprite::BubbleGlasses:
	case Sprite::BubbleSunglasses:
	case Sprite::BubbleSunglassesPink:
	case Sprite::BubbleBow:
	case Sprite::BubblesTie:
	case Sprite::BubbleDevil:
	case Sprite::BubbleAngel:
		return false;
	}
	return true;
}

bool animation_render(App* app, const BubbleAnimation* animation, const Bubble* bubble)
{
	Sprite sprite;
	if (animation->state == BubbleAnimationStatePlaying && animation_try_get_current(animation, &sprite))
	{
		render(app, bubble, sprite);
		return true;
	}
	return false;
}

void render(App* app, const Bubble* bubble, Sprite sprite, float offset_x, float offset_y)
{
	SDL_Color c = bubble->color;

	SDL_Texture* texture = tex[(uint64_t)sprite];

	if (is_recolourable(sprite))
	{
		SDL_SetTextureColorMod(texture, c.r, c.g, c.b);
	}

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
			int window_width, window_height;
			SDL_GetWindowSize(app->window, &window_width, &window_height);
			float window_width_half = window_width / 2.0f;
			float window_height_half = window_height / 2.0f;

			SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
			float w, h;
			SDL_GetTextureSize(texture, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };
			SDL_FRect dst = SDL_FRect{ window_width_half, window_height_half, player_bubble->max_radius * 4.0f, player_bubble->max_radius * 4.0f };
			dst.x -= player_bubble->max_radius * 2.0f;
			dst.y -= player_bubble->max_radius * 2.0f;
			SDL_RenderTexture9Grid(app->renderer, texture, &src, 128.0f, 128.0f, 128.0f, 128.0f, 0.26f, &dst);
			//SDL_RenderTexture(app->renderer, texture, &src, &dst);
		}

		if (!animation_render(app, &player_bubble->pop_animation, &player_bubble->bubble))
		{

			if (player_bubble->is_cat && player_bubble->is_ghost)
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
			else
			{
				render(app, bubble, Sprite::BubbleBase);
			}
			if (player_bubble->has_has_glare)
			{
				render(app, bubble, Sprite::BubbleGlare);
			}
			if (player_bubble->has_devil_horns)
			{
				render(app, bubble, Sprite::BubbleDevil);
			}
			if (player_bubble->has_bow)
			{
				render(app, bubble, Sprite::BubbleBow);
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
				render(app, bubble, Sprite::BubbleSunglassesPink);
			}
			if (player_bubble->has_tie)
			{
				render(app, bubble, Sprite::BubblesTie);
			}
			if (player_bubble->has_has_glare)
			{
				render(app, bubble, Sprite::BubbleGlare);
			}
			if (player_bubble->has_crown)
			{
				render( app, bubble, Sprite::BubbleCrown );
			}
		}
	}
}

void render(App* app, AutoBubble* bubbles, size_t count)
{
	for (size_t index = 0; index < count; index++)
	{
		AutoBubble* auto_bubble = &bubbles[index];
		const Bubble* bubble = &auto_bubble->bubble;
		if (auto_bubble->is_dead) {
			continue;
		}

		if (!animation_render(app, &auto_bubble->pop_animation, &auto_bubble->bubble))
		{
			uint8_t base_mask = 0 & auto_bubble->archetype;
			if (base_mask == 0)
			{
				render(app, bubble, Sprite::BubbleBase);
			}
			else if (auto_bubble->is_cat && auto_bubble->is_ghost)
			{
				render(app, bubble, Sprite::BubbleGhostCat);
			}
			else if (auto_bubble->is_cat)
			{
				render(app, bubble, Sprite::BubbleKot);
			}
			else if (auto_bubble->is_ghost)
			{
				render(app, bubble, Sprite::BubbleGhost);
			}
			if (auto_bubble->has_dead_eyes)
			{
				render(app, bubble, Sprite::BubbleDead);
			}
			if (auto_bubble->has_has_weird_mouth)
			{
				render(app, bubble, Sprite::BubbleWeirdMouth);
			}
			if (auto_bubble->has_sun_glasses)
			{
				render(app, bubble, Sprite::BubbleSunglassesPink);
			}
			if (auto_bubble->has_bow)
			{
				render(app, bubble, Sprite::BubbleBow);
			}
			if (auto_bubble->has_tie)
			{
				render(app, bubble, Sprite::BubblesTie);
			}
			if (auto_bubble->has_has_glare)
			{
				render(app, bubble, Sprite::BubbleGlare);
			}
		}
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

