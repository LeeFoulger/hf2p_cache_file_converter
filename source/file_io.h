#pragma once

#include <cassert> // assert
#include <commapi.h> // GetOpenFileNameA, GetSaveFileNameA
#include <stdio.h> // _vfprintf_s_l, _fprintf_s_l

int print_line(bool error, const char* format, ...)
{
	int result = 0;
	FILE* stream = error ? stderr : stdout;

	va_list arg_list;
	va_start(arg_list, format);
	result = _vfprintf_s_l(stream, format, NULL, arg_list);
	result += _fprintf_s_l(stream, "\n", NULL);
	va_end(arg_list);

	return result;
}

using t_alloc_func = decltype(::malloc);
using t_free_func = decltype(::free);
using t_copy_func = decltype(::memcpy);
using t_fill_func = decltype(::memset);

// "safe"
void* safe_alloc(size_t size, t_alloc_func* alloc_func = ::malloc)
{
	assert(size);
	assert(alloc_func);

	return alloc_func(size);
}

// "safe"
void safe_free(void* buffer, t_free_func* free_func = ::free)
{
	assert(buffer);
	assert(free_func);

	free_func(buffer);
	buffer = nullptr;
}

// "safe"
void* safe_copy(void* destination, void const* source, size_t size, t_copy_func* copy_func = ::memcpy)
{
	assert(destination);
	assert(source);
	assert(size);
	assert(copy_func);

	return copy_func(destination, source, size);
}

// "safe"
void* safe_fill(void* destination, int value, size_t size, t_fill_func* fill_func = ::memset)
{
	assert(destination);
	assert(size);
	assert(fill_func);

	return fill_func(destination, value, size);
}

template<typename t_type = unsigned char>
t_type* balloc(long size)
{
	assert(size);

	return (t_type*)safe_fill(safe_alloc(size), 0, size);
}

template<typename t_type = unsigned char>
long bsize(t_type* buffer)
{
	assert(buffer);

	return (long)::_msize(buffer);
}

template<typename t_type = unsigned char>
t_type* brealloc(t_type* buffer, long size)
{
	assert(buffer);
	assert(size);

	return (t_type*)safe_copy(balloc(size), buffer, bsize(buffer));
}

template<typename t_type = unsigned char>
void bfree(t_type* buffer)
{
	safe_free(buffer);
}

void btest()
{
	unsigned char* buffer = balloc(128);
	long buffer_size = bsize(buffer);

	for (long i = 0; i < buffer_size; i++)
		buffer[i] = (unsigned char)i;

	unsigned char* new_buffer = brealloc(buffer, buffer_size * 2);

	bfree(buffer);
	bfree(new_buffer);
}

char* make_path(const char* format, va_list arg_list, bool end = true)
{
	LPSTR path = (LPSTR)::malloc(MAX_PATH);
	assert(path);
	safe_fill(path, 0, MAX_PATH);

	vsnprintf_s(path, MAX_PATH, MAX_PATH, format, arg_list);

	if (end)
		va_end(arg_list);

	return path;
}

char* make_path(const char* format, ...)
{
	va_list arg_list;
	va_start(arg_list, format);

	return make_path(format, arg_list);
}

struct s_memory_mapped_file
{
	LPSTR path;
	HANDLE file_handle;
	HANDLE file_mapping_handle;

	union
	{
		LPVOID view_of_file;
		unsigned char* file_buffer;
	};
};

struct s_memory_mapped_options
{
	struct s_create_file_options
	{
		DWORD desired_access       = GENERIC_READ;
		DWORD share_mode           = FILE_SHARE_READ;
		DWORD creation_disposition = OPEN_EXISTING;
		DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
	};

	struct s_create_file_mapping_options
	{
		DWORD protect = PAGE_READONLY;
	};

	struct s_map_view_of_file_options
	{
		DWORD desired_access = FILE_MAP_READ;
	};

	s_create_file_options create_file_options;
	s_create_file_mapping_options create_file_mapping_options;
	s_map_view_of_file_options map_view_of_file_options;
};

s_memory_mapped_options read_only_memory_mapped_options
{
	{ GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY },
	{ PAGE_READONLY },
	{ FILE_MAP_READ }
};

s_memory_mapped_options read_write_memory_mapped_options
{
	{ GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL },
	{ PAGE_READWRITE },
	{ FILE_MAP_READ | FILE_MAP_WRITE }
};

