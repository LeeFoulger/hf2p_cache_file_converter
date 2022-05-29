
#include <string.h>
#include <stdio.h>

enum e_cache_file_section_type
{
	_cache_file_debug_section = 0,
	_cache_file_resource_section,
	_cache_file_tag_section,
	_cache_file_localization_section,

	k_cache_file_section_types
};

struct cache_file_section_bounds
{
	long virtual_address;
	long size;
};
static_assert(sizeof(cache_file_section_bounds) == 0x8, "sizeof(cache_file_section_bounds) != 0x8");

using t_section_masks = long[k_cache_file_section_types];
using t_section_bounds = cache_file_section_bounds[k_cache_file_section_types];

struct s_cache_file_tag_group
{
	unsigned long group_tags[3];

	// string_id
	long group_name;
};
static_assert(sizeof(s_cache_file_tag_group) == 0x10, "sizeof(s_cache_file_tag_group) != 0x10");

struct cache_file_tag_instance
{
	unsigned long checksum;
	long total_size;

	short child_tag_count;
	short data_fixup_count;
	short resource_fixup_count;
	short tag_reference_fixup_count;

	long definition_offset;

	s_cache_file_tag_group tag_group;

	bool is_group(unsigned long group_tag)
	{
		return tag_group.group_tags[0] == group_tag || tag_group.group_tags[1] == group_tag || tag_group.group_tags[2] == group_tag;
	}

	template<typename t_type = char>
	t_type* definition_get(unsigned long group_tag)
	{
		return reinterpret_cast<t_type*>(reinterpret_cast<char*>(this) + definition_offset);
	}

	void zero_out()
	{
		memset(this, 0, total_size);
	}
};
static_assert(sizeof(cache_file_tag_instance) == 0x24, "sizeof(cache_file_tag_instance) != 0x24");

class c_hf2p_cache_file_converter
{
	const long k_tags_shared_file_index = 1;
	const long k_render_models_shared_file_index = 6;
	const long k_lightmaps_shared_file_index = 7;

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
	long round_up_read_size(long size);

	void update_shared_file_flags(long bit, bool add);
	void update_section(e_cache_file_section_type section_type, long section_offset, long section_size);
	void update_tags_header(bool zero_old_header = false);

	template<typename t_type = char>
	t_type* get_data_at_offset(long offset);

	long get_file_version();
	long& get_file_size();
	long* get_tag_table();
	long& get_tag_count();
	long& get_map_id();
	long& get_scenario_index();
	long& get_cache_file_resource_gestalt_index();

	template<typename t_type = char>
	t_type* get_tag_data_at_offset(long offset);

	cache_file_tag_instance& tag_instance_get(long tag_index);
};

int main(int argc, const char* argv[])
{
	if (argc <= 2)
	{
		printf("no map names provided\n");
		printf("usage:\n\thf2p_cache_file_converter.exe <maps path> [mapname0] [mapname1]\n");
		printf("example:\n\thf2p_cache_file_converter.exe \"C:\\Games\\ElDewrito\\maps\" mainmenu guardian\n");

		return 1;
	}

	const char* maps_path = argv[1];
	size_t maps_count = argc;
	for (size_t i = 2; i < maps_count; i++)
	{
		const char* map_name = argv[i];
		c_hf2p_cache_file_converter* converter = new c_hf2p_cache_file_converter(maps_path, map_name);
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

	tags_data_offset = round_up_read_size(in_map_data_size);
	out_map_data_size = tags_data_offset + round_up_read_size(tags_data_size);
	out_map_data = new char[out_map_data_size] {};
	memcpy(out_map_data, in_map_data, in_map_data_size);
	memcpy(out_map_data + tags_data_offset, tags_data, tags_data_size);

	delete[] tags_data;
	tags_data = out_map_data + tags_data_offset;
}

c_hf2p_cache_file_converter::~c_hf2p_cache_file_converter()
{
	if (in_map_data) delete[] in_map_data;
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
	if (get_file_version() != 18)
	{
		printf("invalid file version\n");
		return false;
	}

	get_file_size() = out_map_data_size;
	update_shared_file_flags(k_tags_shared_file_index, false);
	update_section(_cache_file_tag_section, tags_data_offset, tags_data_size);
	update_tags_header(true);

	long* tag_table = get_tag_table();
	for (long tag_index = 0; tag_index < get_tag_count(); tag_index++)
	{
		cache_file_tag_instance& tag_instance = tag_instance_get(tag_index);
		if (tag_instance.is_group('scnr') && tag_index != get_scenario_index())
		{
			tag_table[tag_index] = 0;
			tag_instance.zero_out();
		}
	}

	return true;
}

bool c_hf2p_cache_file_converter::write_changes_to_disk(bool replace)
{
	if (replace)
		return write_data_to_file(out_map_data, out_map_data_size, in_map_path);
	return write_data_to_file(out_map_data, out_map_data_size, out_map_path);
}

long c_hf2p_cache_file_converter::round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}
void c_hf2p_cache_file_converter::update_shared_file_flags(long bit, bool add)
{
	unsigned long& shared_file_flags = *get_data_at_offset<unsigned long>(0x168);
	if (add)
		shared_file_flags |= (1 << bit);
	else
		shared_file_flags &= ~(1 << bit);
}

