#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <stdlib.h>

#include <filesystem>
#include <string>
#include <vector>

const char* asset_root = "../assets";
const char* src_root = "../src/";

using u64 = unsigned long long;
using u32 = unsigned int;

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
		return Buffer{nullptr, 0};
	}

	fseek(file, 0, SEEK_END);
	u64 size = ftell(file);
	fseek(file, 0, SEEK_SET);

	Buffer buffer = alloc(size);
	if (!buffer.mem)
	{
		fclose(file);
		return Buffer{nullptr, 0};
	}

	u64 bytes_read = fread(buffer.mem, 1, size, file);
	fclose(file);

	if (bytes_read != size)
	{
		free_buffer(&buffer);
		printf("Failed to read file: %s\n", path);
		return Buffer{nullptr, 0};
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
            Asset& asset = inout_assets->images.emplace_back();
            asset.name = entry.path().filename().stem().string();
            asset.name.erase(std::remove(asset.name.begin(), asset.name.end(), ' '), asset.name.end());
            asset.name.erase(std::remove(asset.name.begin(), asset.name.end(), '-'), asset.name.end());
            asset.buffer = buffer;
			printf("Loaded image: %s with size %lld\n", entry.path().filename().string().c_str(), asset.buffer.size);
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
    s->append("const AssetRef ");
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
    char buf [256];
    sprintf(buf, "\t{%lld, %lld},\n", *current_offset, asset->buffer.size);
    printf("%s", asset->name.c_str());
    *current_offset += asset->buffer.size;
    s->append(buf);
}

void write_asset_enum_elem(std::string* s, const Asset* asset)
{
    char buf [256];
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

void write_asset_array(std::string* s, const Assets* assets, const char* names)
{
    write_asset_array_start(s, names);
    u64 offset = 0;
    for(const auto& img : assets->images)
    {
        write_asset_array_elem(s, &img, &offset);
    }
    write_asset_array_end(s);
}

void write_asset_enum(std::string* s, const Assets* assets, const char* name)
{
    write_asset_enum_start(s, name);
    for(const auto& img : assets->images)
    {
        write_asset_enum_elem(s, &img);
    }
    write_asset_enum_end(s);
}

void write_asset_fwd_decl(std::string* s, const char* name)
{
    s->append("extern const unsigned char ");
    s->append(name);
    s->append("[]");
    s->append(";\n\n");
}

void write_asset_type(std::string* s, const Assets* assets, const char* name, const char* offset_name, const char* data_name)
{
    write_asset_enum(s, assets, name);
    write_asset_array(s, assets, offset_name);
    write_asset_fwd_decl(s, data_name);
}

void write_asset_bin(std::string* s, const Assets* assets, const char* data_name)
{
    s->append("\n");
    s->append("const unsigned char ");
    s->append(data_name);
    s->append("[] = { ");

    for(const auto& asset : assets->images)
    {
        for(u64 i = 0; i < asset.buffer.size; i++)
        {
            char buf[8];
            sprintf(buf, "0x%02x, ", (unsigned char)asset.buffer.mem[i]);
            s->append(buf);
        }
    }
    
    s->append("};\n");
}

std::string generate_header(const Assets* assets)
{
    std::string buf;
    buf.reserve(1024);

    buf.append("#pragma once \n\n");
    buf.append("using u64 = unsigned long long;\n\n");
    buf.append("struct AssetRef \n{\n");
    buf.append("\tu64 offset;\n");
    buf.append("\tu64 size;\n");
    buf.append("};\n");
    buf.append("\n");

    u64 offset = 0;

    write_asset_type(&buf, assets, "Sprite", "g_spriteOffsets", "g_spriteData");
    
    return buf;
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

    for(const auto& asset : assets->images)
    {
        for(u64 i = 0; i < asset.buffer.size; i++)
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

int main()
{
	Assets assets;

    load_assets(&assets);

	printf("Loaded %zu images\n", assets.images.size());
    
    write_source_file(generate_header(&assets), "../src/gen/sprites.h");
    write_source_file(generate_source(&assets, "sprites.h", "g_spriteData"), "../src/gen/sprites.cpp");

	for (auto& img : assets.images)
	{
		free_buffer(&img.buffer);
	}

	return 0;
}