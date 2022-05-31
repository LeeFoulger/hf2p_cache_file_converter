#include <main.h>

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

c_hf2p_cache_file_converter::c_hf2p_cache_file_converter(const char* maps_path, const char* map_name) :
	m_in_map_path(), m_in_map_data(), m_in_map_data_size(),
	m_out_map_path(), m_out_map_data(), m_out_map_data_size(),
	m_tags_data_path(), m_tags_data(), m_tags_data_size(), 
	m_tags_data_offset(), m_header(), m_tags_header()
{
	sprintf_s(m_in_map_path, "%s/%s.map", maps_path, map_name);
	sprintf_s(m_out_map_path, "%s/%s_out.map", maps_path, map_name);
	sprintf_s(m_tags_data_path, "%s/tags.dat", maps_path);

	read_data_from_file(m_in_map_data, m_in_map_data_size, m_in_map_path);
	read_data_from_file(m_tags_data, m_tags_data_size, m_tags_data_path);

	m_tags_data_offset = cache_file_round_up_read_size(m_in_map_data_size);
	m_out_map_data_size = m_tags_data_offset + cache_file_round_up_read_size(m_tags_data_size);
	m_out_map_data = new char[m_out_map_data_size] {};
	memcpy(m_out_map_data, m_in_map_data, m_in_map_data_size);
	memcpy(m_out_map_data + m_tags_data_offset, m_tags_data, m_tags_data_size);

	m_header = new c_cache_file_header(m_out_map_data);
	m_tags_header = reinterpret_cast<s_cache_file_tags_header*>(m_out_map_data + m_tags_data_offset);
}

c_hf2p_cache_file_converter::~c_hf2p_cache_file_converter()
{
	if (m_in_map_data) delete[] m_in_map_data;
	if (m_out_map_data) delete[] m_out_map_data;

	m_in_map_data = nullptr;
	m_tags_data = nullptr;
	m_out_map_data = nullptr;

	m_in_map_data_size = 0;
	m_out_map_data_size = 0;
	m_tags_data_size = 0;
}

bool c_hf2p_cache_file_converter::apply_changes()
{
	if (m_header->file_version != 18)
	{
		printf("invalid file version\n");
		return false;
	}

	add_tag_section();
	zero_unnused_tags();

	return true;
}

bool c_hf2p_cache_file_converter::write_changes_to_disk(bool replace)
{
	if (replace)
		return write_data_to_file(m_out_map_data, m_out_map_data_size, m_in_map_path);

	return write_data_to_file(m_out_map_data, m_out_map_data_size, m_out_map_path);
}

void c_hf2p_cache_file_converter::add_tag_section()
{
	m_header->file_size = m_out_map_data_size;
	m_header->shared_file_type_flags &= ~(1 << k_tags_shared_file_index);
	m_header->section_offsets[_cache_file_tag_section] = m_tags_data_offset;
	m_header->original_section_bounds[_cache_file_tag_section].offset = m_tags_data_offset;
	m_header->original_section_bounds[_cache_file_tag_section].size = m_tags_data_size;
	m_header->tag_table_offset = m_tags_header->tag_table_offset;
	m_header->tag_count = m_tags_header->tag_count;
	//m_tags_header->zero_out();
}

void handle_child_tags(s_cache_file_tags_header& tags_header, long index)
{
	s_cache_file_tag_instance& instance = tags_header.tag_instance_get(index);
	long* table = instance.child_tag_table();
	if (table)
	{
		for (long i = 0; i < instance.child_count; i++)
		{
			s_cache_file_tag_instance& child_instance = tags_header.tag_instance_get(table[i]);

			printf("");
		}
	}
}

void handle_tag_data(s_cache_file_tags_header& tags_header, long index)
{
	s_cache_file_tag_instance& instance = tags_header.tag_instance_get(index);
	long* table = instance.tag_data_table();
	if (table)
	{
		for (long i = 0; i < instance.tag_data_count; i++)
		{
			s_tag_data* tag_data = reinterpret_cast<s_tag_data*>(reinterpret_cast<char*>(&instance) + (table[i] & k_runtime_address_mask));

			printf("");
		}
	}
}

void handle_tag_resources(s_cache_file_tags_header& tags_header, long index)
{
	s_cache_file_tag_instance& instance = tags_header.tag_instance_get(index);
	long* table = instance.tag_resource_table();
	if (table)
	{
		for (long i = 0; i < instance.tag_resource_count; i++)
		{
			s_tag_resource_reference* resource_reference = reinterpret_cast<s_tag_resource_reference*>(reinterpret_cast<char*>(&instance) + (table[i] & k_runtime_address_mask));
			if (!resource_reference->pagable_resource)
				continue;

			//s_tag_resource* resource_page = reinterpret_cast<s_tag_resource*>(reinterpret_cast<char*>(&instance) + (resource_reference->pagable_resource & k_runtime_address_mask));

			printf("");
		}
	}
}

void handle_tag_references(s_cache_file_tags_header& tags_header, long index)
{
	s_cache_file_tag_instance& instance = tags_header.tag_instance_get(index);
	long* table = instance.tag_reference_table();
	if (table)
	{
		for (long i = 0; i < instance.tag_reference_count; i++)
		{
			s_tag_reference* tag_reference = reinterpret_cast<s_tag_reference*>(reinterpret_cast<char*>(&instance) + (table[i] & k_runtime_address_mask));
			printf("");
		}
	}
}

void c_hf2p_cache_file_converter::zero_unnused_tags()
{
	for (long tag_index = 0; tag_index < m_header->tag_count; tag_index++)
	{
		long& tag_offset = m_tags_header->tag_table()[tag_index];
		if (tag_offset == 0)
			continue;

		handle_child_tags(*m_tags_header, tag_index);
		handle_tag_data(*m_tags_header, tag_index);
		handle_tag_resources(*m_tags_header, tag_index);
		handle_tag_references(*m_tags_header, tag_index);

		s_cache_file_tag_instance& instance = m_tags_header->tag_instance_get(tag_index);
		if (instance.is_group('scnr') && tag_index != m_header->scenario_index)
		{
			tag_offset = 0;
			instance.zero_out();
		}
		
		if (instance.is_group('zone') && tag_index != m_header->cache_file_resource_gestalt_index)
		{
			tag_offset = 0;
			instance.zero_out();
		}
	}
}
