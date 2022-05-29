#pragma once

long cache_file_round_up_read_size(long size);

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

	void zero_out();
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
	void update_shared_file_flags(long bit, bool add);
	void update_section(e_cache_file_section_type section_type, long section_offset, long section_size);
	void update_tags_header(bool zero_old_header = false);
	void zero_other_scenarios();

	template<typename t_type>
	t_type* get_data_at_offset(long offset)
	{
		unsigned long shared_file_flags = *reinterpret_cast<unsigned long*>(out_map_data + 0x168);

		if (offset >= 0x1A4) // mapname offset
		{
			offset += shared_file_flags & (1 << k_render_models_shared_file_index) ? 8 : 0;
			offset += shared_file_flags & (1 << k_lightmaps_shared_file_index) ? 8 : 0;
		}

		return reinterpret_cast<t_type*>(out_map_data + offset);
	}

	long get_file_version()
	{
		return *get_data_at_offset<long>(4);
	}

	long& get_file_size()
	{
		return *get_data_at_offset<long>(8);
	}

	long& get_tag_table_offset()
	{
		return *get_data_at_offset<long>(0x2DE4);
	}

	long* get_tag_table()
	{
		long tag_table_offset = get_tag_table_offset();
		return get_tag_data_at_offset<long>(tag_table_offset);
	}

	long& get_tag_count()
	{
		return *get_data_at_offset<long>(0x2DE8);
	}

	long& get_map_id()
	{
		return *get_data_at_offset<long>(0x2DEC);
	}

	long& get_scenario_index()
	{
		return *get_data_at_offset<long>(0x2DF0);
	}

	long& get_cache_file_resource_gestalt_index()
	{
		return *get_data_at_offset<long>(0x2DF4);
	}

	template<typename t_type>
	t_type* get_tag_data_at_offset(long offset)
	{
		return reinterpret_cast<t_type*>(tags_data + offset);
	}

	cache_file_tag_instance& tag_instance_get(long tag_index)
	{
		long* tag_table = get_tag_table();

		return *get_tag_data_at_offset<cache_file_tag_instance>(tag_table[tag_index]);
	}

};

bool read_data_from_file(char*& out_data, long& out_size, const char* filename);
bool write_data_to_file(char* data, long size, const char* filename);