#pragma once

#include <SDL3\SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "gen/fonts.h"
#include "gen/sprites.h"
#include "stb_image.h"


inline SDL_Texture** tex;
inline TTF_Font* fonts[(u64)Font::Count];

struct TexturesExchangeInData
{
	SDL_Renderer* renderer;
	SDL_Surface* queried_surfaces[(u64)Sprite::Count];
	unsigned char* png_source[(u64)Sprite::Count];
};

inline void load_assets_exchange(void* data)
{
	TexturesExchangeInData* in_data = (TexturesExchangeInData*)data;

	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		SDL_Surface* srf = in_data->queried_surfaces[i];
		tex[i] = SDL_CreateTextureFromSurface(in_data->renderer, srf);
		stbi_image_free(in_data->png_source[i]);
		if (tex[i] == nullptr)
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

		int x;
		int y;
		int channels;
		const unsigned char* buf = global_buffer;
		unsigned char* png = stbi_load_from_memory(buf + ref.offset, ref.size, &x, &y, &channels, 4);

		SDL_Surface* srf = SDL_CreateSurfaceFrom(x, y, SDL_PIXELFORMAT_RGBA32, (void*)png, x * 4);
		in_data->png_source[i] = png;
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
			//SDL_IOFromMem

			SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
			TTF_Font* font = TTF_OpenFontIO(stream, true, 42);
			if (font == nullptr)
			{
				SDL_Log("%s", SDL_GetError());
			}
			fonts[i] = font;
		}
	}

	// Load textures
	u64 sprite_count = (u64)Sprite::Count;
	tex = (SDL_Texture**)SDL_malloc(sizeof(SDL_Texture*) * sprite_count);
	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
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