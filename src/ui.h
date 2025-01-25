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
	bool open;
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

void draw_tab_bottom_button(const App* app, const SDL_FRect* canvas, int tab_index)
{
	const float off = 0.2 * canvas->w;
	const float sep = 0.2 * canvas->w;

	Button btn{};
	btn.rect.w = UiTabWidthScale * canvas->w;
	btn.rect.h = UiTabHeightScale * canvas->h;
	btn.rect.x = off + sep * tab_index - (btn.rect.w / 2.0f);
	btn.rect.y = canvas->h - btn.rect.h -  canvas->h * 0.01f;

	if (button(app->renderer, &btn.rect, &app->input.mouse, Sprite::BoxUI))
	{
		bool prev_state = app->ui->tabs[tab_index].open;
		for (auto& tab : app->ui->tabs)
		{
			tab.open = false;
		}
		app->ui->tabs[tab_index].open = !prev_state;
	}
}

