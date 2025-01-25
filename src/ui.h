#pragma once

#include <SDL3/SDL_render.h>

#include "assets.h"
#include "core.h"


constexpr float UiTabWidthScale = 0.06;
constexpr float UiTabHeightScale = 0.08;

struct Button
{
	SDL_FRect rect;
};

enum class Upgrade
{
	BubbleDoubbler1,
	BubbleDoubler2,
	BubbleTripler1,
	BubbleTriple2,


	AutoBubble1,
	AutoBubble2,
	AutoBubble3,
	AutoBubble4,

	Count
};

const double UpgradeCosts[] =
{
	50.0,
	100.0,
	200.0,
	800.0,

	50,
	100,
	150,
	200
};

struct Upgrades
{
	bool owned_upgrades[(int)Upgrade::Count];
};

enum class Tab
{
	Upgrades,
	Prestige,
	Settings,
	Settings2,
	Count
};

struct UiTab
{
	SDL_Color color;
	bool open = false;
};

struct UiState
{
	SDL_FRect canvas;
	UiTab tabs[(u64)Tab::Count];
};

bool button(SDL_Renderer* rend, const SDL_FRect* btn, const MouseDevice* m, Sprite sprite)
{
	SDL_FRect mouse_rect{m->current.x, m->current.y, 1, 1};
	SDL_FRect _;
	bool overlap = SDL_GetRectIntersectionFloat(btn, &mouse_rect, &_);

	SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };

	if (overlap && button_down(m, 1))
	{
		SDL_SetTextureColorModFloat(texture, 0.5f, 0.5f, 0.5f);
	}
	else if (overlap)
	{
		SDL_SetTextureColorModFloat(texture, 0.7f, 0.7f, 0.7f);
	}

	SDL_RenderTexture(rend, texture, &src, btn);


	SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);

	return overlap && button_just_down(m, 1);
}

void draw_tab_bottom_button(const App* app, const SDL_FRect* canvas)
{
	for (int i = 0; i < (int)Tab::Count; ++i)
	{
		const float off = 0.2 * canvas->w;
		const float sep = 0.2 * canvas->w;

		Button btn{};
		btn.rect.w = UiTabWidthScale * canvas->w;
		btn.rect.h = UiTabHeightScale * canvas->h;
		btn.rect.x = off + sep * i - (btn.rect.w / 2.0f);
		btn.rect.y = canvas->h - btn.rect.h -  canvas->h * 0.01f;

		if (button(app->renderer, &btn.rect, &app->input.mouse, Sprite::BoxUI))
		{
			bool prev_state = app->ui->tabs[i].open;
			for (auto& tab : app->ui->tabs)
			{
				tab.open = false;
			}
			app->ui->tabs[i].open = !prev_state;
		}
	}
}

void draw_upgrades_tab(const App* app, UiTab* tab, const SDL_FRect* canvas)
{
	const float off = 0.2 * canvas->w;
	const float width = 0.7 * canvas->w;
	const float height = 0.7 * canvas->h;

	SDL_FRect bg;
	bg.w = width - (off / 2.0f);
	bg.h = height;
	bg.x = off;
	bg.y = off / 4.0f;

	SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI];
	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };
	SDL_RenderTexture(app->renderer, texture, &src, &bg);



}

void draw_tabs(const App* app, const SDL_FRect* canvas)
{
	for (int i = 0; i < (int)Tab::Count; ++i)
	{
		if (app->ui->tabs[i].open)
		{
			draw_upgrades_tab(app, &app->ui->tabs[i], canvas);
		}
	}
}