static bool file_open(s_memory_mapped_file** out_file, const char* path, bool write_access = false);
static bool file_close(s_memory_mapped_file* file);
static bool file_read(s_memory_mapped_file* file, long buffer_size, unsigned char** buffer, long offset = 0);
static bool file_write(s_memory_mapped_file* file, long buffer_size, unsigned char const* buffer, long offset = 0);
static unsigned char* file_get_buffer(s_memory_mapped_file* file, long offset = 0);
static bool file_open(s_memory_mapped_file** out_file, const char* path, bool write_access)
{
	if (!out_file || !PathFileExistsA(path))
		return false;

	s_memory_mapped_file* file = new s_memory_mapped_file();
	assert(file);

	file->path = _strdup(path);
	assert(file->path && *file->path);

	s_memory_mapped_options::s_create_file_options* create_file_options = &read_only_memory_mapped_options.create_file_options;
	s_memory_mapped_options::s_create_file_mapping_options* create_file_mapping_options = &read_only_memory_mapped_options.create_file_mapping_options;
	s_memory_mapped_options::s_map_view_of_file_options* map_view_of_file_options = &read_only_memory_mapped_options.map_view_of_file_options;

	if (write_access)
	{
		create_file_options = &read_write_memory_mapped_options.create_file_options;
		create_file_mapping_options = &read_write_memory_mapped_options.create_file_mapping_options;
		map_view_of_file_options = &read_write_memory_mapped_options.map_view_of_file_options;
	}

	file->file_handle = CreateFileA(
		path,
		create_file_options->desired_access,
		create_file_options->share_mode,
		NULL,
		create_file_options->creation_disposition,
		create_file_options->flags_and_attributes,
		NULL
	);
	assert(file->file_handle && file->file_handle != INVALID_HANDLE_VALUE);

	file->file_mapping_handle = CreateFileMappingA(
		file->file_handle,
		NULL,
		create_file_mapping_options->protect,
		0,
		0,
		NULL
	);
	assert(file->file_mapping_handle && file->file_mapping_handle != INVALID_HANDLE_VALUE);

	file->view_of_file = MapViewOfFile(
		file->file_mapping_handle,
		map_view_of_file_options->desired_access,
		0,
		0,
		0
	);
	assert(file->view_of_file);

	*out_file = file;
	return true;
}

static bool file_close(s_memory_mapped_file* file)
{
	if (!file)
		return false;

	if (file->view_of_file)
	{
		UnmapViewOfFile(file->view_of_file);
		file->view_of_file = NULL;
	}

	if (file->file_mapping_handle)
	{
		CloseHandle(file->file_mapping_handle);
		file->file_mapping_handle = NULL;
	}

	if (file->file_handle)
	{
		CloseHandle(file->file_handle);
		file->file_handle = NULL;
	}

	if (file->path)
	{
		free(file->path);
		file->path = NULL;
	}

	return true;
}

static bool file_read(s_memory_mapped_file* file, long buffer_size, unsigned char** buffer, long offset)
{
	if (!file || !file->view_of_file)
		return false;

	*buffer = new unsigned char[buffer_size];
	safe_fill(*buffer, 0, buffer_size);

	bool result = false;
	safe_copy(*buffer, file_get_buffer(file, offset), buffer_size);

	return true;
}

static bool file_write(s_memory_mapped_file* file, long buffer_size, unsigned char const* buffer, long offset)
{
	if (!file)
		return false;

	bool result = false;
	safe_copy(file_get_buffer(file, offset), buffer, buffer_size);

	return true;
}

static unsigned char* file_get_buffer(s_memory_mapped_file* file, long offset)
{
	if (!file || !file->file_buffer)
		return nullptr;

	return file->file_buffer + offset;
}

class c_memory_mapped_file
{
	using t_open_func = bool(s_memory_mapped_file** out_file, const char* path, bool write_access);
	using t_close_func = bool(s_memory_mapped_file* file);
	using t_read_func = bool(s_memory_mapped_file* file, long buffer_size, unsigned char** buffer, long offset);
	using t_write_func = bool(s_memory_mapped_file* file, long buffer_size, unsigned char const* buffer, long offset);
	using t_get_buffer_func = unsigned char*(s_memory_mapped_file* file, long offset);

public:
	c_memory_mapped_file(
		void* open_func,
		void* close_func,
		void* read_func,
		void* write_func,
		char* path = nullptr,
		bool write_access = false
	) :
		m_file(nullptr),
		m_open_func(open_func),
		m_close_func(close_func),
		m_read_func(read_func),
		m_write_func(write_func)
	{
		assert(open(path, write_access));
	}

	~c_memory_mapped_file()
	{
		assert(close());
	}

	inline bool open(const char* path, bool write_access = false)
	{
		if (!m_open_func || !path)
			return false;

		return ((t_open_func*)m_open_func)(&m_file, path, write_access);
	}

	inline bool close()
	{
		if (!m_close_func)
			return false;

		return ((t_close_func*)m_close_func)(m_file);
	}

