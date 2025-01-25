#pragma once

#include "assets.h"
#include <SDL3/SDL_stdinc.h>

void setup(struct App* app, struct PlayerBubble* player_bubbles, size_t count);
void setup(struct App* app, struct AutoBubble* auto_bubbles, size_t auto_bubble_count);
void setup(struct App* app, struct UpgradeBubble* upgrade_bubbles, size_t auto_bubble_count);


void update(const struct App* app,
	struct Particle* particles,
	size_t* particle_count,
	size_t particle_capacity,
	struct PlayerBubble* player_bubbles,
	size_t player_count);
void update(struct App* app, struct SinglePlayer* player, struct PlayerBubble* player_bubbles, size_t count);
void update(struct App* app, struct SinglePlayer* player, struct AutoBubble* bubbles, size_t count);
void update(struct App* app, struct SinglePlayer* player, struct UpgradeBubble* bubbles, size_t count);
void fixed_update(struct App* app, struct SinglePlayer* player, struct AutoBubble* bubbles, size_t count);
void post_render_update(struct SinglePlayer* player);
void post_render_update(struct App* app, struct SinglePlayer* player, struct SinglePlayerUI* ui, bool force = false);

void render(struct App* app, const struct Bubble* bubble, Sprite sprite, float offset_x = 0.0f, float offset_y = 0.0f, bool recolor = true);
void render(struct App* app, struct PlayerBubble* bubbles, size_t count);
void render(struct App* app, struct AutoBubble* bubbles, size_t count);
void render(struct App* app, struct Particle* particles, size_t count);
void render(struct App* app, struct UpgradeBubble* bubbles, size_t count);
void render(struct App* app, struct SinglePlayerUI* ui);