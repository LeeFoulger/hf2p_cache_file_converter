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

	char m_tags_data_path[260];
	char* m_tags_data;
	long m_tags_data_size;
	long m_tags_data_offset;

	c_cache_file_header* m_header;
	s_cache_file_tags_header* m_tags_header;

private:
	void add_tag_section();
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
