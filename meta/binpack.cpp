#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

const char* asset_root = "assets/";
const char* src_root = "src/";

using u64 = unsigned long long;
using u32 = unsigned int;
using u8 = unsigned char;

struct Buffer
{
	char* mem;
	u64 size;
};

void free_buffer(Buffer* buffer)
{
	free(buffer->mem);
	buffer->mem = nullptr;
	buffer->size = 0;
}

Buffer alloc(u64 size)
{
	Buffer buffer;
	buffer.mem = (char*)malloc(size);
	buffer.size = size;

	return buffer;
}

Buffer read_file(const char* path)
{
	FILE* file = fopen(path, "rb");
	if (!file)
	{
		printf("Failed to open file: %s\n", path);
		return Buffer{ nullptr, 0 };
	}

	fseek(file, 0, SEEK_END);
	u64 size = ftell(file);
	fseek(file, 0, SEEK_SET);

	Buffer buffer = alloc(size);
	if (!buffer.mem)
	{
		fclose(file);
		return Buffer{ nullptr, 0 };
	}

	u64 bytes_read = fread(buffer.mem, 1, size, file);
	fclose(file);

	if (bytes_read != size)
	{
		free_buffer(&buffer);
		printf("Failed to read file: %s\n", path);
		return Buffer{ nullptr, 0 };
	}

	return buffer;
}

struct Asset
{
	std::string name;
	Buffer buffer;
};

struct Assets
{
	std::vector<Asset> images;
	std::vector<Asset> fonts;
	std::vector<Asset> wave;
};

void load_assets(Assets* inout_assets)
{
	std::filesystem::path asset_path(asset_root);

	if (!std::filesystem::exists(asset_path))
	{
		printf("Images directory not found at: %s\n", asset_path.string().c_str());
		return;
	}

	for (const auto& entry : std::filesystem::directory_iterator(asset_path))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

		Buffer buffer = read_file(entry.path().string().c_str());
		if (buffer.mem)
		{
			Asset* asset = nullptr;
			if (entry.path().extension() == ".png")
			{
				asset = &inout_assets->images.emplace_back();
			}

			if (entry.path().extension() == ".ttf")
			{
				asset = &inout_assets->fonts.emplace_back();
			}

			if (entry.path().extension() == ".wav")
			{
				asset = &inout_assets->wave.emplace_back();
			}

			if (asset)
			{
				asset->name = entry.path().filename().stem().string();
				asset->name.erase(std::remove(asset->name.begin(), asset->name.end(), ' '), asset->name.end());
				asset->name.erase(std::remove(asset->name.begin(), asset->name.end(), '-'), asset->name.end());
				asset->buffer = buffer;
				printf("Loaded asset: %s with size %lld\n", entry.path().filename().string().c_str(), asset->buffer.size);
			}
		}
	}
}

struct AssetRef
{
	u64 offset;
	u64 size;
};

void write_asset_array_start(std::string* s, const char* name)
{
	s->append("constexpr AssetRef ");
	s->append(name);
	s->append("[]\n{\n");
}

void write_asset_enum_start(std::string* s, const char* name)
{
	s->append("enum class ");
	s->append(name);
	s->append("\n{\n");
}

void write_asset_array_elem(std::string* s, const Asset* asset, u64* current_offset)
{
	char buf[256];
	sprintf(buf, "\t{%lld, %lld},\n", *current_offset, asset->buffer.size);
	printf("%s", asset->name.c_str());
	*current_offset += asset->buffer.size;
	s->append(buf);
}

void write_asset_enum_elem(std::string* s, const Asset* asset)
{
	char buf[256];
	sprintf(buf, "\t%s,\n", asset->name.c_str());
	s->append(buf);
}

void write_asset_array_end(std::string* s)
{
	s->append("};\n\n");
}

void write_asset_enum_end(std::string* s)
{
	s->append("\tCount\n");
	s->append("};\n\n");
}

void write_asset_array(std::string* s, const std::vector<Asset>* assets, const char* names)
{
	write_asset_array_start(s, names);
	u64 offset = 0;
	for (const auto& asset : *assets)
	{
		write_asset_array_elem(s, &asset, &offset);
	}
	write_asset_array_end(s);
}

void write_asset_enum(std::string* s, const std::vector<Asset>* assets, const char* name)
{
	write_asset_enum_start(s, name);
	for (const auto& asset : *assets)
	{
		write_asset_enum_elem(s, &asset);
	}
	write_asset_enum_end(s);
}

void write_asset_bin_path(std::string* s, const char* name, const char* path)
{
	s->append("constexpr char* g_");

	std::string lower_name = name;
	for (char& c : lower_name)
	{
		c = tolower(c);
	}

	s->append(lower_name);
	s->append("_path = \"");
	s->append(path);
	s->append("\";\n");
}

void write_asset_type(std::string* s, const std::vector<Asset>* assets, const char* name, const char* offset_name, const char* path)
{
	write_asset_enum(s, assets, name);
	write_asset_array(s, assets, offset_name);
	write_asset_bin_path(s, name, path);
}



std::string generate_header(const std::vector<Asset>* assets, const char* name, const char* offsets, const char* path)
{
	std::string buf;
	buf.reserve(1024);

	buf.append("#pragma once \n");
	buf.append("\n");
	buf.append("// GENERATED FILE\n\n");
	buf.append("#include \"core.h\"\n");
	// buf.append("using u64 = unsigned long long;\n\n");
	// buf.append("struct AssetRef \n{\n");
	// buf.append("\tu64 offset;\n");
	// buf.append("\tu64 size;\n");
	// buf.append("};\n");

	buf.append("\n");

	u64 offset = 0;

	write_asset_type(&buf, assets, name, offsets, path);

	return buf;
}

