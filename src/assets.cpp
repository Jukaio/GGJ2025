
#include "assets.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

SDL_Texture* tex[(uint64_t)Sprite::Count + 1];
TTF_Font* fonts_tiny[(uint64_t)Font::Count];
TTF_Font* fonts_small[(uint64_t)Font::Count];
TTF_Font* fonts_med[(uint64_t)Font::Count];
TTF_Font* fonts[(uint64_t)Font::Count];
Mix_Chunk* sounds[(uint64_t)Audio::Count];

struct TexturesExchangeInData
{
	SDL_Renderer* renderer;
	SDL_Surface* queried_surfaces[(u64)Sprite::Count];
	SDL_IOStream* png_source[(u64)Sprite::Count];
};

void load_assets_exchange(void* data)
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

int load_assets_from_gen(void* data)
{
	SDL_Renderer* renderer = (SDL_Renderer*)data;
	u64 sprite_count = (u64)Sprite::Count;


	// Maybe free later
#ifdef EMBED
	const unsigned char* global_buffer = g_sprite_bin;
#else
	size_t size;
	unsigned char* global_buffer = (unsigned char*)SDL_LoadFile(g_sprite_path, &size);
	if (global_buffer == nullptr)
	{
		return -1;
	}
#endif

	TexturesExchangeInData* in_data = (TexturesExchangeInData*)SDL_malloc(sizeof(TexturesExchangeInData));
	if (in_data == nullptr)
	{
		return -1;
	}
	in_data->renderer = renderer;


	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		AssetRef ref = g_sprite_offsets[i];

		const unsigned char* buf = global_buffer;
		const void* begin = buf + ref.offset;

		SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
		SDL_Surface* srf = IMG_LoadPNG_IO(stream);

		// To reduce memory stuff
		// SDL_Surface* new_surface = SDL_ScaleSurface(srf, 256, 256, SDL_SCALEMODE_LINEAR);
		// if (new_surface != nullptr)
		//{
		//	SDL_DestroySurface(srf);

		//	srf = new_surface;
		//}
		in_data->png_source[i] = stream;
		in_data->queried_surfaces[i] = srf;
	}

	SDL_RunOnMainThread(load_assets_exchange, in_data, true);

	SDL_free(in_data);

	return 0;
}

void load_assets(SDL_Renderer* renderer)
{
	// Load fonts
	{
#ifdef EMBED
		const unsigned char* global_buffer = g_font_bin;
#else
		size_t size;
		unsigned char* global_buffer = (unsigned char*)SDL_LoadFile(g_font_path, &size);
#endif
		u64 font_count = (u64)Font::Count;
		for (u64 i = 0; i < (u64)Font::Count; ++i)
		{
			AssetRef ref = g_font_offsets[i];

			const unsigned char* buf = global_buffer;
			const void* begin = buf + ref.offset;

			SDL_PropertiesID proerties = SDL_CreateProperties();

			SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
			float fontScale = 0.75f;
			TTF_Font* font = TTF_OpenFontIO(stream, false, 42 * fontScale);
			TTF_Font* font_m = TTF_OpenFontIO(stream, false, 28 * fontScale);
			TTF_Font* font_s = TTF_OpenFontIO(stream, true, 18 * fontScale);
			TTF_Font* font_xs = TTF_OpenFontIO(stream, true, 12 * fontScale);

			if (font == nullptr)
			{
				SDL_Log("%s", SDL_GetError());
			}
			fonts[i] = font;
			fonts_med[i] = font_m;
			fonts_small[i] = font_s;
			fonts_tiny[i] = font_xs;
		}
	}

	// Load Audio
	{
		SDL_AudioSpec spec;
		spec.freq = MIX_DEFAULT_FREQUENCY;
		spec.format = MIX_DEFAULT_FORMAT;
		spec.channels = MIX_DEFAULT_CHANNELS;

		if (!Mix_OpenAudio(0, &spec))
		{
			SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
		}
		else
		{
			Mix_QuerySpec(&spec.freq, &spec.format, &spec.channels);
			SDL_Log("Opened audio at %d Hz %d bit%s %s", spec.freq, (spec.format & 0xFF),
				(SDL_AUDIO_ISFLOAT(spec.format) ? " (float)" : ""),
				(spec.channels > 2) ? "surround" :
				(spec.channels > 1) ? "stereo" :
				"mono");
		}

#ifdef EMBED
		const unsigned char* global_buffer = g_audio_bin;
#else
		size_t size;
		const unsigned char* global_buffer = (unsigned char*)SDL_LoadFile(g_audio_path, &size);
#endif
		u64 audio_count = (u64)Audio::Count;
		for (u64 i = 0; i < (u64)Audio::Count; ++i)
		{
			AssetRef ref = g_audio_offsets[i];

			const unsigned char* buf = global_buffer;
			const void* begin = buf + ref.offset;

			SDL_IOStream* stream = SDL_IOFromConstMem(begin, ref.size);
			Mix_Chunk* chunk = Mix_LoadWAV_IO(stream, true);

			if (chunk == nullptr)
			{
				SDL_Log("%s", SDL_GetError());
			}
			sounds[i] = chunk;
		}
	}

	// Load textures
	u64 sprite_count = (u64)Sprite::Count + 1; // Empty texture in count
	for (u64 i = 0; i <= (u64)Sprite::Count; ++i)
	{
		tex[i] = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, 0, 0);
	}

#ifdef __EMSCRIPTEN__
	load_assets_from_gen(renderer);
#else
	SDL_Thread* thread = SDL_CreateThread(load_assets_from_gen, "Asset Loader", renderer);
#endif
}

void destroy_assets()
{
	// Unload fonts
	{
		u64 font_count = (u64)Font::Count;
		for (u64 i = 0; i < font_count; ++i)
		{
			//TTF_CloseFont(fonts[i]);
			//TTF_CloseFont(fonts_med[i]);
			//TTF_CloseFont(fonts_small[i]);
			//TTF_CloseFont(fonts_tiny[i]);
		}
	}

	// Unload textures
	for (u64 i = 0; i < (u64)Sprite::Count; ++i)
	{
		SDL_DestroyTexture(tex[i]);
	}
}