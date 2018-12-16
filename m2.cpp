// 2-clause BSD license unless that does not suffice
// else MIT like mono.

//#include "config.h"

#define _DARWIN_USE_64_BIT_INODE

#include <stdint.h>
#include <assert.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#include <vector> // TODO rewrite/rename

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

// integral typedefs from Modula-3 m3core.h.
#if UCHAR_MAX == 0x0FFUL
typedef   signed char        int8;
typedef unsigned char       uint8;
#else
#error unable to find 8bit integer
#endif
#if USHRT_MAX == 0x0FFFFUL
typedef          short      int16;
typedef unsigned short     uint16;
#else
#error unable to find 16bit integer
#endif
#if UINT_MAX == 0x0FFFFFFFFUL
typedef          int        int32;
typedef unsigned int       uint32;
#elif ULONG_MAX == 0x0FFFFFFFFUL
typedef          long       int32;
typedef unsigned long      uint32;
#else
#error unable to find 32bit integer
#endif
#if defined (_MSC_VER) || defined (__DECC) || defined (__DECCXX) || defined (__int64)
typedef          __int64    int64;
typedef unsigned __int64   uint64;
#else
typedef          long long  int64;
typedef unsigned long long uint64;
#endif

typedef unsigned char boolean;
#if defined (__cplusplus) || __STDC__
typedef void* pointer;
#else
typedef char* pointer;
#endif

// WORD_T/INTEGER are always exactly the same size as a pointer.
// VMS sometimes has 32bit size_t/ptrdiff_t but 64bit pointers.
//
// commented out is correct, but so is the #else */
//#if defined (_WIN64) || __INITIAL_POINTER_SIZE == 64 || defined (__LP64__) || defined (_LP64)*/
#if __INITIAL_POINTER_SIZE == 64
typedef int64 integer;
typedef uint64 word_t;
#else
typedef ptrdiff_t integer;
typedef size_t word_t;
#endif

/* longint is always signed and exactly 64 bits. */
typedef int64 longint;

namespace m2
{

std::string
string_vformat (const char *format, va_list va)
{
	// TODO no double buffer
	// TODO Win32?
	// TOOD rewrite
	// TODO %x
	va_list va2;
	va_copy (va2, va);
	std::vector <char> buf (1 + vsnprintf (0, 0, format, va));
	vsnprintf (&buf [0], buf.size (), format, va2);
	va_end (va);
	va_end (va2);
	return std::string (&buf [0]);
}

std::string
string_format (const char *format, ...)
{
	va_list va;
	va_start (va, format);
	std::string a = string_vformat (format, va);
	va_end (va);
	return a;
}

}

void
assertf_failed (const char * format, ...)
{
	va_list args;
	va_start (args, format);
	fputs (("assertf_failed:" + m2::string_vformat (format, args) + "\n").c_str (), stderr);
	assert (0);
	abort ();
}

void
assert_failed (const char * expr)
{
	fprintf (stderr, "assert_failed:%s\n", expr);
	assert (0);
	abort ();
}

#define release_assert(x)     ((x) || (assert_failed (!#x), (int)0))
#define release_assertf(x, y) ((x) || ((assertf_failed y), 0))
#ifdef NDEBUG
#define assertf(x, y)      	/* nothing */
#else
#define assertf(x, y)       	(release_assertf (x, y))
#endif

namespace m2
{

void
throw_int (int i, const char* a = "")
{
	throw string_format ("error %d %s\n", i, a);

}

void
throw_errno (const char* a = "")
{
	throw_int (errno, a);

}

#ifdef _WIN32
void
throw_LastError (const char* a = "")
{
	throw_int (GetLastError (), a);

}
#endif

uint16
unpack2le (const void *a)
{
	uchar* b = (uchar*)a;
	return b [1] * 256 + b [0];
}

uint32
unpack4le (const void *a)
{
	return unpack2le (a) + unpack2le ((char*)a + 2);
}

unsigned
unpack (char (&a)[2]) { return a[1] * 256 | a[0]; }

unsigned
unpack (char (&a)[4]) { return a[1] * 256 | a[0]; }

struct image_dos_header_t
{
	union {
		struct {
			uint16 mz;
			uint8 pad [64-6];
			uint32 pe;
		};
		char a [64];
	};
	
