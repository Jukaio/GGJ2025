#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "assets.h"
#include "core.h"


constexpr float UiTabWidthScale = 0.06;
constexpr float UiTabHeightScale = 0.08;

const SDL_Color btn_over_tint = { 200, 200, 180 };
const SDL_Color btn_press_tint = { 100, 100, 120 };

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
	int owned_upgrades[(int)Upgrade::Count];
	Sprite sprite[(int)Upgrade::Count];

	TTF_Text* name[(int)Upgrade::Count];
	TTF_Text* cost[(int)Upgrade::Count];
	TTF_Text* count[(int)Upgrade::Count];
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

bool button(const App* app, const SDL_FRect* btn, Sprite sprite)
{
	const MouseDevice* m = &app->input.mouse;
	SDL_FRect mouse_rect{ m->current.x, m->current.y, 1, 1 };
	SDL_FRect _;
	bool overlap = SDL_GetRectIntersectionFloat(btn, &mouse_rect, &_);

	SDL_Texture* texture = tex[(uint64_t)sprite];
	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };

	if (overlap && button_down(m, 1))
	{
		SDL_SetTextureColorMod(texture, btn_press_tint.r, btn_press_tint.g, btn_press_tint.b);
	}
	else if (overlap)
	{
		SDL_SetTextureColorMod(texture, btn_over_tint.r, btn_over_tint.g, btn_over_tint.b);
	}

	SDL_RenderTexture(app->renderer, texture, &src, btn);


	SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);

	return overlap && button_just_down(m, 1);
}

//void draw_tab_bottom_button(const App* app, const SDL_FRect* canvas)
//{
	//for (int i = 0; i < (int)Tab::Count; ++i)
	//{
	//	const float sep = UiTabWidthScale * canvas->w;
	//	const float off = canvas->w - ((int)Tab::Count * UiTabWidthScale * canvas->w);

	//	Button btn{};
	//	btn.rect.w = UiTabWidthScale * canvas->w;
	//	btn.rect.h = UiTabHeightScale * canvas->h;
	//	btn.rect.x = off + sep * i - (btn.rect.w / 2.0f);
	//	btn.rect.y = canvas->h - btn.rect.h - canvas->h * 0.01f;

	//	if (button(app->renderer, &btn.rect, &app->input.mouse, Sprite::BoxUI))
	//	{
	//		bool prev_state = app->ui->tabs[i].open;
	//		for (auto& tab : app->ui->tabs)
	//		{
	//			tab.open = false;
	//		}
	//		app->ui->tabs[i].open = !prev_state;
	//	}
	//}
//}

//void draw_upgrades_tab(const App* app, UiTab* tab, const SDL_FRect* canvas)
//{
	//const float off = 0.2 * canvas->w;
	//const float width = 0.7 * canvas->w;
	//const float height = 0.7 * canvas->h;

	//SDL_FRect bg;
	//bg.w = width - (off / 2.0f);
	//bg.h = height;
	//bg.x = off;
	//bg.y = off / 4.0f;

	//SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI2];
	//float w, h;
	//SDL_GetTextureSize(texture, &w, &h);
	//SDL_FRect src = SDL_FRect{ 0, 0, w, h };
	//SDL_RenderTexture(app->renderer, texture, &src, &bg);

	//for (int i = 0; i < (int)Upgrade::Count; ++i)
	//{
	//	SDL_FRect btn;
	//	btn.x = bg.x + bg.w * 0.1 + (i > 3) * bg.w * 0.4;
	//	btn.y = bg.y + bg.h * 0.1 + ((i & 0b11) * bg.h * 0.2);
	//	btn.w = bg.w * 0.1;
	//	btn.h = bg.w * 0.1;

	//	if (button(app->renderer, &btn, &app->input.mouse, Sprite::BoxUI2))
	//	{
	//		app->upgrades->owned_upgrades[i] += 1;
	//	}

		//float offset_x = (btn.w * 0.1f) + btn.w;
		//float offset_y = btn.h * 0.25f;
		//if (!TTF_DrawRendererText(app->upgrades->labels[i], btn.x + offset_x, btn.y + offset_y))
		//{
		//	SDL_Log(SDL_GetError());
		//};
//	}
//
//}

//void draw_tabs(const App* app, const SDL_FRect* canvas)
//{
//	for (int i = 0; i < (int)Tab::Count; ++i)
//	{
//		if (app->ui->tabs[i].open)
//		{
//			draw_upgrades_tab(app, &app->ui->tabs[i], canvas);
//		}
//	}
//}

const SDL_Color afford = {120, 255, 120, 255};
const SDL_Color no_afford = {255, 120, 120, 255};

void draw_stack_panel(const App* app, const SDL_FRect* canvas, u64* money)
{
	SDL_FRect btn;
	btn.w = canvas->w * 0.2f;
	btn.h = canvas->h * 0.08f;
	btn.x = canvas->w - btn.w;
	btn.y = canvas->h * 0.3f;

	float off = btn.h + 0.1f * btn.h;

	for (int i = 0; i < (int)Upgrade::Count; ++i)
	{
		double cost = UpgradeCosts[i];
		bool affordable = (double)*money >= cost;
		if (button(app, &btn, app->upgrades->sprite[i]) && affordable)
		{
			*money -= (u64)cost;
			++app->upgrades->owned_upgrades[i];
		}
		char buf[64];
		int len = SDL_snprintf(buf, 64, "Cost : %.0f", cost);

		const SDL_Color* c = affordable ? &afford : &no_afford;
		TTF_SetTextColor(app->upgrades->cost[i], c->r, c->g, c->b, c->a);
		TTF_SetTextString(app->upgrades->cost[i], buf, len);

		len = SDL_snprintf(buf, 64, "%d", app->upgrades->owned_upgrades[i]);
		TTF_SetTextString(app->upgrades->count[i], buf, len);

		TTF_DrawRendererText(app->upgrades->name[i], btn.x + btn.w * 0.1f, btn.y + btn.h * 0.15f);
		TTF_DrawRendererText(app->upgrades->cost[i], btn.x + btn.w * 0.1f, btn.y + btn.h * 0.6f);
		TTF_DrawRendererText(app->upgrades->count[i], btn.x + btn.w * 0.75f, btn.y + btn.h * 0.2f);

		btn.y += off;
	}
}