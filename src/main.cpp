#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "assets.h"
#include "Bouncee.h"
#include "prototypes.h"
#include "types.h"
#include "ui.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

struct PremiumDuck
{
	float accumulator;
	uint32_t amount;
};

struct LuxuryDuck
{
	float accumulator;
	uint32_t amount;
};


struct SimpleBathtubs
{
	float accumulator;
	uint32_t amount;
};
struct LuxuryBathtubs
{
	float accumulator;
	uint32_t amount;
};

constexpr SDL_Color white = SDL_Color{ 255, 255, 255, 255 };

constexpr size_t player_bubble_count = 1;
constexpr size_t auto_bubble_capacity = 256;
constexpr size_t upgrade_bubble_count = 4;
constexpr size_t particle_capacity = 1024;

App app;
TTF_TextEngine* text_engine;
SinglePlayer player;
SinglePlayerUI player_ui;
PlayerBubble player_bubbles[player_bubble_count];

size_t auto_bubble_count = 0;
AutoBubble auto_bubbles[auto_bubble_capacity];
UpgradeBubble upgrade_bubbles[upgrade_bubble_count];

size_t particle_count = 0;
Particle particles[particle_capacity];

bool is_running;
milliseconds tp;

size_t simple_bathtub_count;
SimpleBathtubs simple_bathtubs[1024];
size_t luxury_bathtub_count;
LuxuryBathtubs luxury_bathtub[1024];

SimpleDuck simple_duck;

size_t premium_duck_count;
PremiumDuck premium_simple_duck[1024];

size_t luxury_duck_count;
LuxuryDuck luxury_simple_duck[1024];

void cleanup()
{
	destroy_assets();

	TTF_DestroyRendererTextEngine(text_engine);

	SDL_DestroyRenderer(app.renderer);

	SDL_DestroyWindow(app.window);

	Mix_Quit();

	TTF_Quit();

	SDL_Quit();
}

static bool bubble_bubble_intersection(const Bubble* lhs, const Bubble* rhs)
{
	float alpha = get_legal_radius(lhs);
	float beta = get_legal_radius(rhs);
	float dx = lhs->x - rhs->x;
	float dy = lhs->y - rhs->y;
	float length = (dx * dx) + (dy * dy);
	if (length > 0.0f)
	{
		length = SDL_sqrtf(length);
	}

	return length <= alpha + beta;
}

static AutoBubble* spawn_random_auto_bubble(int burst_min,
	int burst_max,
	int pop_reward_money,
	int pop_reward_multiplier)
{
	AutoBubble* next = nullptr;

	if (auto_bubble_count >= auto_bubble_capacity)
	{
		AutoBubble* next = &auto_bubbles[rand() % auto_bubble_capacity];
		next->pop_countdown = 0;

		player.current_money =
			player.current_money + (pop_reward_money + 1 * (player.current_base * player.current_multiplier));
		player.addon_multiplier = player.addon_multiplier + pop_reward_multiplier;
		animation_start(&next->pop_animation);
		return nullptr;
	}


	AutoBubble* auto_bubble = &auto_bubbles[auto_bubble_count];
	auto_bubble_count++;

	auto_bubble->pop_countdown = (rand() % 15) + 20;
	auto_bubble->spawned_at = app.now;
	next = auto_bubble;
	next->is_dead = false;
	auto_bubble->bubble.consecutive_clicks = 0;
	auto_bubble->bubble.burst_cap = (rand() % (burst_max - burst_min)) + burst_min;
	auto_bubble->pop_animation.state = BubbleAnimationStateStop;

	int window_width, window_height;
	GetWindowSizeI(app.window, &window_width, &window_height);
	float window_width_half = window_width / 2.0f;
	float window_height_half = window_height / 2.0f;

	auto_bubble->max_radius = 256.0f * 0.3f;
	auto_bubble->min_radius = 228.0f * 0.3f;

	float difference = auto_bubble->max_radius - auto_bubble->min_radius;

	auto_bubble->min_radius = (rand() % 32) + auto_bubble->min_radius;
	auto_bubble->max_radius = auto_bubble->min_radius + difference;
	next->bubble.x = (rand() % int(window_width * 0.8f)) + int(window_width * 0.1f);
	next->bubble.y = (rand() % int(window_height * 0.8f)) + int(window_height * 0.1f);
	for (size_t iterations = 0; iterations < 8; iterations++)
	{
		if (!bubble_bubble_intersection(&player_bubbles->bubble, &next->bubble))
		{
			break;
		}
		next->bubble.x = (rand() % int(window_width * 0.8f)) + int(window_width * 0.1f);
		next->bubble.y = (rand() % int(window_height * 0.8f)) + int(window_height * 0.1f);
	}

	play(Audio::MildPop);
	return next;
}