	bool check_sig () { return a [0] == 'M' && a [1] == 'Z'; }
	unsigned get_pe () { return unpack4le (&a [60]); }
};

struct image_file_header_t
{
	uint16 Machine;
	uint16 NumberOfSections;
	uint32 TimeDateStamp;
	uint32 PointerToSymbolTable;
	uint32 NumberOfSymbols;
	uint16 SizeOfOptionalHeader;
	uint16 Characteristics;
};

struct image_data_directory_t
{
	uint32 VirtualAddress;
	uint32 Size;
};

struct image_optional_header32
{
	uint16 Magic;
	uint8  MajorLinkerVersion;
	uint8  MinorLinkerVersion;
	uint32 SizeOfCode;
	uint32 SizeOfInitializedData;
	uint32 SizeOfUninitializedData;
	uint32 AddressOfEntryPoint;
	uint32 BaseOfCode;
	uint32 BaseOfData;
	uint32 ImageBase;
	uint32 SectionAlignment;
	uint32 FileAlignment;
	uint16 MajorOperatingSystemVersion;
	uint16 MinorOperatingSystemVersion;
	uint16 MajorImageVersion;
	uint16 MinorImageVersion;
	uint16 MajorSubsystemVersion;
	uint16 MinorSubsystemVersion;
	uint32 Win32VersionValue;
	uint32 SizeOfImage;
	uint32 SizeOfHeaders;
	uint32 CheckSum;
	uint16 Subsystem;
	uint16 DllCharacteristics;
	uint32 SizeOfStackReserve;
	uint32 SizeOfStackCommit;
	uint32 SizeOfHeapReserve;
	uint32 SizeOfHeapCommit;
	uint32 LoaderFlags;
	uint32 NumberOfRvaAndSizes;
	image_data_directory_t DataDirectory[16];
};

struct image_optional_header64
{
	uint16 Magic;
	uint8  MajorLinkerVersion;
	uint8  MinorLinkerVersion;
	uint32 SizeOfCode;
	uint32 SizeOfInitializedData;
	uint32 SizeOfUninitializedData;
	uint32 AddressOfEntryPoint;
	uint32 BaseOfCode;
	uint64 ImageBase;
	uint32 SectionAlignment;
	uint32 FileAlignment;
	uint16 MajorOperatingSystemVersion;
	uint16 MinorOperatingSystemVersion;
	uint16 MajorImageVersion;
	uint16 MinorImageVersion;
	uint16 MajorSubsystemVersion;
	uint16 MinorSubsystemVersion;
	uint32 Win32VersionValue;
	uint32 SizeOfImage;
	uint32 SizeOfHeaders;
	uint32 CheckSum;
	uint16 Subsystem;
	uint16 DllCharacteristics;
	uint64 SizeOfStackReserve;
	uint64 SizeOfStackCommit;
	uint64 SizeOfHeapReserve;
	uint64 SizeOfHeapCommit;
	uint32 LoaderFlags;
	uint32 NumberOfRvaAndSizes;
	image_data_directory_t DataDirectory[16];
};

struct image_nt_headers_t
{
	uint32 Signature;
	image_file_header_t FileHeader;
	char OptionalHeader;
};

struct image_section_header_t
{
	uint8 Name [8];
	union {
		uint32 PhysicalAddress;
		uint32 VirtualSize;
	} Misc;
	uint32 VirtualAddress;
	uint32 SizeOfRawData;
	uint32 PointerToRawData;
	uint32 PointerToRelocations;
	uint32 PointerToLinenumbers;
	uint16 NumberOfRelocations;
	uint16 NumberOfLinenumbers;
	uint32 Characteristics;
};

// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
	typedef void (explicit_operator_bool::*T) () const;
	void True () const;
};

typedef void (explicit_operator_bool::*bool_type) () const;

#ifdef _WIN32
struct handle_t
{
	void * h;

	handle_t (void *a = 0) : h (a) { }

	void* get () { return h; }

	bool valid () const { return static_valid (h); }

	static bool static_valid (void* h) { return h && h != INVALID_HANDLE_VALUE; }

	operator void* () { return get (); }

	static void static_cleanup (void* h)
	{
		if (!static_valid (h)) return;
		CloseHandle (h);
	}

	void* detach ()
	{
		void* const a = h;
		h = 0;
		return a;
	}

	void cleanup ()
	{
		static_cleanup (detach ());
	}

	handle_t& operator= (void* a)
	{
		if (h == a) return *this;
		cleanup ();
		h = a;
		return *this;
	}

#if 0 // C++11
	explicit operator bool () { return valid (); } // C++11
#else
	operator explicit_operator_bool::T () const
	{
		return valid () ? &explicit_operator_bool::True : NULL;
	}
#endif

	bool operator ! () { return !valid (); }

	handle_t () : h (0) { }

	~handle_t ()
	{
		if (valid ()) CloseHandle (h);
		h = 0;
	}
};
#endif

struct fd_t
{
	int fd;

#if 0 // C++11
	explicit operator bool () { return valid (); } // C++11
#else
	operator explicit_operator_bool::T () const
	{
		return valid () ? &explicit_operator_bool::True : NULL;
	}
#endif

	bool operator ! () { return !valid (); }

	operator int () { return get (); }
	static bool static_valid (int fd) { return fd != -1; }
	int get () const { return fd; }
	bool valid () const { return static_valid (fd); }