void c_hf2p_cache_file_converter::update_section(e_cache_file_section_type section_type, long section_offset, long section_size)
{
	t_section_masks &section_masks = *get_data_at_offset<t_section_masks>(0x434);
	t_section_bounds& section_bounds = *get_data_at_offset<t_section_bounds>(0x444);

	section_masks[section_type] = section_offset;
	section_bounds[section_type].virtual_address = section_offset;
	section_bounds[section_type].size = section_size;
}

void c_hf2p_cache_file_converter::update_tags_header(bool zero_old_header)
{
	*get_data_at_offset<long>(0x2DE4) = *get_tag_data_at_offset<long>(4);
	*get_data_at_offset<long>(0x2DE8) = *get_tag_data_at_offset<long>(8);
	if (zero_old_header)
		memset(tags_data, 0, 0x20);
}

template<typename t_type>
t_type* c_hf2p_cache_file_converter::get_data_at_offset(long offset)
{
	unsigned long shared_file_flags = *reinterpret_cast<unsigned long*>(out_map_data + 0x168);
	if (offset >= 0x1A4) // mapname offset
	{
		offset += shared_file_flags & (1 << k_render_models_shared_file_index) ? 8 : 0;
		offset += shared_file_flags & (1 << k_lightmaps_shared_file_index) ? 8 : 0;
	}

	return reinterpret_cast<t_type*>(out_map_data + offset);
}

long c_hf2p_cache_file_converter::get_file_version()
{
	return *get_data_at_offset<long>(4);
}

long& c_hf2p_cache_file_converter::get_file_size()
{
	return *get_data_at_offset<long>(8);
}

long* c_hf2p_cache_file_converter::get_tag_table()
{
	return get_tag_data_at_offset<long>(*get_data_at_offset<long>(0x2DE4));
}

long& c_hf2p_cache_file_converter::get_tag_count()
{
	return *get_data_at_offset<long>(0x2DE8);
}

long& c_hf2p_cache_file_converter::get_map_id()
{
	return *get_data_at_offset<long>(0x2DEC);
}

long& c_hf2p_cache_file_converter::get_scenario_index()
{
	return *get_data_at_offset<long>(0x2DF0);
}

long& c_hf2p_cache_file_converter::get_cache_file_resource_gestalt_index()
{
	return *get_data_at_offset<long>(0x2DF4);
}

template<typename t_type>
t_type* c_hf2p_cache_file_converter::get_tag_data_at_offset(long offset)
{
	return reinterpret_cast<t_type*>(tags_data + offset);
}

cache_file_tag_instance& c_hf2p_cache_file_converter::tag_instance_get(long tag_index)
{
	long* tag_table = get_tag_table();
	return *get_tag_data_at_offset<cache_file_tag_instance>(tag_table[tag_index]);
}
