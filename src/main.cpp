#include <SDL_mixer.h>
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "assets.h"
#include "Bouncee.h"
#include "prototypes.h"
#include "types.h"
#include "ui.h"


int main(int argc, char* argv[])
{
	platform_init();
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

	App app{};
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
		app.upgrades->cost[index] = TTF_CreateText(text_engine, fonts_small[(u64)Font::JuicyFruity], "Cost/Cost", 0);
		app.upgrades->count[index] = TTF_CreateText(text_engine, fonts_small[(u64)Font::JuicyFruity], "123", 0);
		app.upgrades->name[index] = TTF_CreateText(text_engine, fonts_med[(u64)Font::JuicyFruity], "Grandma", 0);
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

	setup(&app, player_bubbles, player_bubble_count);
	setup(&app, auto_bubbles, auto_bubble_count);
	setup(&app, upgrade_bubbles, upgrade_bubble_count);

	post_render_update(&app, &player, &player_ui, true);

	bool is_running = true;
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
		draw_stack_panel(&app, &canvas, &player.current_money);

		SDL_RenderPresent(app.renderer);

		post_render_update(&app, &player, &player_ui);
		post_render_update(&player);
	}

	destroy_assets();

	TTF_DestroyRendererTextEngine(text_engine);

	SDL_DestroyRenderer(app.renderer);

	SDL_DestroyWindow(app.window);

	Mix_Quit();

	TTF_Quit();

	SDL_Quit();

	platform_destroy();
	return 0;
}