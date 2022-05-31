#pragma once
#pragma pack(push, 1)

__forceinline long cache_file_round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}

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

enum e_cache_file_shared_type
{
	_cache_file_shared_unknown0 = 0,

	_cache_file_shared_tags,
	_cache_file_shared_resources,

	_cache_file_shared_unknown3,
	_cache_file_shared_unknown4,
	_cache_file_shared_unknown5,

	k_cache_file_shared_old_count,

	_cache_file_shared_render_models = k_cache_file_shared_old_count,
	_cache_file_shared_lightmaps,

	k_cache_file_shared_new_count
};

const char* k_cache_file_shared_type_names[k_cache_file_shared_new_count]
{
	"",
	"tags",
	"resources",
	"",
	"",
	"",
	"render_models",
	"lightmaps"
};

template<long k_shared_file_count>
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
static_assert(sizeof(s_cache_file_header<k_cache_file_shared_old_count>) == 0x3390, "sizeof(s_cache_file_header) == 0x3390");
static_assert(sizeof(s_cache_file_header<k_cache_file_shared_new_count>) == 0x3390, "sizeof(s_cache_file_header) == 0x3390");

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

template<long k_added>
struct s_tag_resource_page
{
	unsigned short identifier;

	//unsigned char checksum : 1;
	//unsigned char resources : 1;
	//unsigned char textures : 1;
	//unsigned char textures_b : 1;
	//unsigned char audio : 1;
	//unsigned char video : 1;
	//unsigned char render_models : 1;
	//unsigned char lightmaps : 1;
	unsigned char flags;

	char compression_codec_index;
	long index;
	long compressed_block_size;
	long uncompressed_block_size;
	long crc_checksum;
	short block_asset_count;
	short : 16;
	long extra[3 + k_added];
};
static_assert(sizeof(s_tag_resource_page<0>) == 0x24, "sizeof(s_tag_resource_page) != 0x24");
static_assert(sizeof(s_tag_resource_page<1>) == 0x28, "sizeof(s_tag_resource_page) != 0x28");

// post zone added 4 bytes
struct s_tag_resource : s_tag_resource_page<0>
{
	s_tag_reference parent_tag;
	unsigned short salt;
	char resource_type_index;
	unsigned char flags;
	struct s_tag_data definition_data;
	unsigned long definition_address;
	s_tag_block resource_fixups;
	s_tag_block resource_definition_fixups;
	long : 32;
};
static_assert(sizeof(s_tag_resource) == 0x6C, "sizeof(s_tag_resource) != 0x6C");
//static_assert(sizeof(s_tag_resource_page) == 0x70, "sizeof(s_tag_resource_page) != 0x70");

#pragma pack(pop)