	static void static_cleanup (int fd)
	{
		if (!static_valid (fd)) return;
#ifdef _WIN32
		_close (fd);
#else
		close (fd);
#endif
	}

	int detach ()
	{
		int const a = fd;
		fd = -1;
		return a;
	}

	void cleanup ()
	{
		static_cleanup (detach ());
	}

	fd_t (int a = -1) : fd (a) { }

	fd_t& operator= (int a)
	{
		if (fd == a) return *this;
		cleanup ();
		fd = a;
		return *this;
	}

	~fd_t ()
	{
		cleanup ();
	}
};

struct memory_mapped_file_t
{
// TODO allow for redirection to built-in data
// TODO allow for systems that must read, not mmap
	void * base;
	size_t size;

	memory_mapped_file_t () : base (0), size (0) { }

	~memory_mapped_file_t ()
	{
		if (!base) return;
#ifdef _WIN32
		UnmapViewOfFile (base);
#else
		munmap (base, size);
#endif
		base = 0;
	}
	void read (const char* a)
	{
#ifdef _WIN32
		handle_t h = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (!h) throw_LastError (string_format ("CreateFileA(%s)", a).c_str ());
		LARGE_INTEGER b;
		b.QuadPart = 0;
		if (!GetFileSizeEx (h, &b)) throw_LastError (string_format ("GetFileSizeEx(%s)", a).c_str());
		// FIXME check for size==0
		size = (size_t)b.QuadPart;
		handle_t h2 = CreateFileMappingW (h, 0, PAGE_READONLY, 0, 0, 0);
		if (!h2) throw_LastError (string_format ("CreateFileMapping(%s)", a).c_str ());
		base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
		if (!base) throw_LastError (string_format ("MapViewOfFile(%s)", a).c_str ());
#else
		fd_t fd = open (a, O_RDONLY);
		if (!fd) throw_errno (string_format ("open(%s)", a).c_str ());
		struct stat st; // FIXME? stat64
		memset (&st, 0, sizeof (st));
		if (fstat (fd, &st)) throw_errno (string_format ("fstat(%s)", a).c_str ());
		size = st.st_size;
		base = mmap (0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (base == MAP_FAILED) throw_errno (string_format ("mmap(%s)", a).c_str ());
#endif
	}
};

image_section_header_t*
image_first_section (image_nt_headers_t* base)
{
	return (image_section_header_t*)((char*)&base->OptionalHeader + base->FileHeader.SizeOfOptionalHeader);
}

}

using namespace m2;

int
main (int argc, char** argv)
{
	memory_mapped_file_t mmf;
#define X(x) printf ("%s %#x\n", #x, (int)x)
X (sizeof (image_dos_header_t));
X (sizeof (image_file_header_t));
X (sizeof (image_nt_headers_t));
X (sizeof (image_section_header_t));
#undef X
	try
	{
		const char *file_name = argv [1];
		mmf.read (file_name);
		void * base = mmf.base;
		image_dos_header_t* dos = (image_dos_header_t*)base;
		printf ("mz: %02x%02x\n", ((uchar*)dos) [0], ((uchar*)dos) [1]);
		if (memcmp (base, "MZ", 2))
			throw string_format ("incorrect MZ signature %s", file_name);
		printf ("mz: %c%c\n", ((char*)dos) [0], ((char*)dos) [1]);
		uint pe_offset = dos->get_pe ();
		printf ("pe_offset: %#x\n", pe_offset);
		uchar* pe = (pe_offset + (uchar*)base);
		printf ("pe: %02x%02x%02x%02x\n", pe [0], pe [1], pe [2], pe [3]);
		if (memcmp (pe, "PE\0\0", 4))
			throw string_format ("incorrect PE00 signature %s", file_name);
		printf ("pe: %c%c\\%d\\%d\n", pe [0], pe [1], pe [2], pe [3]);
		image_nt_headers_t *nt = (image_nt_headers_t*)pe;
		printf ("machine:%x\n", nt->FileHeader.Machine);
		image_optional_header32 *opt32 = (image_optional_header32*)(&nt->OptionalHeader);
		image_optional_header64 *opt64 = (image_optional_header64*)(&nt->OptionalHeader);
		int opt_magic = opt32->Magic;
		release_assertf ((opt_magic == 0x10b && !(opt64 = 0)) || (opt_magic == 0x20b && !(opt32 = 0)), ("file:%s opt_magic:%x", file_name, opt_magic));
		printf ("opt.magic:%x opt32:%p opt64:%p\n", opt_magic, opt32, opt64);
		printf ("opt.rvas:%X\n", opt32 ? opt32->NumberOfRvaAndSizes : opt64->NumberOfRvaAndSizes);
	}
	catch (int er)
	{
		fprintf (stderr, "error %d\n", er);
	}
	catch (const std::string& er)
	{
		fprintf (stderr, "error %s\n", er.c_str());
	}
}