void main_run()
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

	if (!is_running)
	{
#ifdef __EMSCRIPTEN__
		cleanup();
		emscripten_cancel_main_loop(); /* this should "kill" the app. */
#endif
	}

	// Input
	update(&app.input);

	// Pre-caln
	uint64_t base_pop = player.current_base;
	for (int i = 0; i < player_bubble_count; i++)
	{
		uint32_t archetype_owned = 0;
		for (int j = 0; j < 16; j++)
		{
			if (player_bubbles[i].owned_cosmetics[j])
			{
				archetype_owned |= 1 << j;
			}
		}
		player.current_base = player.current_base + archetype_owned / 2;
	}
	player.current_base = player.current_base + player.addon_base;

	uint64_t multiplier_pop = player.current_multiplier;
	;
	for (int i = 0; i < auto_bubble_count; i++)
	{
		if (!auto_bubbles[i].is_dead)
		{
			player.current_multiplier = player.current_multiplier + auto_bubbles[i].archetype + 1;
		}
	}
	player.current_multiplier = player.current_multiplier + player.addon_multiplier;


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
	update(&app, &player, &simple_duck, player_bubbles, player_bubble_count);
	update(&app, &player, player_bubbles, auto_bubbles, &auto_bubble_count);
	// update(&app, &player, upgrade_bubbles, upgrade_bubble_count);
	update(&app, particles, &particle_count, particle_capacity, player_bubbles, player_bubble_count, auto_bubbles,
		auto_bubble_count);
	// Render

	SDL_SetRenderDrawColor(app.renderer, background_color.r, background_color.g, background_color.b,
		background_color.a);

	SDL_RenderClear(app.renderer);

	// render(&app, upgrade_bubbles, upgrade_bubble_count);
	render(&app, player_bubbles, player_bubble_count);
	render(&app, particles, particle_count);

	SDL_FRect canvas;
	int w, h;
	GetWindowSizeI(app.window, &w, &h);
	canvas.w = w;
	canvas.h = h;
	draw_stack_panel_left(&app, &canvas, &player_ui, &player_bubbles[0], &player.current_money);
	draw_stack_panel(&app, &canvas, &player, &player.current_money);

	render(&app, auto_bubbles, auto_bubble_capacity);

	SDL_RenderPresent(app.renderer);

	// player.current_base = SDL_clamp(player.current_base, 0, 9999);
	// player.current_multiplier = SDL_clamp(player.current_multiplier, 0, 9999);

	{
		{
			uint32_t index = (uint32_t)Upgrade::BubbleDoubbler1;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				player.addon_base = player.addon_base + 1;
			}
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleDoubler2;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					AutoBubble* bubble = spawn_random_auto_bubble(0, 1, 2, 1);
					if (bubble != nullptr)
					{
						bubble->archetype = 0;
					}
				}
			}
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleTripler1;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					int archetype_bias = (rand() % 255) + 1;
					AutoBubble* bubble =
						spawn_random_auto_bubble(archetype_bias, archetype_bias + 2, archetype_bias, archetype_bias);
					if (bubble != nullptr)
					{
						bubble->archetype = uint8_t(archetype_bias);
					}
				}
			}
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleTriple2;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				simple_duck.amount = simple_duck.amount + difference;
				const float upper_limit = 500.0f;

				float frac = (upper_limit - simple_duck.amount) / upper_limit;

				frac = SDL_clamp(frac * simple_duck.amount, 0.0f, 1.0f);

				simple_duck.duration = lerp(0.125f, 5.0f, frac);
				simple_duck.accumulator = SDL_min(simple_duck.accumulator, simple_duck.duration);
			}
		}
		{
			size_t count = SDL_min(premium_duck_count, SDL_arraysize(premium_simple_duck));
			for (int i = 0; i < count; i++)
			{
				PremiumDuck* duck = &premium_simple_duck[i];
				duck->accumulator = duck->accumulator + app.delta_time;
				if (duck->accumulator > 5.0f)
				{
					if (auto_bubble_count > 0)
					{
						play(Audio::Quak);
					}
				}
				while (duck->accumulator > 5.0f)
				{
					// Add a stutter
					duck->accumulator -= 5.0f - (rand() % 10) / 10.0f;
					for (int j = 0; j < duck->amount; j++)
					{
						if (auto_bubble_count > 0)
						{
							size_t random_index = rand() % auto_bubble_count;
							AutoBubble* bubble = &auto_bubbles[random_index];

							int archetype_bias = bubble->archetype;
							player.current_money = player.current_money =
								archetype_bias + (1 * (player.current_base * player.current_multiplier));

							bubble->bubble.consecutive_clicks = bubble->bubble.consecutive_clicks;
							player.current_money =
								player.current_money + (1 * (player.current_base * player.current_multiplier));
							break;
						}
						else
						{
							break;
						}
					}
				}
			}
			uint32_t index = (uint32_t)Upgrade::AutoBubble1;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					size_t index = premium_duck_count % SDL_arraysize(premium_simple_duck);
					premium_simple_duck[index].amount++;
					premium_duck_count++;
				}
			}
		}
		{
			size_t count = SDL_min(luxury_duck_count, SDL_arraysize(luxury_simple_duck));
			for (int i = 0; i < count; i++)
			{
				LuxuryDuck* duck = &luxury_simple_duck[i];
				duck->accumulator = duck->accumulator + app.delta_time;
				if (duck->accumulator > 5.0f)
				{
					if (auto_bubble_count > 0)
					{
						play(Audio::Quak);
					}
				}
				while (duck->accumulator > 5.0f)
				{
					// Add a stutter
					duck->accumulator -= 5.0f - (rand() % 10) / 10.0f;
					for (int j = 0; j < duck->amount; j++)
					{
						if (auto_bubble_count > 0)
						{
							size_t random_index = rand() % auto_bubble_count;
							AutoBubble* bubble = &auto_bubbles[random_index];

							int archetype_bias = bubble->archetype;
							player.current_money =
								player.current_money +
								(bubble->archetype + 1 * (player.current_base * player.current_multiplier));
							player.addon_multiplier = player.addon_multiplier + bubble->archetype;
							animation_start(&bubble->pop_animation);
							break;
						}
						else
						{
							break;
						}
					}
				}
			}
			uint32_t index = (uint32_t)Upgrade::AutoBubble2;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					size_t index = luxury_duck_count % SDL_arraysize(luxury_simple_duck);
					luxury_simple_duck[index].amount++;
					luxury_duck_count++;
				}
			}
		}
		{
			size_t count = SDL_min(simple_bathtub_count, SDL_arraysize(simple_bathtubs));
			for (int i = 0; i < count; i++)
			{
				SimpleBathtubs* tub = &simple_bathtubs[i];
				tub->accumulator = tub->accumulator + app.delta_time;
				while (tub->accumulator > 5.0f)
				{
					// Add a stutter
					tub->accumulator -= 5.0f - (rand() % 10) / 10.0f;
					for (int j = 0; j < tub->amount; j++)
					{
						AutoBubble* bubble = spawn_random_auto_bubble(0, 1, 2, 1);
						if (bubble != nullptr)
						{
							bubble->archetype = 0;
						}
					}
				}
			}

			uint32_t index = (uint32_t)Upgrade::AutoBubble3;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					size_t index = simple_bathtub_count % SDL_arraysize(simple_bathtubs);
					simple_bathtubs[index].amount++;
					simple_bathtub_count++;
				}
			}
		}
		{
			size_t count = SDL_min(luxury_bathtub_count, SDL_arraysize(luxury_bathtub));
			for (int i = 0; i < count; i++)
			{
				LuxuryBathtubs* tub = &luxury_bathtub[i];
				tub->accumulator = tub->accumulator + app.delta_time;
				while (tub->accumulator > 5.0f)
				{
					// Add a stutter
					tub->accumulator -= 5.0f - (rand() % 10) / 10.0f;
					for (int j = 0; j < tub->amount; j++)
					{
						int archetype_bias = (rand() % 255) + 1;
						AutoBubble* bubble = spawn_random_auto_bubble(archetype_bias, archetype_bias + 2,
							archetype_bias, archetype_bias);
						if (bubble != nullptr)
						{
							bubble->archetype = uint8_t(archetype_bias);
						}
					}
				}
			}

			uint32_t index = (uint32_t)Upgrade::AutoBubble4;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{
					size_t index = luxury_bathtub_count % SDL_arraysize(luxury_bathtub);
					luxury_bathtub[index].amount++;
					luxury_bathtub_count++;
				}
			}
		}

		for (int i = 0; i < (int)Upgrade::Count; ++i)
		{
			double cost = UpgradeCosts[i];
			int difference = player.current_upgrade_levels[i] - player.previous_upgrades_levels[i];
			player.addon_multiplier = player.addon_multiplier + (difference * i);
		}

		SDL_memcpy(player.previous_upgrades_levels, player.current_upgrade_levels,
			sizeof(player.current_upgrade_levels));
	}


	post_render_update(&app, &player, player_bubbles, 1, auto_bubbles, auto_bubble_capacity, &player_ui, true);
	post_render_update(&player);

	player.current_base = base_pop;
	player.current_multiplier = multiplier_pop;
}

