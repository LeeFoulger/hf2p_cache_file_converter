#pragma once
#pragma pack(push, 1)

__forceinline long cache_file_round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}

const long k_tags_shared_file_index = 1;
const long k_render_models_shared_file_index = 6;
const long k_lightmaps_shared_file_index = 7;

// 1011 1111 1111 1111 1111 1111 1111 1111
//  ^ bit 30
const unsigned long k_runtime_address_mask = 0xBFFFFFFF;

struct s_file_last_modification_date
{
	long low;
	long high;
};
static_assert(sizeof(s_file_last_modification_date) == 0x8, "sizeof(s_file_last_modification_date) != 0x8");

struct s_network_http_request_hash
{
	char __data[0x14];
};
static_assert(sizeof(s_network_http_request_hash) == 0x14, "sizeof(s_network_http_request_hash) != 0x14");

struct s_rsa_signature
{
	char __data[0x100];
};
static_assert(sizeof(s_rsa_signature) == 0x100, "sizeof(s_rsa_signature) != 0x100");

enum e_cache_file_section
{
	_cache_file_debug_section = 0,
	_cache_file_resource_section,
	_cache_file_tag_section,
	_cache_file_localization_section,

	k_cache_file_section_count
};

struct s_cache_file_section_file_bounds
{
	long offset;
	long size;
};
static_assert(sizeof(s_cache_file_section_file_bounds) == 0x8, "sizeof(s_cache_file_section) != 0x8");

struct s_cache_file_shared_resource_usage
{
	char __data[0x2328];
};
static_assert(sizeof(s_cache_file_shared_resource_usage) == 0x2328, "sizeof(s_cache_file_shared_resource_usage) != 0x2328");

struct s_cache_file_insertion_point_resource_usage
{
	char __data[0xB4];
};
static_assert(sizeof(s_cache_file_insertion_point_resource_usage) == 0xB4, "sizeof(s_cache_file_insertion_point_resource_usage) != 0xB4");

template<size_t k_shared_file_count>
struct s_cache_file_header
{
	unsigned long header_signature;

	long file_version;
	long file_size;

	long file_compressed_size;

	long tag_data_offset;
	long tag_buffer_offset;
	long total_tags_size;

	char source_file[256];
	char build[32];

	short scenario_type;
	short scenario_load_type;

	bool __unknown140;
	bool tracked_build;
	bool has_insertion_points;
	unsigned char header_flags;

	s_file_last_modification_date last_modification_date;

	long __unknown14C;
	long __unknown150;
	long __unknown154;

	long string_id_index_buffer_count;
	long string_id_string_storage_size;
	long string_id_index_buffer;
	long string_id_string_storage;

	unsigned char shared_file_type_flags;
	unsigned char __align169[3];
	s_file_last_modification_date creation_time;
	s_file_last_modification_date shared_file_times[k_shared_file_count];

	char name[32];
	long game_language;
	char relative_path[256];
	long minor_version;

	long debug_tag_name_count;
	long debug_tag_name_buffer;
	long debug_tag_name_buffer_length;
	long debug_tag_name_offsets;

	long reports_offset;
	long reports_size;

	char __data2F4[60];

	s_network_http_request_hash hash;
	s_rsa_signature rsa_signature;

	long section_offsets[4];
	s_cache_file_section_file_bounds original_section_bounds[4];

	s_cache_file_shared_resource_usage shared_resource_usage;
	char insertion_point_count;
	unsigned char __align279D[3];
	s_cache_file_insertion_point_resource_usage insertion_point_resource_usage_storage[9];

	long tag_table_offset;
	long tag_count;
	long map_id;
	long scenario_index;
	long cache_file_resource_gestalt_index;

	unsigned char __data2E08[0x5C4 - sizeof(shared_file_times)];

	unsigned long footer_signature;
};
static_assert(sizeof(s_cache_file_header<6>) == 0x3390, "sizeof(s_cache_file_header) == 0x3390");
static_assert(sizeof(s_cache_file_header<8>) == 0x3390, "sizeof(s_cache_file_header) == 0x3390");

class c_cache_file_header
{
private:
	s_cache_file_header<6>* pre_zone;
	s_cache_file_header<8>* post_zone;

