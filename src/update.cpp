#include <SDL3_ttf/SDL_ttf.h>

#include "prototypes.h"
#include "types.h"

static float math_distance(float ax, float ay, float bx, float by)
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

static void emit_particles(const App* app, Particle* particles, int x, int y, SDL_Color color, int count)
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
		particle->lifetime = (float)(rand() % 1000 + 100) / 1000.0f;

		int num = (rand() % 10 + 1);
		switch (num)
		{
		case 1:
			particle->sprite = Sprite::ParticleKot;
			particle->bubble.color = color;
			break;
		case 2:
			particle->sprite = Sprite::ParticleGhost;
			particle->bubble.color = bubble_white;
			break;
		case 3:
			particle->sprite = Sprite::ParticleGhostCat;
			particle->bubble.color = bubble_white;
			break;
		case 4:
			particle->sprite = Sprite::ParticleClick;
			particle->bubble.color = bubble_white;
			break;
		default:
			particle->sprite = Sprite::ParticleBasic;
			particle->bubble.color = color;
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

	bool is_space_press = key_just_down(&app->input.keyboard, SDL_SCANCODE_SPACE);
	for (size_t index = 0; index < player_count; index++)
	{
		PlayerBubble* player_bubble = &player_bubbles[index];
		float mx = is_space_press ? player_bubble->bubble.x : app->input.mouse.current.x;
		float my = is_space_press ? player_bubble->bubble.y : app->input.mouse.current.y;

		if (player_bubble->pop_animation.state == BubbleAnimationStatePlaying)
		{
			continue;
		}

		float time_difference = app->now - player_bubble->bubble.time_point_last_clicked;
		if (time_difference > 1.0f)
		{
			player_bubble->bubble.consecutive_clicks = 0;
		}

		float distance = math_distance(mx, my, player_bubble->bubble.x, player_bubble->bubble.y);
		if (distance < get_legal_radius(&player_bubble->bubble) || is_space_press)
		{
			bool is_player_clicking = button_just_down(&app->input.mouse, 1) || is_space_press;

			if (is_player_clicking)
			{
				uint32_t total_emit_count = player_bubble->bubble.consecutive_clicks + emit_count;
				if (*particle_count + total_emit_count > particle_capacity)
				{
					uint32_t difference = particle_capacity - *particle_count;
					emit_particles(app, particles + difference, mx, my, bubble_pink_bright, difference);
					*particle_count = particle_capacity;
				}
				else
				{
					emit_particles(app, particles + *particle_count, mx, my, bubble_pink_bright, total_emit_count);
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
	for (size_t index = 0; index < count; index++)
	{
		PlayerBubble* bubble = &player_bubbles[index];
		animation_update(app, &bubble->pop_animation);
	}

	bool is_player_clicking = button_just_down(&app->input.mouse, 1);
	bool is_space_press = key_just_down(&app->input.keyboard, SDL_SCANCODE_SPACE);

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
		PlayerBubble* player_bubble = &player_bubbles[index];

		if (player_bubble->pop_animation.state == BubbleAnimationStatePlaying)
		{
			continue;
		}

		MouseState const* state = &app->input.mouse.current;
		float distance = math_distance(state->x, state->y, player_bubble->bubble.x, player_bubble->bubble.y);

		float t = SDL_sin(app->now * 0.5f) * 0.5f + 0.5f;
		player_bubble->bubble.radius = lerp(player_bubble->min_radius, player_bubble->max_radius,
			Bouncee::in_elastic(t) + Bouncee::out_elastic(t));

		if (distance < get_legal_radius(&player_bubble->bubble) || is_space_press)
		{
			player_bubble->bubble.color = bubble_blue;

			if (is_player_clicking || is_space_press)
			{
				player_bubble->bubble.consecutive_clicks = player_bubble->bubble.consecutive_clicks + 1;
				if (player_bubble->bubble.consecutive_clicks > player_bubble->bubble.burst_cap)
				{
					animation_start(&player_bubble->pop_animation);
					player_bubble->bubble.consecutive_clicks = 0;
					play(Audio::PopCrit);
					break;
				}
				const uint32_t sliding_pop_window = 4;
				const Audio pops[] = {
					Audio::Pop0, Audio::Pop1, Audio::Pop2, Audio::Pop3, Audio::Pop4,
					Audio::Pop5, Audio::Pop6, Audio::Pop7, Audio::Pop8,
				};

				const uint32_t pop_count = (sizeof(pops) / sizeof(*pops)) - sliding_pop_window;

				const uint32_t burst_difference =
					player_bubble->bubble.burst_cap - player_bubble->bubble.consecutive_clicks;
				const float frac =
					SDL_clamp(1.0f - (float(burst_difference) / float(player_bubble->bubble.burst_cap)), 0.0f, 1.0f);


				const uint32_t lower_bound = pop_count * frac;
				const uint32_t upper_bound = (pop_count * frac) + sliding_pop_window;
				const uint32_t random = rand() % sliding_pop_window;

				Audio target = pops[lower_bound + random];
				play(target);

				player->current_money = player->current_money + (player->current_base * player->current_multiplier);
				player_bubble->bubble.time_point_last_clicked = app->now;

				player_bubble->bubble.color = bubble_blue_dark;
			}
			else
			{
				player_bubble->bubble.color = bubble_blue;
			}
		}
		else
		{
			player_bubble->bubble.color = bubble_white_bright;

			t = SDL_sin(app->now * 0.5f) * 0.5f + 0.5f;
			player_bubble->bubble.y = lerp(player_bubble->bubble.yMin, player_bubble->bubble.yMax,
				Bouncee::spike_bounce(t));
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
				Mix_PlayChannel(-1, sounds[(u64)Audio::MildPop], 0);

				player->current_money = player->current_money - bubble->inc.cost;
				bubble->inc.amount = bubble->inc.amount + 1;
				bubble->bubble.time_point_last_clicked = app->now;
				bubble->bubble.color = bubble_blue_dark;
			}
			else
			{
				bubble->bubble.color = bubble_blue;
			}
		}
		else
		{
			bubble->bubble.color = bubble_white_bright;
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
				Mix_PlayChannel(-1, sounds[(u64)Audio::MildPop], 0);

				player->current_money = player->current_money - bubble->inc.cost;
				player->current_base = player->current_base + bubble->inc.base_bonus;
				player->current_multiplier = player->current_multiplier + bubble->inc.multiplier_bonus;

				bubble->bubble.time_point_last_clicked = app->now;
				bubble->bubble.color = bubble_blue_dark;
			}
			else
			{
				bubble->bubble.color = bubble_blue;
			}
		}
		else
		{
			bubble->bubble.color = bubble_white_bright;
		}
	}
}


void post_render_update(App* app, SinglePlayer* player, SinglePlayerUI* ui, bool force)
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