SDL_EnumerationResult fck_print_directory(void* userdata, const char* dirname, const char* fname)
{
	const char* extension = SDL_strrchr(fname, '.');

	SDL_PathInfo path_info;

	size_t path_count = SDL_strlen(dirname);
	size_t file_name_count = SDL_strlen(fname);
	size_t total_count = file_name_count + path_count + 1;
	char* path = (char*)SDL_malloc(total_count);
	path[0] = '\0';

	size_t last = 0;
	last = SDL_strlcat(path, dirname, total_count);
	last = SDL_strlcat(path, fname, total_count);

	SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s - %s - %s", dirname, fname, extension);
	if (SDL_GetPathInfo(path, &path_info))
	{
		if (path_info.type == SDL_PATHTYPE_DIRECTORY)
		{
			SDL_EnumerateDirectory(path, fck_print_directory, userdata);
		}
	}

	SDL_free(path);

	return SDL_ENUM_CONTINUE;
}


int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

	if (!TTF_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not init TTF: %s\n", SDL_GetError());
		return -1;
	}

	if (!Mix_Init(MIX_INIT_WAVPACK))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not init WAV: %s\n", SDL_GetError());
		return -1;
	}

	app.ui = (UiState*)SDL_malloc(sizeof(UiState));
	SDL_zero(*app.ui);
	app.upgrades = (Upgrades*)SDL_malloc(sizeof(Upgrades));
	SDL_zero(*app.upgrades);

	app.tick_frequency = 0.25f;
	app.window = SDL_CreateWindow("Bubble Clicker", 1920, 1080, SDL_WINDOW_RESIZABLE);
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

	text_engine = TTF_CreateRendererTextEngine(app.renderer);
	player.current_base = 1;
	player.current_multiplier = 1;

	for (int index = 0; index < (int)Upgrade::Count; index++)
	{
		app.upgrades->cost[index] = TTF_CreateText(text_engine, fonts_small[(u64)Font::JuicyFruity], "Cost/Cost", 0);
		app.upgrades->count[index] = TTF_CreateText(text_engine, fonts_small[(u64)Font::JuicyFruity], "123", 0);
		app.upgrades->name[index] =
			TTF_CreateText(text_engine, fonts_med[(u64)Font::JuicyFruity], UpgradeNames[index], 0);
	}
	player_ui.x = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "x", 0);
	player_ui.money = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);
	player_ui.base = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);
	player_ui.multiplier = TTF_CreateText(text_engine, fonts[(u64)Font::JuicyFruity], "0000000", 0);

	setup(&app, player_bubbles, player_bubble_count);
	setup(&app, auto_bubbles, auto_bubble_capacity);
	setup(&app, upgrade_bubbles, upgrade_bubble_count);

	post_render_update(&app, &player, nullptr, 0, nullptr, 0, &player_ui, true);

	is_running = true;
	tp = SDL_GetTicks();

	SDL_EnumerateDirectory(SDL_GetCurrentDirectory(), fck_print_directory, nullptr);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_run, 0, 1);
#else
	while (is_running)
	{
		main_run();
	}
	cleanup();
#endif


	return 0;
}