	bool older_build()
	{
		if ((*pre_zone).shared_file_type_flags & (1 << k_render_models_shared_file_index) &&
			(*pre_zone).shared_file_type_flags & (1 << k_lightmaps_shared_file_index))
			return false;

		return true;
	}

public:
	c_cache_file_header(char*& data):
		pre_zone(reinterpret_cast<decltype(pre_zone)>(data)),
		post_zone(reinterpret_cast<decltype(post_zone)>(data)),
#define GET_VERSIONED_FIELD(field) field(older_build() ? (*pre_zone).field : (*post_zone).field)
		GET_VERSIONED_FIELD(header_signature),
		GET_VERSIONED_FIELD(file_version),
		GET_VERSIONED_FIELD(file_size),
		GET_VERSIONED_FIELD(file_compressed_size),
		GET_VERSIONED_FIELD(tag_data_offset),
		GET_VERSIONED_FIELD(tag_buffer_offset),
		GET_VERSIONED_FIELD(total_tags_size),
		GET_VERSIONED_FIELD(source_file),
		GET_VERSIONED_FIELD(build),
		GET_VERSIONED_FIELD(scenario_type),
		GET_VERSIONED_FIELD(scenario_load_type),
		GET_VERSIONED_FIELD(__unknown140),
		GET_VERSIONED_FIELD(tracked_build),
		GET_VERSIONED_FIELD(has_insertion_points),
		GET_VERSIONED_FIELD(header_flags),
		GET_VERSIONED_FIELD(last_modification_date),
		GET_VERSIONED_FIELD(__unknown14C),
		GET_VERSIONED_FIELD(__unknown150),
		GET_VERSIONED_FIELD(__unknown154),
		GET_VERSIONED_FIELD(string_id_index_buffer_count),
		GET_VERSIONED_FIELD(string_id_string_storage_size),
		GET_VERSIONED_FIELD(string_id_index_buffer),
		GET_VERSIONED_FIELD(string_id_string_storage),
		GET_VERSIONED_FIELD(shared_file_type_flags),
		GET_VERSIONED_FIELD(creation_time),
		GET_VERSIONED_FIELD(shared_file_times),
		GET_VERSIONED_FIELD(name),
		GET_VERSIONED_FIELD(game_language),
		GET_VERSIONED_FIELD(relative_path),
		GET_VERSIONED_FIELD(minor_version),
		GET_VERSIONED_FIELD(debug_tag_name_count),
		GET_VERSIONED_FIELD(debug_tag_name_buffer),
		GET_VERSIONED_FIELD(debug_tag_name_buffer_length),
		GET_VERSIONED_FIELD(debug_tag_name_offsets),
		GET_VERSIONED_FIELD(reports_offset),
		GET_VERSIONED_FIELD(reports_size),
		GET_VERSIONED_FIELD(__data2F4),
		GET_VERSIONED_FIELD(hash),
		GET_VERSIONED_FIELD(rsa_signature),
		GET_VERSIONED_FIELD(section_offsets),
		GET_VERSIONED_FIELD(original_section_bounds),
		GET_VERSIONED_FIELD(shared_resource_usage),
		GET_VERSIONED_FIELD(insertion_point_count),
		GET_VERSIONED_FIELD(insertion_point_resource_usage_storage),
		GET_VERSIONED_FIELD(tag_table_offset),
		GET_VERSIONED_FIELD(tag_count),
		GET_VERSIONED_FIELD(map_id),
		GET_VERSIONED_FIELD(scenario_index),
		GET_VERSIONED_FIELD(cache_file_resource_gestalt_index),
		GET_VERSIONED_FIELD(footer_signature)
#undef GET_VERSIONED_FIELD
	{
		pre_zone = nullptr;
		post_zone = nullptr;
	}

	unsigned long& header_signature;

	long& file_version;
	long& file_size;

	long& file_compressed_size;

	long& tag_data_offset;
	long& tag_buffer_offset;
	long& total_tags_size;

	char(&source_file)[256];
	char(&build)[32];

	short& scenario_type;
	short& scenario_load_type;

	bool& __unknown140;
	bool& tracked_build;
	bool& has_insertion_points;
	unsigned char& header_flags;

	s_file_last_modification_date& last_modification_date;

	long& __unknown14C;
	long& __unknown150;
	long& __unknown154;

	long& string_id_index_buffer_count;
	long& string_id_string_storage_size;
	long& string_id_index_buffer;
	long& string_id_string_storage;

	unsigned char& shared_file_type_flags;
	s_file_last_modification_date& creation_time;
	s_file_last_modification_date* shared_file_times;

	char(&name)[32];
	long& game_language;
	char(&relative_path)[256];
	long& minor_version;

	long& debug_tag_name_count;
	long& debug_tag_name_buffer;
	long& debug_tag_name_buffer_length;
	long& debug_tag_name_offsets;

