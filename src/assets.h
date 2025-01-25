#pragma once

#include "gen/audio.h"
#include "gen/fonts.h"
#include "gen/sprites.h"

extern struct SDL_Texture** tex;
extern struct TTF_Font* fonts_small[(uint64_t)Font::Count];
extern struct TTF_Font* fonts_med[(uint64_t)Font::Count];
extern struct TTF_Font* fonts[(uint64_t)Font::Count];
extern struct Mix_Chunk* sounds[(uint64_t)Audio::Count];

#define NO_SPRITE Sprite::Count

void load_assets(struct SDL_Renderer* renderer);

void destroy_assets();