void write_asset_bin(const std::vector<Asset>* assets, const char* target_path)
{
	FILE* file = fopen(target_path, "wb");
	if (!file)
	{
		printf("Failed to open file for writing: %s\n", target_path);
		return;
	}

	for (const auto& asset : *assets)
	{
		fwrite(asset.buffer.mem, 1, asset.buffer.size, file);
	}

	fclose(file);
}

std::string generate_source(const Assets* assets, const char* header_name, const char* data_name)
{
	std::string source;

	source.append("#include \"");
	source.append(header_name);
	source.append("\"\n\n");
	source.append("const unsigned char ");
	source.append(data_name);
	source.append("[] = { ");

	for (const auto& asset : assets->images)
	{
		for (u64 i = 0; i < asset.buffer.size; i++)
		{
			char buf[8];
			sprintf(buf, "0x%02x, ", (unsigned char)asset.buffer.mem[i]);
			source.append(buf);
		}
	}

	source.append("};\n");

	return source;
}

void write_source_file(const std::string& content, const char* path)
{
	FILE* file = fopen(path, "w");
	if (!file) {
		printf("Failed to open file for writing: %s\n", path);
		return;
	}

	fwrite(content.c_str(), 1, content.size(), file);
	fclose(file);
}

void create_directory(const char* path)
{
	std::filesystem::path dir_path(path);
	if (!std::filesystem::exists(dir_path))
	{
		std::filesystem::create_directories(dir_path);
		printf("Created directory: %s\n", path);
	}
}

u64 murmur3_64(const void* key, size_t len, u64 seed = 0)
{
	const u64 m = 0xc6a4a7935bd1e995ULL;
	const int r = 47;

	u64 h = seed ^ (len * m);

	const u64* data = (const u64*)key;
	const u64* end = data + (len / 8);

	while (data != end)
	{
		u64 k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	const unsigned char* data2 = (const unsigned char*)data;

	switch (len & 7)
	{
	case 7: h ^= u64(data2[6]) << 48; [[fallthrough]];
	case 6: h ^= u64(data2[5]) << 40; [[fallthrough]];
	case 5: h ^= u64(data2[4]) << 32; [[fallthrough]];
	case 4: h ^= u64(data2[3]) << 24; [[fallthrough]];
	case 3: h ^= u64(data2[2]) << 16; [[fallthrough]];
	case 2: h ^= u64(data2[1]) << 8; [[fallthrough]];
	case 1: h ^= u64(data2[0]);
		h *= m;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}

u64 check_assets()
{
	u64 hash = 0;
	std::filesystem::path asset_path(asset_root);

	if (!std::filesystem::exists(asset_path))
	{
		return hash;
	}

	for (const auto& entry : std::filesystem::directory_iterator(asset_path))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

		std::string filename = entry.path().filename().string();
		hash = murmur3_64(filename.c_str(), filename.size(), hash);

		auto ftime = std::filesystem::last_write_time(entry.path());
		auto duration = ftime.time_since_epoch();
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		hash = murmur3_64(&seconds, sizeof(seconds), hash);
	}

	return hash;
}

bool should_rebuild()
{
	u64 current_hash = check_assets();

	// Try to read previous hash
	FILE* state_file = fopen("meta/.state", "rb");
	if (!state_file)
	{
		// No previous state, write current hash and rebuild
		state_file = fopen("meta/.state", "wb");
		if (state_file)
		{
			fwrite(&current_hash, sizeof(current_hash), 1, state_file);
			fclose(state_file);
		}
		return true;
	}

	// Read previous hash and compare
	u64 previous_hash;
	size_t read = fread(&previous_hash, sizeof(previous_hash), 1, state_file);
	fclose(state_file);

	if (read != 1 || previous_hash != current_hash)
	{
		// Hash changed or read failed, write new hash and rebuild
		state_file = fopen("meta/.state", "wb");
		if (state_file)
		{
			fwrite(&current_hash, sizeof(current_hash), 1, state_file);
			fclose(state_file);
		}
		return true;
	}

	return false;
}

int main()
{
	Assets assets;

	if (!should_rebuild())
	{
		printf("Assets up to date, skipping rebuild\n");
		return 0;
	}

	load_assets(&assets);

	printf("Loaded %zu images\n", assets.images.size());
	create_directory("assets/gen");
	create_directory("src/gen");

	write_source_file(generate_header(&assets.images, "Sprite", "g_sprite_offsets", "assets/gen/sprites.bin"), "src/gen/sprites.h");
	write_asset_bin(&assets.images, "assets/gen/sprites.bin");

	write_source_file(generate_header(&assets.fonts, "Font", "g_font_offsets", "assets/gen/fonts.bin"), "src/gen/fonts.h");
	write_asset_bin(&assets.fonts, "assets/gen/fonts.bin");

	write_source_file(generate_header(&assets.wave, "Audio", "g_audio_offsets", "assets/gen/audio.bin"), "src/gen/audio.h");
	write_asset_bin(&assets.wave, "assets/gen/audio.bin");

	for (auto& img : assets.images)
	{
		free_buffer(&img.buffer);
	}

	for (auto& font : assets.fonts)
	{
		free_buffer(&font.buffer);
	}

	for (auto& wave : assets.wave)
	{
		free_buffer(&wave.buffer);
	}

	return 0;
}