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

	for (int map_index = 2; map_index < argc; map_index++)
	{
		c_hf2p_cache_file_converter* converter = new c_hf2p_cache_file_converter(argv[1], argv[map_index]);
		if (converter->apply_changes())
			converter->write_changes_to_disk();

		delete converter;
	}

	return 0;
}

c_cache_file_header::c_cache_file_header(char* data) :
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
}



c_hf2p_cache_file_converter::c_hf2p_cache_file_converter(const char* maps_path, const char* map_name) :
	m_in_map_path(),
	m_in_map_data(),
	m_in_map_data_size(),
	m_out_map_path(),
	m_out_map_data(),
	m_out_map_data_size(),
	m_shared_file_data_paths(),
	m_shared_file_datas(),
	m_shared_file_data_sizes(), 
	m_shared_file_data_offsets(),
	m_header(),
	m_shared_file_headers()
{
	sprintf_s(m_in_map_path, "%s/%s.map", maps_path, map_name);
	sprintf_s(m_out_map_path, "%s/%s_out.map", maps_path, map_name);

	read_data_from_file(m_in_map_data, m_in_map_data_size, m_in_map_path);

	// only enable tags for now
	for (long i = _cache_file_shared_tags; i < _cache_file_shared_tags + 1; i++)
	{
		m_shared_file_data_paths[i] = new char[260]{};
		sprintf_s(m_shared_file_data_paths[i], 260, "%s/%s.dat", maps_path, k_cache_file_shared_type_names[i]);
		read_data_from_file(m_shared_file_datas[i], m_shared_file_data_sizes[i], m_shared_file_data_paths[i]);
	}

	m_out_map_data_size = cache_file_round_up_read_size(m_in_map_data_size);
	for (long i = 0; i < k_cache_file_shared_new_count; i++)
	{
		if (m_shared_file_datas[i] && m_shared_file_data_sizes[i])
		{
			m_shared_file_data_offsets[i] = m_out_map_data_size;
			m_out_map_data_size += cache_file_round_up_read_size(m_shared_file_data_sizes[i]);
		}
	}

	m_out_map_data = new char[m_out_map_data_size] {};
	memcpy(m_out_map_data, m_in_map_data, m_in_map_data_size);
	m_header = new c_cache_file_header(m_out_map_data);

	for (long i = 0; i < k_cache_file_shared_new_count; i++)
	{
		if (m_shared_file_datas[i] && m_shared_file_data_sizes[i])
		{
			memcpy(m_out_map_data + m_shared_file_data_offsets[i], m_shared_file_datas[i], m_shared_file_data_sizes[i]);
			m_shared_file_headers[i] = new c_cache_file_tags_header(m_out_map_data + m_shared_file_data_offsets[i]);
		}
	}
}

c_hf2p_cache_file_converter::~c_hf2p_cache_file_converter()
{
	if (m_in_map_data)
	{
		delete[] m_in_map_data;
		m_in_map_data = nullptr;
		m_in_map_data_size = 0;
	}

	if (m_out_map_data)
	{
		delete[] m_out_map_data;
		m_out_map_data = nullptr;
		m_out_map_data_size = 0;
	}

	for (long i = 0; i < k_cache_file_shared_new_count; i++)
	{
		if (m_shared_file_data_paths[i])
			delete[] m_shared_file_data_paths[i];

		if (m_shared_file_datas[i])
			delete[] m_shared_file_datas[i];

		m_shared_file_datas[i] = nullptr;
		m_shared_file_data_sizes[i] = 0;
	}

}

bool c_hf2p_cache_file_converter::apply_changes()
{
	if ((*m_header).file_version != 18)
	{
		printf("invalid file version\n");
		return false;
	}

	(*m_header).file_size = m_out_map_data_size;

	//add_debug_section();
	//add_resource_section();
	add_tag_section();
	//add_localization_section();
	zero_unnused_tags();

	return true;
}

bool c_hf2p_cache_file_converter::write_changes_to_disk(bool replace)
{
	if (replace)
		return write_data_to_file(m_out_map_data, m_out_map_data_size, m_in_map_path);

	return write_data_to_file(m_out_map_data, m_out_map_data_size, m_out_map_path);
}