	inline bool read(long buffer_size, unsigned char** buffer, long offset)
	{
		if (!m_read_func)
			return false;

		return ((t_read_func*)m_read_func)(m_file, buffer_size, buffer, offset);
	}

	inline bool write(long buffer_size, unsigned char const* buffer, long offset)
	{
		if (!m_write_func)
			return false;

		return ((t_write_func*)m_write_func)(m_file, buffer_size, buffer, offset);
	}

	inline unsigned char* get_buffer(long offset = 0)
	{
		if (!m_get_buffer_func)
			return nullptr;

		return ((t_get_buffer_func*)m_get_buffer_func)(m_file, offset);
	}

protected:
	s_memory_mapped_file* m_file;

private:
	void* m_open_func;
	void* m_close_func;
	void* m_read_func;
	void* m_write_func;
	void* m_get_buffer_func;
};

class c_read_only_file : public c_memory_mapped_file
{
public:
	c_read_only_file(const char* format, ...) :
		c_memory_mapped_file(file_open, file_close, file_read, nullptr)
	{
		va_list arg_list;
		va_start(arg_list, format);
		char* path = make_path(format, arg_list);
		assert(open(path));
		free(path);
	}
};

inline bool read_data_from_file(unsigned char*& out_data, long& out_size, const char* filename)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filename, "rb"), file != nullptr)
	{
		fseek(file, 0, SEEK_END);
		out_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		out_data = new unsigned char[out_size];
		fread(out_data, sizeof(unsigned char), out_size, file);

		fclose(file);
		return true;
	}

	return false;
}

inline bool write_data_to_file(unsigned char* data, long size, const char* filename)
{
	FILE* file = nullptr;
	if (fopen_s(&file, filename, "wb"), file != nullptr)
	{
		fwrite(data, sizeof(unsigned char), size, file);

		fclose(file);
		return true;
	}

	return false;
}

template<long k_size>
inline bool write_data_to_file(unsigned char(&data)[k_size], const char* filename)
{
	return write_data_to_file(data, k_size, filename);
}

enum e_get_file_name_type
{
	_get_file_name_type_open = 0,
	_get_file_name_type_save,

	k_get_file_name_type_count
};

inline bool get_file_name(const char* filter, const char* default_extension, char*& buffer, long maximum_file_count, const char* initial_dir, e_get_file_name_type get_file_name_type)
{
	buffer = new char[(MAX_PATH * maximum_file_count) + 1]{};

	OPENFILENAMEA open_file_struct{};
	open_file_struct.lStructSize = sizeof(open_file_struct);
	open_file_struct.hwndOwner = NULL;
	open_file_struct.hInstance = GetModuleHandleA(NULL);
	open_file_struct.lpstrFilter = filter;
	open_file_struct.lpstrFile = buffer;
	open_file_struct.nMaxFile = MAX_PATH * maximum_file_count;
	open_file_struct.lpstrInitialDir = initial_dir;
	open_file_struct.lpstrDefExt = default_extension;
	open_file_struct.Flags = OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	switch (get_file_name_type)
	{
	case _get_file_name_type_open: return !!GetOpenFileNameA(&open_file_struct);
	case _get_file_name_type_save: return !!GetSaveFileNameA(&open_file_struct);
	}

	return false;
}

inline long create_filter(const char* text, long text_size, char** buffer, bool null_char = true)
{
	if (!buffer)
		return 0;

	long buffer_size = 2 * text_size;
	if (null_char)
		buffer_size++;

	*buffer = new char[buffer_size] {};

	safe_copy(*buffer, text, text_size);
	safe_copy(*buffer + text_size, text, text_size);

	return buffer_size;
}

inline long create_filter(const char* text, char** buffer, bool null_char = true)
{
	return create_filter(text, (long)strlen(text) + 1, buffer, null_char);
}

inline char* create_filters(const char** strings, long count)
{
	char** filters = new char* [count] {};
	long* filter_sizes = new long[count] {};

	long buffer_size = 1;
	for (long i = 0; i < count; i++)
	{
		filter_sizes[i] = create_filter(strings[i], &filters[i], false);
		buffer_size += filter_sizes[i];
	}
	char* buffer = new char[buffer_size] {};

	long offset = 0;
	for (long i = 0; i < count; offset += filter_sizes[i++])
	{
		safe_copy(buffer + offset, filters[i], filter_sizes[i]);
		delete[] filters[i];
		filters[i] = nullptr;
	}
	delete[] filters;

	return buffer;
}

template<long k_count>
inline char* create_filters(const char* (&strings)[k_count])
{
	return create_filters(strings, k_count);
}
