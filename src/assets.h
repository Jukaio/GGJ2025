#pragma once

#include <SDL_image.h>
#include <SDL3\SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "gen/fonts.h"
#include "gen/sprites.h"


inline SDL_Texture** tex;
inline TTF_Font* fonts_small[(u64)Font::Count];
inline TTF_Font* fonts_med[(u64)Font::Count];
inline TTF_Font* fonts[(u64)Font::Count];

#define NO_SPRITE Sprite::Count

struct TexturesExchangeInData
{
	SDL_Renderer* renderer;
	SDL_Surface* queried_surfaces[(u64)Sprite::Count];
	SDL_IOStream* png_source[(u64)Sprite::Count];
};

inline void load_assets_exchange(void* data)
{
	TexturesExchangeInData* in_data = (TexturesExchangeInData*)data;

	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		SDL_Surface* srf = in_data->queried_surfaces[i];

		SDL_DestroyTexture(tex[i]);

		tex[i] = SDL_CreateTextureFromSurface(in_data->renderer, srf);

		if (!SDL_CloseIO(in_data->png_source[i]) || tex[i] == nullptr)
		{
			SDL_Log("%s", SDL_GetError());
		}

		if (!SDL_SetTextureScaleMode(tex[i], SDL_SCALEMODE_LINEAR))
		{
			SDL_Log("%s", SDL_GetError());
		}

		SDL_DestroySurface(srf);
	}
}

inline int load_assets_from_gen(void* data)
{
	SDL_Renderer* renderer = (SDL_Renderer*)data;
	u64 sprite_count = (u64)Sprite::Count;


	size_t size;
	// Maybe free later
	unsigned char* global_buffer = (unsigned char*)SDL_LoadFile(g_sprite_path, &size);
	if (global_buffer == nullptr)
	{
		return -1;
	}

	TexturesExchangeInData* in_data = (TexturesExchangeInData*)SDL_malloc(sizeof(TexturesExchangeInData));
	if (in_data == nullptr)
	{
		return -1;
	}
	in_data->renderer = renderer;


	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		AssetRef ref = g_sprite_offsets[i];

		unsigned char* buf = global_buffer;
		void* begin = buf + ref.offset;

		SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
		SDL_Surface* srf = IMG_LoadPNG_IO(stream);

		in_data->png_source[i] = stream;
		in_data->queried_surfaces[i] = srf;
	}

	SDL_RunOnMainThread(load_assets_exchange, in_data, true);

	SDL_free(in_data);

	return 0;
}

inline void load_assets(SDL_Renderer* renderer)
{
	// Load fonts
	{
		size_t size;
		unsigned char* global_buffer = (unsigned char*)SDL_LoadFile(g_font_path, &size);
		u64 font_count = (u64)Font::Count;
		for (u64 i = 0; i < (u64)Font::Count; ++i)
		{
			AssetRef ref = g_font_offsets[i];

			unsigned char* buf = global_buffer;
			void* begin = buf + ref.offset;

			SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
			TTF_Font* font = TTF_OpenFontIO(stream, false, 42);
			TTF_Font* font_m = TTF_OpenFontIO(stream, false, 28);
			TTF_Font* font_s = TTF_OpenFontIO(stream, true, 18);

			if (font == nullptr)
			{
				SDL_Log("%s", SDL_GetError());
			}
			fonts[i] = font;
			fonts_med[i] = font_m;
			fonts_small[i] = font_s;
		}
	}

	// Load textures
	u64 sprite_count = (u64)Sprite::Count + 1; // Empty texture in count
	tex = (SDL_Texture**)SDL_malloc(sizeof(SDL_Texture*) * sprite_count);
	for (u64 i = 0; i <= (u64)Sprite::Count; ++i)
	{
		tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 0, 0);
	}
	SDL_Thread* thread = SDL_CreateThread(load_assets_from_gen, "Asset Loader", renderer);
}

inline void destroy_assets()
{
	// Unload fonts
	{
		size_t size;
		u64 font_count = (u64)Font::Count;
		for (u64 i = 0; i < (u64)Font::Count; ++i)
		{
			TTF_CloseFont(fonts[i]);
		}
	}

	// Unload textures
	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		SDL_DestroyTexture(tex[i]);
	}
	SDL_free(tex);
}