void c_hf2p_cache_file_converter::add_debug_section()
{
	//(*m_header).section_offsets[_cache_file_debug_section] = m_debug_data_offset;
	//(*m_header).original_section_bounds[_cache_file_debug_section].offset = m_debug_data_offset;
	//(*m_header).original_section_bounds[_cache_file_debug_section].size = m_debug_data_size;
	printf("");
}

void c_hf2p_cache_file_converter::add_resource_section()
{
	if (m_shared_file_datas[_cache_file_shared_resources])
	{
		(*m_header).shared_file_type_flags &= ~(1 << _cache_file_shared_resources);
		(*m_header).section_offsets[_cache_file_resource_section] = m_shared_file_data_offsets[_cache_file_shared_resources];
		(*m_header).original_section_bounds[_cache_file_resource_section].offset = m_shared_file_data_offsets[_cache_file_shared_resources];
		(*m_header).original_section_bounds[_cache_file_resource_section].size = m_shared_file_data_sizes[_cache_file_shared_resources];
	}
}

void c_hf2p_cache_file_converter::add_tag_section()
{
	if (m_shared_file_datas[_cache_file_shared_tags])
	{
		c_cache_file_tags_header& tags_header(*m_shared_file_headers[_cache_file_shared_tags]);

		(*m_header).shared_file_type_flags &= ~(1 << _cache_file_shared_tags);
		(*m_header).section_offsets[_cache_file_tag_section] = m_shared_file_data_offsets[_cache_file_shared_tags];
		(*m_header).original_section_bounds[_cache_file_tag_section].offset = m_shared_file_data_offsets[_cache_file_shared_tags];
		(*m_header).original_section_bounds[_cache_file_tag_section].size = m_shared_file_data_sizes[_cache_file_shared_tags];
		(*m_header).tag_table_offset = tags_header.tag_table_offset;
		(*m_header).tag_count = tags_header.tag_count;
		//(*m_tags_header).zero_out();
	}
}

void c_hf2p_cache_file_converter::add_localization_section()
{
	printf("");
}

void c_cache_file_tags_header::handle_child_tags(long index)
{
	s_cache_file_tag_instance& instance = tag_instance_get(index);
	long* table = instance.child_tag_table();
	if (table)
	{
		for (long i = 0; i < instance.child_count; i++)
		{
			s_cache_file_tag_instance& child_instance = tag_instance_get(table[i]);

			printf("");
		}
	}
}

void c_cache_file_tags_header::handle_tag_data(long index)
{
	s_cache_file_tag_instance& instance = tag_instance_get(index);
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

void c_cache_file_tags_header::handle_tag_resources(long index)
{
	s_cache_file_tag_instance& instance = tag_instance_get(index);
	long* table = instance.tag_resource_table();
	if (table)
	{
		for (long i = 0; i < instance.tag_resource_count; i++)
		{
			s_tag_resource_reference* resource_reference = reinterpret_cast<s_tag_resource_reference*>(reinterpret_cast<char*>(&instance) + (table[i] & k_runtime_address_mask));
			if (!resource_reference->pagable_resource)
				continue;

			s_tag_resource* resource_page = reinterpret_cast<s_tag_resource*>(reinterpret_cast<char*>(&instance) + (resource_reference->pagable_resource & k_runtime_address_mask));

			// disable checksum
			//resource_page->flags &= ~(1 << 0);

			// remove resources.dat
			//resource_page->flags &= ~(1 << 1);

			printf("");
		}
	}
}

void c_cache_file_tags_header::handle_tag_references(long index)
{
	s_cache_file_tag_instance& instance = tag_instance_get(index);
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
	c_cache_file_tags_header& tags_header(*m_shared_file_headers[_cache_file_shared_tags]);

	for (long tag_index = 0; tag_index < (*m_header).tag_count; tag_index++)
	{
		long& tag_offset = tags_header.tag_table[tag_index];
		if (tag_offset == 0)
			continue;

		tags_header.handle_child_tags(tag_index);
		tags_header.handle_tag_data(tag_index);
		tags_header.handle_tag_resources(tag_index);
		tags_header.handle_tag_references(tag_index);

		s_cache_file_tag_instance& instance = tags_header.tag_instance_get(tag_index);
		if (instance.is_group('scnr') && tag_index != (*m_header).scenario_index)
		{
			tag_offset = 0;
			instance.zero_out();
		}
		
		if (instance.is_group('zone') && tag_index != (*m_header).cache_file_resource_gestalt_index)
		{
			tag_offset = 0;
			instance.zero_out();
		}
	}
}
