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

constexpr SDL_Color white = SDL_Color{ 255, 255, 255, 255 };

constexpr size_t player_bubble_count = 1;
constexpr size_t auto_bubble_capacity = 6;
constexpr size_t upgrade_bubble_count = 4;
constexpr size_t particle_capacity = 1024;

App app;
TTF_TextEngine* text_engine;
SinglePlayer player;
SinglePlayerUI player_ui;
PlayerBubble player_bubbles[player_bubble_count];

AutoBubble auto_bubbles[auto_bubble_capacity];
UpgradeBubble upgrade_bubbles[upgrade_bubble_count];

size_t particle_count = 0;
Particle particles[particle_capacity];

bool is_running;
milliseconds tp;

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

	// Fixed Update
	app.now = now / 1000.0f;
	app.delta_time = delta_time / 1000.0f;
	app.tick_accumulator = app.tick_accumulator + app.delta_time;
	while (app.tick_accumulator >= app.tick_frequency)
	{
		app.tick_accumulator = app.tick_accumulator - app.tick_frequency;
		fixed_update(&app, &player, auto_bubbles, auto_bubble_capacity);
	}

	// Update
	update(&app, &player, player_bubbles, player_bubble_count);
	update(&app, &player, player_bubbles, auto_bubbles, auto_bubble_capacity);
	// update(&app, &player, upgrade_bubbles, upgrade_bubble_count);
	update(&app, particles, &particle_count, particle_capacity, player_bubbles, player_bubble_count);
	// Render

	if (key_just_down(&app.input.keyboard, SDL_SCANCODE_X))
	{
		player.current_money = player.current_money << 1;
	}

	SDL_SetRenderDrawColor(app.renderer, background_color.r, background_color.g, background_color.b,
		background_color.a);

	SDL_RenderClear(app.renderer);

	// render(&app, upgrade_bubbles, upgrade_bubble_count);
	render(&app, player_bubbles, player_bubble_count);
	render(&app, auto_bubbles, auto_bubble_capacity);
	render(&app, particles, particle_count);

	SDL_FRect canvas;
	int w, h;
	SDL_GetWindowSize(app.window, &w, &h);
	canvas.w = w;
	canvas.h = h;
	draw_stack_panel_left(&app, &canvas, &player_ui, &player_bubbles[0], &player.current_money);
	draw_stack_panel(&app, &canvas, &player, &player.current_money);

	SDL_RenderPresent(app.renderer);

	// player.current_base = SDL_clamp(player.current_base, 0, 9999);
	// player.current_multiplier = SDL_clamp(player.current_multiplier, 0, 9999);

	{
		{
			uint32_t index = (uint32_t)Upgrade::BubbleDoubbler1;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				player.current_base = player.current_base + difference * 2;
			}
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleDoubler2;
			int difference = player.current_upgrade_levels[index] - player.previous_upgrades_levels[index];
			if (difference > 0)
			{
				for (int i = 0; i < difference; i++)
				{

					AutoBubble* next = nullptr;
					for (size_t j = 0; j < auto_bubble_capacity; j++)
					{
						AutoBubble* auto_bubble = &auto_bubbles[j];
						if (auto_bubble->is_dead)
						{
							auto_bubble->pop_countdown = (rand() % 15) + 20;
							auto_bubble->spawned_at = app.now;
							next = auto_bubble;
							next->is_dead = false;
							auto_bubble->bubble.consecutive_clicks = 0;
							auto_bubble->bubble.burst_cap = (rand() % 10) + 2;
							auto_bubble->pop_animation.state = BubbleAnimationStateStop;
							int window_width, window_height;
							SDL_GetWindowSize(app.window, &window_width, &window_height);
							float window_width_half = window_width / 2.0f;
							float window_height_half = window_height / 2.0f;
							SDL_FRect dst =
								SDL_FRect{ window_width_half, window_height_half, player_bubbles->max_radius * 4.0f,
										  player_bubbles->max_radius * 4.0f };

							next->bubble.x = (rand() % (int)dst.w) + (int)dst.x - (dst.w / 2);
							next->bubble.y = (rand() % (int)dst.h) + (int)dst.y - (dst.h / 2);
							for (size_t iterations = 0; iterations < 8; iterations++)
							{
								if (!bubble_bubble_intersection(&player_bubbles->bubble, &next->bubble))
								{
									break;
								}
								next->bubble.x = (rand() % (int)canvas.w) + (int)canvas.x;
								next->bubble.y = (rand() % (int)canvas.h) + (int)canvas.y;
							}
							break;
						}
					}
					if (next == nullptr)
					{
						next = &auto_bubbles[rand() % auto_bubble_capacity];
						next->pop_countdown = 0;
						animation_start(&next->pop_animation);
					}
				}
			}
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleTripler1;
		}
		{
			uint32_t index = (uint32_t)Upgrade::BubbleTriple2;
		}
		{
			uint32_t index = (uint32_t)Upgrade::AutoBubble1;
		}
		{
			uint32_t index = (uint32_t)Upgrade::AutoBubble2;
		}
		{
			uint32_t index = (uint32_t)Upgrade::AutoBubble3;
		}
		{
			uint32_t index = (uint32_t)Upgrade::AutoBubble4;
		}

		SDL_memcpy(player.previous_upgrades_levels, player.current_upgrade_levels,
			sizeof(player.current_upgrade_levels));
	}

	post_render_update(&app, &player, &player_ui);
	post_render_update(&player);
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
	app.window = SDL_CreateWindow("Bubble Clicker", 1920, 1080, 0);
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

	post_render_update(&app, &player, &player_ui, true);

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