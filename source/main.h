#pragma once

#include <stdio.h>
#include <string.h>

#include <cache_file.h>

void s_cache_file_tag_instance::zero_out()
{
	memset(this, 0, total_tag_size);
}

void s_cache_file_tags_header::zero_out()
{
	memset(this, 0, sizeof(*this));
}

class c_cache_file_header
{
private:
	s_cache_file_header<6>* pre_zone;
	s_cache_file_header<8>* post_zone;

	bool older_build()
	{
		if ((*pre_zone).shared_file_type_flags & (1 << _cache_file_shared_render_models) &&
			(*pre_zone).shared_file_type_flags & (1 << _cache_file_shared_lightmaps))
			return false;

		return true;
	}

public:
	c_cache_file_header(char* data);

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

class c_cache_file_tags_header
{
private:
	s_cache_file_tags_header* tags_header;

public:
	c_cache_file_tags_header(char* data) :
		tags_header(reinterpret_cast<s_cache_file_tags_header*>(data)),
		tag_table_offset(tags_header->tag_table_offset),
		tag_count(tags_header->tag_count),
		creation_date(tags_header->creation_date),
		tag_table(reinterpret_cast<long*>(reinterpret_cast<char*>(tags_header) + tags_header->tag_table_offset))
	{
	}

	__forceinline s_cache_file_tag_instance& tag_instance_get(long tag_index)
	{
		return *reinterpret_cast<s_cache_file_tag_instance*>(reinterpret_cast<char*>(tags_header) + tag_table[tag_index]);
	}
	
	void handle_child_tags(long index);
	void handle_tag_data(long index);
	void handle_tag_resources(long index);
	void handle_tag_references(long index);

	long& tag_table_offset;
	long& tag_count;
	s_file_last_modification_date& creation_date;

	long* tag_table;
};

class c_hf2p_cache_file_converter
{
public:
	c_hf2p_cache_file_converter(const char* maps_path, const char* map_name);
	~c_hf2p_cache_file_converter();
	bool apply_changes();
	bool write_changes_to_disk(bool replace = false);

protected:
	char m_in_map_path[260];
	char* m_in_map_data;
	long m_in_map_data_size;

	char m_out_map_path[260];
	char* m_out_map_data;
	long m_out_map_data_size;

	char* m_shared_file_data_paths[k_cache_file_shared_new_count];
	char* m_shared_file_datas[k_cache_file_shared_new_count];
	long m_shared_file_data_sizes[k_cache_file_shared_new_count];
	long m_shared_file_data_offsets[k_cache_file_shared_new_count];

	c_cache_file_header* m_header;
	c_cache_file_tags_header* m_shared_file_headers[k_cache_file_shared_new_count];

private:
	void add_debug_section();
	void add_resource_section();
	void add_tag_section();
	void add_localization_section();
	void zero_unnused_tags();
};

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
