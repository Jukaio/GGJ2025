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
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);
	bool is_space_press = key_just_down(&app->input.keyboard, SDL_SCANCODE_SPACE);

	for (size_t index = 0; index < count; index++)
	{
		PlayerBubble* player_bubble = &player_bubbles[index];

		if (player_bubble->pop_animation.state == BubbleAnimationStatePlaying)
		{
			animation_update(app, &player_bubble->pop_animation);
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

				player->current_money = player->current_money + (1 * (player->current_base * player->current_multiplier));
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

			t = app->now * 0.5f;
			float radiusX = (player_bubble->bubble.xMax - player_bubble->bubble.xMin) / 2.0f;
			float radiusY = (player_bubble->bubble.yMax - player_bubble->bubble.yMin) / 2.0f;
			float centerX = (player_bubble->bubble.xMax + player_bubble->bubble.xMin) / 2.0f;
			float centerY = (player_bubble->bubble.yMax + player_bubble->bubble.yMin) / 2.0f;

			float zigzagFrequency = 3.0f;
			float zigzagAmplitude = 0.2f;

			float zigzagModulation = 1.0f + zigzagAmplitude * SDL_sin(app->now * zigzagFrequency);

			radiusX = radiusX * zigzagModulation;
			radiusY = radiusY * zigzagModulation;

			player_bubble->bubble.x = centerX + SDL_cos(t) * radiusX;
			player_bubble->bubble.y = centerY + SDL_sin(t) * radiusY;
		}
	}
}

