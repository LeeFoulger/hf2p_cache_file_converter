#include <main.h>

#include <string.h>
#include <stdio.h>

int main(int argc, const char* argv[])
{
	if (argc <= 2)
	{
		printf("no map names provided\n");
		printf("usage:\n\thf2p_cache_file_converter.exe <maps path> [mapname0] [mapname1]\n");
		printf("example:\n\thf2p_cache_file_converter.exe \"C:\\Games\\ElDewrito\\maps\" mainmenu guardian\n");

		return 1;
	}

	for (size_t map_index = 2; map_index < argc; map_index++)
	{
		c_hf2p_cache_file_converter* converter = new c_hf2p_cache_file_converter(argv[1], argv[map_index]);
		if (converter->apply_changes())
			converter->write_changes_to_disk();

		delete converter;
	}

	return 0;
}

long cache_file_round_up_read_size(long size)
{
	return (size & 0xF) != 0 ? (size | 0xF) + 1 : size;
}

void cache_file_tag_instance::zero_out()
{
	memset(this, 0, total_size);
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
	update_section(_cache_file_tag_section, tags_data_offset, tags_data_offset, tags_data_size);
	update_tags_header(true);
	zero_other_scenarios();

	return true;
}

bool c_hf2p_cache_file_converter::write_changes_to_disk(bool replace)
{
	if (replace)
		return write_data_to_file(out_map_data, out_map_data_size, in_map_path);

	return write_data_to_file(out_map_data, out_map_data_size, out_map_path);
}

void c_hf2p_cache_file_converter::update_shared_file_flags(long bit, bool add)
{
	unsigned long& shared_file_flags = *get_data_at_offset<unsigned long>(0x168);

	if (add)
		shared_file_flags |= (1 << bit);
	else
		shared_file_flags &= ~(1 << bit);
}

void c_hf2p_cache_file_converter::update_section(e_cache_file_section section_index, long section_base_offset, long section_offset, long section_size)
{
	s_cache_file_section_table& section_table = *get_data_at_offset<s_cache_file_section_table>(0x434);

	section_table.base_offsets[section_index] = section_base_offset;
	section_table.section[section_index].offset = section_offset;
	section_table.section[section_index].size = section_size;
}

void c_hf2p_cache_file_converter::update_tags_header(bool zero_old_header)
{
	get_tag_table_offset() = *get_tag_data_at_offset<long>(4);
	get_tag_count() = *get_tag_data_at_offset<long>(8);

	if (zero_old_header)
		memset(tags_data, 0, 0x20);
}

void c_hf2p_cache_file_converter::zero_other_scenarios()
{
	static long* tag_table = get_tag_table();
	static long tag_count = get_tag_count();
	static long scenario_tag_index = get_scenario_index();
	static long cache_file_resource_gestalt_index = get_cache_file_resource_gestalt_index();

	for (long tag_index = 0; tag_index < tag_count; tag_index++)
	{
		if (tag_table[tag_index] == 0)
			continue;

		cache_file_tag_instance& tag_instance = tag_instance_get(tag_index);
		if (tag_instance.is_group('scnr') && tag_index != scenario_tag_index)
		{
			tag_table[tag_index] = 0;
			tag_instance.zero_out();
		}
		
		if (tag_instance.is_group('zone') && tag_index != cache_file_resource_gestalt_index)
		{
			tag_table[tag_index] = 0;
			tag_instance.zero_out();
		}
	}
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
