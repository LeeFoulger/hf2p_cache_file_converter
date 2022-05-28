
#include <string.h>
#include <stdio.h>

class c_hf2p_cache_file_converter
{
	const long k_tags_shared_file_index = 1;
	const long k_render_models_shared_file_index = 6;
	const long k_lightmaps_shared_file_index = 7;
	const long k_tags_section_index = 2;

public:
	c_hf2p_cache_file_converter(const char* maps_path, const char* map_name);
	~c_hf2p_cache_file_converter();
	bool apply_changes();
	bool write_changes_to_disk(bool replace = false);

protected:
	char in_map_path[260];
	char* in_map_data;
	long in_map_data_size;

	char out_map_path[260];
	char* out_map_data;
	long out_map_data_size;

	char tags_data_path[260];
	char* tags_data;
	long tags_data_size;
	long tags_data_offset;

private:
	long cache_file_round_up_read_size(long size);
	long cache_file_get_file_version();
	long& cache_file_get_file_size();
	void cache_file_set_shared_file_flags(long bit, bool add);
	long get_post_shared_file_dates_offset();
	void cache_file_set_section_info(long section_index);
	void cache_file_set_tags_info();
};

int main()
{
	const char* maps_path = "P:/Dev/hf2p/maps";
	const char* maps_to_convert[]
	{
		"mainmenu",
		"bunkerworld",
		"chill",
		"cyberdyne",
		"deadlock",
		"guardian",
		"riverworld",
		"shrine",
		"zanzibar",
		//"s3d_avalanche",
		//"s3d_edge",
		//"s3d_reactor",
		//"s3d_turf",
	};

	size_t map_count = sizeof(maps_to_convert) / sizeof(maps_to_convert[0]);
	for (size_t i = 0; i < map_count; i++)
	{
		c_hf2p_cache_file_converter* converter = new c_hf2p_cache_file_converter(maps_path, maps_to_convert[i]);
		if (converter->apply_changes())
			converter->write_changes_to_disk();
		delete converter;
	}

	return 0;
}

bool read_data_from_file(char*& out_data, long& out_size, const char* filename)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filename, "rb"), file != nullptr)
	{
		fseek(file, 0, SEEK_END);
		out_size = ftell(file);
		fseek(file, 0, SEEK_SET);
		out_data = new char[out_size];
		fread(out_data, 1, out_size, file);
		fclose(file);
		return true;
	}
	return false;
}

bool write_data_to_file(char* data, long size, const char* filename)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filename, "wb"), file != nullptr)
	{
		fwrite(data, 1, size, file);
		fclose(file);
		return true;
	}
	return false;
}

c_hf2p_cache_file_converter::c_hf2p_cache_file_converter(const char* maps_path, const char* map_name) :
	in_map_path(), in_map_data(), in_map_data_size(),
	out_map_path(), out_map_data(), out_map_data_size(),
	tags_data_path(), tags_data(), tags_data_size(), tags_data_offset()
{
	sprintf_s(in_map_path, "%s/%s.map", maps_path, map_name);
	sprintf_s(out_map_path, "%s/%s_out.map", maps_path, map_name);
	sprintf_s(tags_data_path, "%s/tags.dat", maps_path);

	read_data_from_file(in_map_data, in_map_data_size, in_map_path);
	read_data_from_file(tags_data, tags_data_size, tags_data_path);

	tags_data_offset = cache_file_round_up_read_size(in_map_data_size);
	out_map_data_size = tags_data_offset + cache_file_round_up_read_size(tags_data_size);
	out_map_data = new char[out_map_data_size] {};
	memcpy(out_map_data, in_map_data, in_map_data_size);
	memcpy(out_map_data + tags_data_offset, tags_data, tags_data_size);
}

c_hf2p_cache_file_converter::~c_hf2p_cache_file_converter()
{
	if (in_map_data) delete[] in_map_data;
	if (tags_data) delete[] tags_data;
	if (out_map_data) delete[] out_map_data;

	in_map_data = nullptr;
	tags_data = nullptr;
	out_map_data = nullptr;

	in_map_data_size = 0;
	out_map_data_size = 0;
	tags_data_size = 0;
}

bool c_hf2p_cache_file_converter::apply_changes()
{
	if (cache_file_get_file_version() != 18)
	{
		printf("invalid file version\n");
		return false;
	}

	cache_file_get_file_size() = out_map_data_size;
	cache_file_set_shared_file_flags(k_tags_shared_file_index, false);
	cache_file_set_section_info(k_tags_section_index);
	cache_file_set_tags_info();

	return true;
}

bool c_hf2p_cache_file_converter::write_changes_to_disk(bool replace)
{
	if (replace)
		return write_data_to_file(out_map_data, out_map_data_size, in_map_path);
	return write_data_to_file(out_map_data, out_map_data_size, out_map_path);
}

long c_hf2p_cache_file_converter::cache_file_round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}

long c_hf2p_cache_file_converter::cache_file_get_file_version()
{
	return *reinterpret_cast<long*>(&in_map_data[4]);
}

long& c_hf2p_cache_file_converter::cache_file_get_file_size()
{
	return *reinterpret_cast<long*>(&out_map_data[8]);
}

void c_hf2p_cache_file_converter::cache_file_set_shared_file_flags(long bit, bool add)
{
	unsigned long& shared_file_flags = *reinterpret_cast<unsigned long*>(&out_map_data[0x168]);
	if (add)
		shared_file_flags |= (1 << bit);
	else
		shared_file_flags &= ~(1 << bit);
}

long c_hf2p_cache_file_converter::get_post_shared_file_dates_offset()
{
	unsigned long& shared_file_flags = *reinterpret_cast<unsigned long*>(&out_map_data[0x168]);
	return shared_file_flags & (1 << k_render_models_shared_file_index) && shared_file_flags & (1 << k_lightmaps_shared_file_index) ? 0x10 : 0;
}

void c_hf2p_cache_file_converter::cache_file_set_section_info(long section_index)
{
	long(&section_masks)[4] = *reinterpret_cast<long(*)[4]>(&out_map_data[0x434 + get_post_shared_file_dates_offset()]);
	long(&section_bounds)[4][2] = *reinterpret_cast<long(*)[4][2]>(&out_map_data[0x444 + get_post_shared_file_dates_offset()]);

	section_masks[section_index] = tags_data_offset;
	section_bounds[section_index][0] = tags_data_offset;
	section_bounds[section_index][1] = tags_data_size;
}

void c_hf2p_cache_file_converter::cache_file_set_tags_info()
{
	memcpy(&out_map_data[0x2DE0 + get_post_shared_file_dates_offset()], tags_data, 0x10);
}