void update(App* app, SinglePlayer* player, PlayerBubble* main_player, AutoBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);

	int window_width, window_height;
	SDL_GetWindowSize(app->window, &window_width, &window_height);

	float offset_x = 0.65f * window_width;
	float offset_y = 0.15f * window_height;


	for (size_t index = 0; index < count; index++)
	{
		AutoBubble* bubble = &bubbles[index];

		if (bubble->is_dead)
		{
			continue;
		}
		if (bubble->pop_animation.state == BubbleAnimationStatePlaying)
		{
			if (!animation_update(app, &bubble->pop_animation))
			{
				bubble->is_dead = true;
			}
			continue;
		}
		bubble->pop_countdown = bubble->pop_countdown - app->delta_time;
		if (bubble->pop_countdown < 0.0f)
		{

			player->current_money = player->current_money + (1 * (player->current_base * player->current_multiplier));
			player->current_multiplier = player->current_multiplier + bubble->archetype + 1;
			animation_start(&bubble->pop_animation);
			continue;
		}

		float t = SDL_sin((app->now - bubble->spawned_at) * 0.5f) * 0.5f + 0.5f;
		bubble->bubble.radius =
			lerp(bubble->min_radius, bubble->max_radius, Bouncee::in_elastic(t) + Bouncee::out_elastic(t));

		MouseState const* state = &app->input.mouse.current;
		float global_x = bubble->bubble.x;
		float global_y = bubble->bubble.y;
		float distance = math_distance(state->x, state->y, global_x, global_y);

		if (distance < get_legal_radius(&bubble->bubble))
		{
			bubble->bubble.color = bubble_blue;

			if (is_player_clicking)
			{
				player->current_money = player->current_money + (1 * (player->current_base * player->current_multiplier));
				;
				bubble->bubble.time_point_last_clicked = app->now;
				bubble->bubble.color = bubble_blue_dark;

				bubble->bubble.consecutive_clicks = bubble->bubble.consecutive_clicks + 1;
				if (bubble->bubble.consecutive_clicks > bubble->bubble.burst_cap)
				{
					animation_start(&bubble->pop_animation);
					continue;
				}
				const uint32_t sliding_pop_window = 3;
				const Audio pops[] = {
					Audio::Pop0, Audio::Pop1, Audio::Pop2, Audio::Pop3,
					Audio::Pop4, Audio::Pop5, Audio::Pop6, Audio::Pop7,
				};

				const uint32_t pop_count = (sizeof(pops) / sizeof(*pops)) - sliding_pop_window;

				const uint32_t burst_difference = bubble->bubble.burst_cap - bubble->bubble.consecutive_clicks;
				const float frac =
					SDL_clamp(1.0f - (float(burst_difference) / float(bubble->bubble.burst_cap)), 0.0f, 1.0f);
				const uint32_t lower_bound = pop_count * frac;
				const uint32_t upper_bound = (pop_count * frac) + sliding_pop_window;
				const uint32_t random = rand() % sliding_pop_window;

				Audio target = pops[lower_bound + random];
				play(target);
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

}


void post_render_update(App* app,
	SinglePlayer* player,
	PlayerBubble* player_bubbles,
	size_t player_bubbles_count,
	AutoBubble* auto_bubbles,
	size_t auto_bubble_count,
	SinglePlayerUI* ui,
	bool force)
{
	if (force || player->previous_money != player->current_money)
	{
		if (player->current_money > 9999999999ull)
		{
			TTF_SetTextFont(ui->money, fonts_med[(u64)Font::JuicyFruity]);
		}
		else
		{
			TTF_SetTextFont(ui->money, fonts[(u64)Font::JuicyFruity]);
		}
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "%llu", player->current_money);
		if (length >= 0)
		{
			TTF_SetTextString(ui->money, ui->buffer, 0);
		}
	}

	if (force || player->previous_base != player->current_base)
	{
		if (player->current_base > 9999999999ull)
		{
			TTF_SetTextFont(ui->base, fonts_tiny[(u64)Font::JuicyFruity]);
		}
		else if (player->current_base > 9999999ull)
		{
			TTF_SetTextFont(ui->base, fonts_small[(u64)Font::JuicyFruity]);
		}
		else if (player->current_base > 9999ull)
		{
			TTF_SetTextFont(ui->base, fonts_med[(u64)Font::JuicyFruity]);
		}
		else
		{
			TTF_SetTextFont(ui->base, fonts[(u64)Font::JuicyFruity]);
		}
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "%llu", player->current_base);
		if (length >= 0)
		{
			TTF_SetTextString(ui->base, ui->buffer, 0);
		}
	}
	if (force || player->previous_multiplier != player->current_multiplier)
	{
		if (player->current_multiplier > 9999999999ull)
		{
			TTF_SetTextFont(ui->multiplier, fonts_tiny[(u64)Font::JuicyFruity]);
		}
		else if (player->current_multiplier > 9999999ull)
		{
			TTF_SetTextFont(ui->multiplier, fonts_small[(u64)Font::JuicyFruity]);
		}
		else if (player->current_multiplier > 9999ull)
		{
			TTF_SetTextFont(ui->multiplier, fonts_med[(u64)Font::JuicyFruity]);
		}
		else
		{
			TTF_SetTextFont(ui->multiplier, fonts[(u64)Font::JuicyFruity]);
		}
		int length = SDL_snprintf(ui->buffer, sizeof(ui->buffer), "%llu", player->current_multiplier);
		if (length >= 0)
		{
			TTF_SetTextString(ui->multiplier, ui->buffer, 0);
		}
	}
}


void fixed_update(App* app, SinglePlayer* player, AutoBubble* bubbles, size_t count)
{
	bool is_player_clicking = button_just_down(&app->input.mouse, 1);

	for (size_t index = 0; index < count; index++)
	{
		AutoBubble* bubble = &bubbles[index];
		if (bubble->is_dead)
		{
			continue;
		}
		if (bubble->pop_animation.state == BubbleAnimationStatePlaying)
		{
			continue;
		}
		AutoBubbleIncremental* inc = &bubble->inc;

		inc->accumulator = inc->accumulator + 1;
		while (inc->accumulator >= inc->cooldown)
		{
			inc->accumulator = inc->accumulator - inc->cooldown;
			player->current_money = player->current_money + inc->gain * (player->current_base * player->current_multiplier);
		}
	}
}

void post_render_update(SinglePlayer* player)
{
	player->previous_money = player->current_money;
	player->previous_base = player->current_base;
	player->previous_multiplier = player->current_multiplier;
}