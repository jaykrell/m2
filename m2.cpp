
//#include "config.h"

namespace mxx
{
// First we need to be able to read the files.

unsigned short unpack2le(const void *a)
{
	auto b = (unsigned char*)a;
	return b[1] * 256 + b[0];
}

unsigned int unpack4le(const void *a)
{
	return unpack2le(a) + unpack2le((char*)a + 2);
}

unsigned unpack(char (&a)[2]) { return a[1] * 256 | a[0]; }
unsigned unpack(char (&a)[4]) { return a[1] * 256 | a[0]; }

struct dos_header_t
{
	union {
		struct {
			short mz;
			char pad[64-6];
			int pe;
		};
		char a[64];
	};
	
	bool check_sig() { return a[0] == 'M' && a[1] == 'Z'; }
	unsigned get_pe() { return unpack4le(&a[60]); }
};

struct file_header_t
{
	union {
		struct {
			short machine;
			short section_count;
			int   timedate;
			int   symbols;
			int   symbol_count;
			short optional_header_size;
			short characteristics;
		};
		char a[20];
	};
};

struct optional_header32_t
{
	union {
		struct {
			short magic;
			char major;
			char minor;
			int code_size;
			int init_data_size;
			int uninit_data_size;
			int entry;
			int code_base;
			int data_base;
			int image_base;
			int section_align; // in memory
			int file_align; // in file
			short os_major;
			short os_minor;
			short user_major;
			short user_minor;
			short subsys_major
			short subsys_minor;
			int reserved;
			int image_size;
			int header_size;
			int checksum;
			short dll_flags;
			int stack_reserve;
			int stack_commit;
			int heap_reserve;
			int heap_commit;
			int loader_flags;
			int number_of_data;
			image_data_t image_data]16]; // [number_of_data]
		};
		char a[96+128];
	};
};

struct optional_header64
{
	union {
		struct {
			short magic;
			char major;
			char minor;
			int code_size;
			int init_data_size;
			int uninit_data_size;
			int entry;
			int code_base;
			int data_base;
			int image_base;
			int section_align; // in memory
			int file_align; // in file
			short os_major;
			short os_minor;
			short user_major;
			short user_minor;
			short subsys_major
			short subsys_minor;
			int reserved;
			int image_size;
			int header_size;
			int checksum;
			short dll_flags;
			int stack_reserve;
			int stack_commit;
			int heap_reserve;
			int heap_commit;
			int loader_flags;
			int number_of_data;
			image_data_t image_data]16]; // [number_of_data]
		};
		char a[20];
	};
};

};
