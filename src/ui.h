#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "assets.h"
#include "core.h"


constexpr float UiTabWidthScale = 0.06;
constexpr float UiTabHeightScale = 0.08;

const SDL_Color btn_over_tint = bubble_pink_bright;
const SDL_Color btn_press_tint = bubble_pink;

struct Button
{
	SDL_FRect rect;
};

const Sprite UpgradeIcons[] = { Sprite::UpgradeDeadFish,   Sprite::UpgradeSoapUsed, Sprite::UpgradeDuck,
							   Sprite::UpgradSoap,        Sprite::UpgradeDuckNeon, Sprite::UpgradeBathEmpty,
							   Sprite::UpgradeDuckBetter, Sprite::UpgradeBath };

const char* UpgradeNames[] = { "Dead Fish", "Used Soap",  "Duck",        "Soap",
							  "Neon Duck", "Empty Bath", "Better Duck", "Full Bath" };

const double UpgradeCostsIncrement[] = { 2,      50.0,     200.0,     25000.0,

										50000.0, 50000000.0, 250000000.0, 500000000.0 };

double UpgradeCosts[] = { 50.0, 100.0, 2000.0, 1000000.0, 2500000.0, 250000000.0, 500000000.0, 1000000000.0 };

double UpgradeCostsIncrementScalar[] = { 1.0, 4.0, 2.0, 4.0, 2.0, 4.0, 2.0, 4.0 };

const double SkinCosts[] = { 100.0,   200.0,   500.0,   10000.0,  250000.0,   500000.0,   750000.0,
							10000000.0, 20000000.0, 50000000.0, 700000000.0, 1000000000.0, 2000000000.0, 30000000000.0 };

// Keep in sort!

const Sprite SkinIcons[] = { Sprite::BubbleGlare,     Sprite::BubbleAngel,   Sprite::BubbleDevil,
							Sprite::BubbleGhostEyes, Sprite::BubbleGlasses, Sprite::BubbleWeirdMouth,
							Sprite::BubbleGhost,     Sprite::BubbleDead,    Sprite::BubbleSunglassesPink,
							Sprite::BubbleBow,       Sprite::BubblesTie,    Sprite::BubbleKot,
							Sprite::BubbleCrown };


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

inline bool button(const App* app, const SDL_FRect* btn, Sprite sprite, bool blocked = false)
{
	const MouseDevice* m = &app->input.mouse;
	SDL_FRect mouse_rect{ m->current.x, m->current.y, 1, 1 };
	SDL_FRect _;
	bool overlap = SDL_GetRectIntersectionFloat(btn, &mouse_rect, &_);

	SDL_Texture* texture = tex[(uint64_t)sprite];
	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };

	if (!blocked)
	{
		if (overlap && button_down(m, 1))
		{
			SDL_SetTextureColorMod(texture, btn_press_tint.r, btn_press_tint.g, btn_press_tint.b);
		}
		else if (overlap)
		{
			SDL_SetTextureColorMod(texture, btn_over_tint.r, btn_over_tint.g, btn_over_tint.b);
		}
	}
	else {
		SDL_SetTextureColorMod(texture, 200, 200, 200);
	}
	SDL_RenderTexture9Grid(app->renderer, texture, &src, 128.0f, 128.0f, 128.0f, 128.0f, 0.16f, btn);

	SDL_SetTextureColorModFloat(texture, 1.0f, 1.0f, 1.0f);

	return overlap && button_just_down(m, 1);
}

// void draw_tab_bottom_button(const App* app, const SDL_FRect* canvas)
//{
// for (int i = 0; i < (int)Tab::Count; ++i)
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

// void draw_upgrades_tab(const App* app, UiTab* tab, const SDL_FRect* canvas)
//{
// const float off = 0.2 * canvas->w;
// const float width = 0.7 * canvas->w;
// const float height = 0.7 * canvas->h;

// SDL_FRect bg;
// bg.w = width - (off / 2.0f);
// bg.h = height;
// bg.x = off;
// bg.y = off / 4.0f;

// SDL_Texture* texture = tex[(uint64_t)Sprite::BoxUI2];
// float w, h;
// SDL_GetTextureSize(texture, &w, &h);
// SDL_FRect src = SDL_FRect{ 0, 0, w, h };
// SDL_RenderTexture(app->renderer, texture, &src, &bg);

// for (int i = 0; i < (int)Upgrade::Count; ++i)
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

