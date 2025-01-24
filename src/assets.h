#pragma once

#include "gen/sprites.h"
#include "stb_image.h"
#include <SDL3\SDL_surface.h>

inline SDL_Texture** tex;

inline void load_assets(SDL_Renderer* renderer) 
{
	u64 sprite_count = (u64)Sprite::Count;

	tex = (SDL_Texture**)malloc(sizeof(void*) * sprite_count);

	for(u64 i = 0; i < (u64)Sprite::Count; ++i) 
	{
		int x;
		int y;
		int channels;
		AssetRef ref = g_spriteOffsets[i];

		const unsigned char* buf = g_spriteData;
		const unsigned char* png = stbi_load_from_memory(buf + ref.offset, ref.size, &x, &y, &channels, 4);
		
		SDL_Surface* srf = SDL_CreateSurfaceFrom(x, y, SDL_PIXELFORMAT_RGBA32, (void*)png, x*4);
		tex[i] = SDL_CreateTextureFromSurface(renderer,  srf);
		SDL_DestroySurface(srf);
	}
}