	long& reports_offset;
	long& reports_size;

	char(&__data2F4)[60];

	s_network_http_request_hash& hash;
	s_rsa_signature& rsa_signature;

	long(&section_offsets)[4];
	s_cache_file_section_file_bounds(&original_section_bounds)[4];

	s_cache_file_shared_resource_usage& shared_resource_usage;
	char& insertion_point_count;
	s_cache_file_insertion_point_resource_usage(&insertion_point_resource_usage_storage)[9];

	long& tag_table_offset;
	long& tag_count;
	long& map_id;
	long& scenario_index;
	long& cache_file_resource_gestalt_index;

	unsigned long& footer_signature;
};

struct s_cache_file_tag_group
{
	unsigned long group_tags[3];

	// string_id
	unsigned long group_name;
};
static_assert(sizeof(s_cache_file_tag_group) == 0x10, "sizeof(s_cache_file_tag_group) != 0x10");

struct s_cache_file_tag_instance
{
	unsigned long checksum;
	unsigned long total_tag_size;

	short child_count;
	short tag_data_count;
	short tag_resource_count;
	short tag_reference_count; // padding

	unsigned long address;

	s_cache_file_tag_group tag_group;

	__forceinline bool is_group(unsigned long group_tag)
	{
		return tag_group.group_tags[0] == group_tag || tag_group.group_tags[1] == group_tag || tag_group.group_tags[2] == group_tag;
	}

	template<typename t_type = char>
	__forceinline t_type* definition_get(unsigned long group_tag, long offset = 0)
	{
		return reinterpret_cast<t_type*>(reinterpret_cast<char*>(this) + address + offset);
	}

	__forceinline long* child_tag_table()
	{
		if (child_count)
			return buffer_end<long>();

		return nullptr;
	}

	__forceinline long* tag_data_table()
	{
		if (tag_data_count)
			return buffer_end<long>() + child_count;

		return nullptr;
	}

	__forceinline long* tag_resource_table()
	{
		if (tag_resource_count)
			return buffer_end<long>() + child_count + tag_data_count;

		return nullptr;
	}

	__forceinline long* tag_reference_table()
	{
		if (tag_reference_count)
			return buffer_end<long>() + child_count + tag_data_count + tag_resource_count;

		return nullptr;
	}

	__forceinline void zero_out();

private:
	template<typename t_type>
	__forceinline t_type* buffer_end(long offset = 0)
	{
		return reinterpret_cast<t_type*>(reinterpret_cast<char*>(this) + sizeof(s_cache_file_tag_instance) + offset);
	}
};
static_assert(sizeof(s_cache_file_tag_instance) == 0x24, "sizeof(s_cache_file_tag_instance) != 0x24");

struct s_cache_file_tags_header
{
	long : 32;
	long tag_table_offset;
	long tag_count;
	long : 32;
	s_file_last_modification_date creation_date;
	long : 32;
	long : 32;

	__forceinline long* tag_table()
	{
		return reinterpret_cast<long*>(reinterpret_cast<char*>(this) + tag_table_offset);
	}

	__forceinline s_cache_file_tag_instance& tag_instance_get(long tag_index)
	{
		return *reinterpret_cast<s_cache_file_tag_instance*>(reinterpret_cast<char*>(this) + tag_table()[tag_index]);
	}

	__forceinline void zero_out();
};
static_assert(sizeof(s_cache_file_tags_header) == 0x20, "sizeof(s_cache_file_tags_header) != 0x20");

struct s_tag_block
{
	long count;
	unsigned long address;
	unsigned long definition;
};
static_assert(sizeof(s_tag_block) == 0xC, "sizeof(s_tag_block) != 0xC");

struct s_tag_data
{
	unsigned long size;

	unsigned long flags;
	unsigned long stream_position;

	unsigned long address;
	unsigned long definition;
};
static_assert(sizeof(s_tag_data) == 0x14, "sizeof(s_tag_data) != 0x14");

struct s_tag_reference
{
	unsigned long group_tag;
	unsigned long name_address;
	unsigned long name_length;
	long index;
};
static_assert(sizeof(s_tag_reference) == 0x10, "sizeof(s_tag_reference) != 0x10");

struct s_tag_resource_reference
{
	unsigned long pagable_resource;
	long : 32;
};
static_assert(sizeof(s_tag_resource_reference) == 0x8, "sizeof(s_tag_resource_reference) != 0x8");

template<size_t k_added>