// float offset_x = (btn.w * 0.1f) + btn.w;
// float offset_y = btn.h * 0.25f;
// if (!TTF_DrawRendererText(app->upgrades->labels[i], btn.x + offset_x, btn.y + offset_y))
//{
//	SDL_Log(SDL_GetError());
// };
//	}
//
//}

// void draw_tabs(const App* app, const SDL_FRect* canvas)
//{
//	for (int i = 0; i < (int)Tab::Count; ++i)
//	{
//		if (app->ui->tabs[i].open)
//		{
//			draw_upgrades_tab(app, &app->ui->tabs[i], canvas);
//		}
//	}
// }

const SDL_Color afford = { 120, 255, 120, 255 };
const SDL_Color no_afford = { 255, 120, 120, 255 };

inline bool is_bit_set(u64 value, int bit)
{
	u64 mask = (1 << bit);
	return value & mask;
}

inline void flip_bit(u64* value, int bit)
{
	u64 mask = (1 << bit);
	*value = *value ^ mask;
}

inline void draw_wallet(const App* app, const SDL_FRect* canvas, SinglePlayerUI* player_ui)
{
	SDL_Texture* texture = tex[(u64)Sprite::BoxUI];
	float w, h;
	SDL_GetTextureSize(texture, &w, &h);
	SDL_FRect src = SDL_FRect{ 0, 0, w, h };

	float half_height = canvas->h * 0.5f;
	SDL_FRect top = *canvas;
	top.h = half_height;
	top.y = canvas->y - (top.h * 0.5f) - half_height * 0.15f;

	SDL_FRect bottom = *canvas;
	bottom.h = half_height;
	bottom.y = canvas->y + (bottom.h * 0.5f);

	float half_width = bottom.w * 0.5f;
	SDL_FRect left = bottom;
	left.w = half_width;
	left.x = canvas->x;

	SDL_FRect right = bottom;
	right.w = half_width;
	right.x = canvas->x + half_width;

	float bottom_height = 128.0f * 0.5f;
	SDL_RenderTexture9Grid(app->renderer, texture, &src, bottom_height, bottom_height, bottom_height, bottom_height, 0.16f, &top);
	SDL_RenderTexture9Grid(app->renderer, texture, &src, bottom_height, bottom_height, bottom_height, bottom_height, 0.16f, &bottom);

	int measured_width;
	size_t measured_length;
	TTF_Font* font = TTF_GetTextFont(player_ui->money);
	float font_height = TTF_GetFontHeight(font);
	float font_height_half = font_height * 0.5f;
	float offset = font_height * 0.5f;
	TTF_MeasureString(TTF_GetTextFont(player_ui->money), player_ui->money->text, 0, 0, &measured_width,
		&measured_length);
	TTF_DrawRendererText(player_ui->money, top.x + (top.w * 0.5f) - (measured_width * 0.5f),
		top.y + (top.h * 0.5) - offset);

	font = TTF_GetTextFont(player_ui->base);
	font_height = TTF_GetFontHeight(font);
	font_height_half = font_height * 0.5f;
	offset = font_height * 0.5f;
	TTF_MeasureString(font, player_ui->base->text, 0, 0, &measured_width, &measured_length);
	TTF_DrawRendererText(player_ui->base, left.x + (left.w * 0.55f) - (measured_width * 0.5f),
		left.y + (left.h * 0.5) - offset);

	font = TTF_GetTextFont(player_ui->multiplier);
	font_height = TTF_GetFontHeight(font);
	font_height_half = font_height * 0.5f;
	offset = font_height * 0.5f;
	TTF_MeasureString(TTF_GetTextFont(player_ui->multiplier), player_ui->multiplier->text, 0, 0, &measured_width,
		&measured_length);
	TTF_DrawRendererText(player_ui->multiplier, right.x + (right.w * 0.50f) - (measured_width * 0.5f),
		right.y + (right.h * 0.5) - offset);

	TTF_DrawRendererText(player_ui->x, bottom.x + (bottom.w * 0.46f), bottom.y + (bottom.h * 0.5f));
}

