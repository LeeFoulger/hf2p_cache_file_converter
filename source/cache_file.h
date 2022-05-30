#pragma once
#pragma pack(push, 1)

long cache_file_round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}

const long k_tags_shared_file_index = 1;
const long k_render_models_shared_file_index = 6;
const long k_lightmaps_shared_file_index = 7;

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

struct s_cache_file_header_pre_zone
{
	s_file_last_modification_date shared_file_times[6];

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

	char __data2D4[60];

	s_network_http_request_hash hash;
	s_rsa_signature rsa_signature;

	long section_offsets[4];
	s_cache_file_section_file_bounds original_section_bounds[4];

	s_cache_file_shared_resource_usage shared_resource_usage;

	char insertion_point_count;
	unsigned char __align278D[3];
	s_cache_file_insertion_point_resource_usage insertion_point_resource_usage_storage[9];
	long tag_table_offset;
	long tag_count;
	long map_id;
	long scenario_index;
	long cache_file_resource_gestalt_index;

	unsigned char __data2DF8[0x594];

	unsigned long footer_signature;
};

struct s_cache_file_header_post_zone
{
	s_file_last_modification_date shared_file_times[8];

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

	unsigned char __data2E08[0x584];

	unsigned long footer_signature;
};

struct s_cache_file_header
{
	unsigned long header_signature;

	long file_version;
	long file_size;

	long file_compressed_length;

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

	union
	{
		s_cache_file_header_pre_zone pre_zone;
		s_cache_file_header_post_zone post_zone;
	};

	bool older_build()
	{
		if (shared_file_type_flags & (1 << k_render_models_shared_file_index) &&
			shared_file_type_flags & (1 << k_lightmaps_shared_file_index))
			return false;

		return true;
	}

	inline s_file_last_modification_date* shared_file_times()
	{
		return older_build() ? pre_zone.shared_file_times : post_zone.shared_file_times;
	}
#define DECLARE_VERSIONED_FIELD(field) __forceinline auto& field() { return older_build() ? pre_zone.field : post_zone.field; }
	DECLARE_VERSIONED_FIELD(name);
	DECLARE_VERSIONED_FIELD(game_language);
	DECLARE_VERSIONED_FIELD(relative_path);
	DECLARE_VERSIONED_FIELD(minor_version);
	DECLARE_VERSIONED_FIELD(debug_tag_name_count);
	DECLARE_VERSIONED_FIELD(debug_tag_name_buffer);
	DECLARE_VERSIONED_FIELD(debug_tag_name_buffer_length);
	DECLARE_VERSIONED_FIELD(debug_tag_name_offsets);
	DECLARE_VERSIONED_FIELD(reports_offset);
	DECLARE_VERSIONED_FIELD(reports_size);
	DECLARE_VERSIONED_FIELD(hash);
	DECLARE_VERSIONED_FIELD(rsa_signature);
	DECLARE_VERSIONED_FIELD(section_offsets);
	DECLARE_VERSIONED_FIELD(original_section_bounds);
	DECLARE_VERSIONED_FIELD(shared_resource_usage);
	DECLARE_VERSIONED_FIELD(insertion_point_count);
	DECLARE_VERSIONED_FIELD(insertion_point_resource_usage_storage);
	DECLARE_VERSIONED_FIELD(tag_table_offset);
	DECLARE_VERSIONED_FIELD(tag_count);
	DECLARE_VERSIONED_FIELD(map_id);
	DECLARE_VERSIONED_FIELD(scenario_index);
	DECLARE_VERSIONED_FIELD(cache_file_resource_gestalt_index);
	DECLARE_VERSIONED_FIELD(footer_signature);
#undef DECLARE_VERSIONED_FIELD
};
static_assert(sizeof(s_cache_file_header) == 0x3390, "sizeof(s_cache_file_header) != 0x3390");

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

	__forceinline cache_file_tag_instance& tag_instance_get(long tag_index)
	{
		return *reinterpret_cast<cache_file_tag_instance*>(reinterpret_cast<char*>(this) + tag_table()[tag_index]);
	}
};
static_assert(sizeof(s_cache_file_tags_header) == 0x20, "sizeof(s_cache_file_tags_header) != 0x20");

#pragma pack(pop)