inline void draw_stack_panel_left(const App* app,
	const SDL_FRect* canvas,
	SinglePlayerUI* player_ui,
	PlayerBubble* bub,
	u64* money)
{
	SDL_FRect btn;
	btn.w = canvas->w * 0.06f;
	btn.h = btn.w;
	btn.x = canvas->h * 0.04f;
	btn.y = canvas->h * 0.04f;

	const int num = SDL_arraysize(SkinIcons);

	float off = btn.h + 0.15f * btn.h;

	for (int i = 0; i < num; ++i)
	{
		double cost = SkinCosts[i];
		bool affordable = (double)*money >= cost || bub->owned_cosmetics[i];
		bool will_buy = affordable && bub->owned_cosmetics[i] == false;

		if (button(app, &btn, Sprite::BoxUI, !affordable))
		{
			if (will_buy)
			{
				bub->owned_cosmetics[i] = true;
				play(Audio::PopUI);
				flip_bit(&bub->archetype, i);
				*money -= (u64)cost;
			}
			else if (bub->owned_cosmetics[i])
			{
				play(Audio::PopUI);
				flip_bit(&bub->archetype, i);
			}
		}


		char buf[64];
		int len = SDL_snprintf(buf, 64, "Cost : %.0f", cost);

		SDL_FRect dst_icon = SDL_FRect{ btn.x + btn.w * 0.2f, btn.y + btn.h * 0.2f, btn.w * 0.6f, btn.h * 0.6f };
		if (affordable)
		{
			const SDL_Color* c = affordable ? &afford : &no_afford;
			// TTF_SetTextColor(app->upgrades->cost[i], c->r, c->g, c->b, c->a);
			// TTF_SetTextString(app->upgrades->cost[i], buf, len);
			{
				float w, h;
				SDL_Texture* texture = tex[(uint64_t)SkinIcons[i]];
				SDL_SetTextureColorMod(texture, 255, 255, 255);
				SDL_SetTextureAlphaMod(texture, 255);
				SDL_GetTextureSize(texture, &w, &h);
				SDL_FRect src = SDL_FRect{ 0, 0, w, h };
				SDL_RenderTexture(app->renderer, texture, &src, &dst_icon);
			}
		}
		if (!bub->owned_cosmetics[i])
		{
			SDL_Texture* lock = tex[(u64)Sprite::Lock];
			float w, h;
			SDL_GetTextureSize(lock, &w, &h);
			SDL_FRect src = SDL_FRect{ 0, 0, w, h };
			if (!affordable)
			{
				SDL_SetTextureAlphaMod(lock, 128);
			}
			SDL_RenderTexture(app->renderer, lock, &src, &dst_icon);
			if (!affordable)
			{
				SDL_SetTextureAlphaMod(lock, 255);
			}
		}

		if ((i % 3) == 2)
		{
			btn.x -= off * 2;
			btn.y += off;
		}
		else if ((i % 3) == 1)
		{
			btn.x += off;
		}
		else
		{
			btn.x += off;
		}
	}

	{
		float height = canvas->h * 0.25f;
		float half_height = height * 0.5f;
		float width = off * 3.0f;
		float half_width = width * 0.25f;
		SDL_FRect wallet_container = { off - half_width, canvas->h - off - half_height, width, height };

		draw_wallet(app, &wallet_container, player_ui);
	}
}

inline void draw_stack_panel(const App* app, const SDL_FRect* canvas, SinglePlayer* player, u64* money)
{
	SDL_FRect btn;
	btn.w = canvas->w * 0.2f;
	btn.h = canvas->h * 0.1f;
	btn.x = canvas->w - btn.w - btn.w * 0.08f;
	btn.y = canvas->h * 0.04f;

	float off = btn.h + 0.15f * btn.h;

	for (int i = 0; i < (int)Upgrade::Count; ++i)
	{
		double cost = UpgradeCosts[i];
		bool affordable = (double)*money >= cost;
		if (button(app, &btn, Sprite::BoxUI))
		{
			if (affordable)
			{
				play(Audio::PopUI);
				*money -= (u64)cost;
				++app->upgrades->owned_upgrades[i];
				cost = UpgradeCosts[i] = UpgradeCostsIncrement[i] + (UpgradeCosts[i] * UpgradeCostsIncrementScalar[i]);
			}
			else {}
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

		SDL_FRect dstIcon = SDL_FRect{ btn.x + btn.w * 0.75f, btn.y + btn.h * 0.1f, btn.h, btn.h };
		float w, h;
		SDL_Texture* texture = tex[(uint64_t)UpgradeIcons[i]];
		SDL_GetTextureSize(*tex, &w, &h);
		SDL_FRect src = SDL_FRect{ 0, 0, w * 2.0f, h * 2.0f };

		SDL_RenderTexture(app->renderer, texture, &src, &dstIcon);

		btn.y += off;
	}

	SDL_memcpy(player->current_upgrade_levels, app->upgrades->owned_upgrades, sizeof(player->current_upgrade_levels));
}