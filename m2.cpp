// 2-clause BSD license unless that does not suffice
// else MIT like mono. Need to research the difference.

// https://www.ntcore.com/files/dotnetformat.htm
// https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf.

// Implementation language is C++. At least C+11.
// The following features of C++ are desirable:
//   RAII (destructors, C++98)
//   enum class (C++11)
//   std::size (C++17)
//   std::string::data (direct sprintf into std::string) (C++17)
//   non-static member initialization (C++11)
//   thread safe static initializers, maybe (C++11)
//   char16 (C++11, but could use C++98 unsigned short)
//   explicit operator bool (C++11 but easy to emulate in C++98)
//   variadic template (C++11, really needed?)
//   variadic macros (really needed?)
//
// C++ library dependencies are likely to be removed, but we'll see.

// Goals: clarity, simplicity, portability, size, interpreter, compile to C++, and maybe
// later some JIT

#define _CRT_SECURE_NO_WARNINGS 1

//#include "config.h"
#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#define _cpp_max max // old compiler/library compat
#define _cpp_min min // old compiler/library compat

#ifndef HAS_TYPED_ENUM
#if __cplusplus >= 201103L || _MSC_VER >= 1500 // TODO test more compilers
#define HAS_TYPED_ENUM 1
#else
#define HAS_TYPED_ENUM 0
#endif
#endif

#ifdef _MSC_VER

// We have to include yvals.h early because it breaks warnings.
#pragma warning(disable:4820) // padding
#if _MSC_VER <= 1100
#include <yvals.h>
#endif
// TODO These warnings are in standard headers and not in our code.
#if _MSC_VER <= 1100 // TODO test more compilers/libraries
#pragma warning(disable:4018) // unsigned/signed mismatch
#pragma warning(disable:4365) // unsigned/signed mismatch
#pragma warning(disable:4244) // integer conversion
#endif
#pragma warning(disable:4615) // not a valid warning (depends on compiler version)
#pragma warning(disable:4619) // not a valid warning (depends on compiler version)
#pragma warning(disable:4663) // c:\msdev\50\VC\INCLUDE\iosfwd(132) : warning C4663: C++ language change: to explicitly specialize class template 'char_traits' use the following syntax:
                              // template<> struct char_traits<unsigned short>
#pragma warning(disable:4100) // unused parameter
#pragma warning(disable:4146) // unary minus unsigned is still unsigned (xlocnum)
#pragma warning(disable:4201) // non standard extension : nameless struct/union (windows.h)
#pragma warning(disable:4238) // non standard extension : class rvalue as lvalue (utility)
#pragma warning(disable:4480) // non standard extension : typed enums
#pragma warning(disable:4510) // function could not be generated
#pragma warning(disable:4511) // function could not be generated
#pragma warning(disable:4512) // function could not be generated
#pragma warning(disable:4513) // function could not be generated
#pragma warning(disable:4514) // unused function
#pragma warning(disable:4610) // cannot be instantiated
#pragma warning(disable:4616) // not a valid warning
#pragma warning(disable:4623) // default constructor deleted
#pragma warning(disable:4626) // assignment implicitly deleted
#pragma warning(disable:4706) // assignment within conditional
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:5027) // move assignment implicitly deleted
#if _MSC_VER > 1100 // TODO test more compilers
#pragma warning(push)
#endif
#pragma warning(disable:4480) // specifying enum type
#pragma warning(disable:4571) // catch(...)
#pragma warning(disable:4625) // copy constructor implicitly deleted
#pragma warning(disable:4668) // #if not_defined is #if 0
#pragma warning(disable:4774) // printf used without constant format
#pragma warning(disable:5026) // move constructor implicitly deleted
#pragma warning(disable:5039) // exception handling and function pointers
#endif

#ifdef __clang__
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif
#include <memory.h>

#include <stddef.h>
#include <memory.h>
#include <assert.h>
#include <string>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include <algorithm> // TODO? remove STL dependency?
#ifdef _WIN32
#define NOMINMAX 1
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#include <vector> // TODO? remove STL dependency?
#include <string> // TODO? remove STL dependency?
#if _MSC_VER
#include <malloc.h>
#endif

#if _MSC_VER > 1100 // TODO test more compilers
#pragma warning(pop)
#endif

//#if !defined(PRIX64)
//#if defined(_WIN32)
//#define PRIX64 "I64X"
//#elif defined(_ILP64) || defined(__LP64__)
//#define PRIX64 "lX"
//#else
//#define PRIX64 "llX"
//#endif
//#endif

typedef unsigned char uchar;
//typedef unsigned short ushort;
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4668) // #if not_defined is #if 0
#endif

#if defined(_ILP64) || __BITS_PER_LONG == 64 || __WORDSIZE == 64
typedef          long  int64;
typedef unsigned long uint64;
#else
#if defined (_MSC_VER) || defined (__DECC) || defined (__DECCXX) || defined (__int64)
typedef          __int64    int64;
typedef unsigned __int64   uint64;
#else
typedef          long long  int64;
typedef unsigned long long uint64;
#endif
#endif

// VMS sometimes has 32bit size_t/ptrdiff_t but 64bit pointers.
//
// commented out is correct, but so is the #else */
//#if defined (_WIN64) || __INITIAL_POINTER_SIZE == 64 || defined (__LP64__) || defined (_LP64)*/
#if __INITIAL_POINTER_SIZE == 64
//typedef int64 intptr;
//typedef uint64 uintptr;
#else
//typedef ptrdiff_t intptr;
//typedef size_t uintptr;
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace m2
{

/* This does not compile with Visual C++ 5.0 compiler/library.
#include <vector>

namespace a
{
struct b { };
struct c { vector<a::b> d; };
}

c:\msdev\50\VC\INCLUDE\vector(103) : error C2065: 'b' : undeclared identifier
c:\msdev\50\VC\INCLUDE\vector(103) : error C2440: 'default argument' : cannot convert from 'int' to 'const struct a::b &'
                                                  Reason: cannot convert from 'int' to 'const struct a::b'
                                                  No constructor could take the source type, or constructor overload resolution was ambiguous
	void resize(size_type _N, const _Ty& _X = _Ty()) // 103

There are two problems.
Workaround either by not using namespaces or having local vector without that default construction.
Adding a constructor from int, and then a default constructor helps, but does not fix the entire problem.
*/
#if _MSC_VER == 1100
template <typename T> struct vector : std::vector { };
#else
template <typename T> struct vector : std::vector<T> { };
#endif

// Portable to old (and new) Visual C++ runtime.
uint
string_vformat_length (const char *format, va_list va)
{
#if !_MSC_VER
    return 2 + vsnprintf (0, 0, format, va);
#else
    // newer runtime: _vscprintf (format, va);
    // else loop until it fits, getting -1 while it does not.
    int n = 0;
    for (;;)
    {
        int inc = n ? n : 64;
        if (_vsnprintf((char*)_alloca(inc), n += inc, format, va) != -1)
            return n + 2;
    }
#endif
}

std::string
string_vformat (const char *format, va_list va)
{
    // Some systems, including Linux/amd64, cannot consume a
    // va_list multiple times. It must be copied first.
    // Passing the parameter twice does not work.
#ifndef _MSC_VER
    va_list va2;
#ifdef __va_copy
    __va_copy (va2, va);
#else
    va_copy (va2, va); // C99
#endif
#endif
    std::vector<char> s((size_t)string_vformat_length(format, va));
#if _MSC_VER
    _vsnprintf (&s [0], s.size(), format, va);
#else
    vsnprintf (&s [0], s.size(), format, va2);
#endif
    return &s [0];
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

#define not_implemented_yet() (assertf (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

void
throw_string (const std::string& a)
{
    //fprintf (stderr, "%s\n", a.c_str());
    throw a;
    //abort ();
}

void
throw_int (int i, const char* a = "")
{
    throw_string (string_format ("error 0x%08X %s\n", i, a));
}

void
throw_errno (const char* a = "")
{
    throw_int (errno, a);
}

#ifdef _WIN32
void
throw_Win32Error (int err, const char* a = "")
{
    throw_int (err, a);

}
void
throw_GetLastError (const char* a = "")
{
    throw_int ((int)GetLastError (), a);

}
#endif

void
assertf_failed (const char* condition, const std::string& extra)
{
    //fputs (("assertf_failed:" + std::string (condition) + ":" + m2::string_vformat (format, args) + "\n").c_str (), stderr);
    //assert (0);
    //abort ();
    throw_string ("assert_failed:" + std::string (condition) + ":" + extra + "\n");
}

void
assert_failed (const char * expr)
{
    fprintf (stderr, "assert_failed:%s\n", expr);
    assert (0);
    abort ();
}

#define release_assert(x)     ((x) || (assert_failed (!#x), (int)0))
#define release_assertf(x, extra) ((x) || (assertf_failed (#x, string_format extra), 0))
#ifdef NDEBUG
#define assertf(x, y)          /* nothing */
#else
#define assertf           release_assertf
#endif

uint16
unpack2le (const void *a)
{
    uchar* b = (uchar*)a;
    uint16 c = b [1];
    c <<= 8;
    c |= b[0];
    return c;
}

uint32
unpack4le (const void *a)
{
    uint32 c = unpack2le ((char*)a + 2);
    c <<= 16;
    c |= unpack2le (a);
    return c;
}

uint32
unpack_2_or_4le (const void *a, uint size)
{
    switch (size)
    {
    case 2: return unpack2le(a);
    case 4: return unpack4le(a);
    }
    release_assertf(size == 2 || size == 4, ("%X", size));
    return ~0u;
}

unsigned
unpack (char (&a)[2])
{
    return unpack2le (a);
}

unsigned
unpack (char (&a)[4])
{
    return unpack4le (a);
}

struct image_dos_header_t
{
    union {
        struct {
            uint16 mz;
            uint8 pad [64-6];
            uint32 pe;
        } b;
        char a [64];
    } u;

    bool check_sig () { return u.a [0] == 'M' && u.a [1] == 'Z'; }
    unsigned get_pe () { return unpack4le (&u.a [60]); }
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

struct image_optional_header32_t
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
    image_data_directory_t DataDirectory[1];
};

struct image_optional_header64_t
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
    image_data_directory_t DataDirectory[1];
};

struct image_section_header_t;

struct image_nt_headers_t
{
    uint32 Signature;
    image_file_header_t FileHeader;
    char OptionalHeader;

    image_section_header_t*
    first_section_header ()
    {
        return (image_section_header_t*)((char*)&OptionalHeader + FileHeader.SizeOfOptionalHeader);
    }
};

struct image_section_header_t
{
    // In very old images, VirtualSize is zero, in which case, use SizeOfRawData.
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
struct Handle_t
{
    // TODO Handle_t vs. win32file_t, etc.

    uint64 get_file_size (const char * file_name = "")
    {
        DWORD hi = 0;
        DWORD lo = GetFileSize(h, &hi);
        if (lo == INVALID_FILE_SIZE)
        {
            DWORD err = GetLastError();
            if (err != NO_ERROR)
                throw_Win32Error (err, string_format ("GetFileSizeEx(%s)", file_name).c_str());
        }
        return (((uint64)hi) << 32) | lo;
    }

    void * h;

    Handle_t (void *a) : h (a) { }
    Handle_t () : h (0) { }

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

    Handle_t& operator= (void* a)
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

    ~Handle_t ()
    {
        if (valid ()) CloseHandle (h);
        h = 0;
    }
};
#endif

struct fd_t
{
    int fd;

#ifndef _WIN32
    uint64 get_file_size (const char * file_name = "")
    {
#ifdef __CYGWIN__
        struct stat st = { 0 }; // TODO test more systems
        if (fstat (fd, &st))
#else
        struct stat64 st = { 0 }; // TODO test more systems
        if (fstat64 (fd, &st))
#endif
            throw_errno (string_format ("fstat(%s)", file_name).c_str ());
        return st.st_size;
    }
#endif

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
// TODO allow for redirection to built-in data (i.e. filesystem emulation with builtin BCL)
// TODO allow for systems that must read, not mmap
    void * base;
    size_t size;
#ifdef _WIN32
    Handle_t file;
#else
    fd_t file;
#endif
    memory_mapped_file_t () : base(0), size(0) { }

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
        file = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (!file) throw_GetLastError (string_format ("CreateFileA(%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size(a);
        Handle_t h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
        if (!h2) throw_GetLastError (string_format ("CreateFileMapping(%s)", a).c_str ());
        base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
        if (!base) throw_GetLastError (string_format ("MapViewOfFile(%s)", a).c_str ());
#else
        file = open (a, O_RDONLY);
        if (!file) throw_errno (string_format ("open(%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size(a);
        base = mmap (0, size, PROT_READ, MAP_PRIVATE, file, 0);
        if (base == MAP_FAILED) throw_errno (string_format ("mmap(%s)", a).c_str ());
#endif
    }
};

// Apart from the persistent metadata, an in-memory pointer-ful type system is required,
// unless the metadata can aggressively be used as the in-memory data, which does not
// seem likely.

enum NativeType_t
{
    NativeType_Boolean = 2,
    NativeType_I1 = 3
    // TODO
};

struct Param_t;
struct Field_t;
struct Property_t;
struct TypeDef_t;
struct MethodRef_t;
struct MethodDef_t;
struct Assembly_t;

struct String_t : std::string
{
};

#ifdef _WIN32
typedef wchar_t char16;
#else
typedef unsigned short char16;
#endif

typedef std::basic_string<char16> ustring;

struct UString_t : ustring
{
};

struct Blob_t
{
    void* data;
    uint32 size;
};

struct Member_t
{
    std::string name;
    uint64 offset;
};

union Parent_t // Constant table0x0B
{
    Param_t* Param;
    Field_t* Field;
    Property_t* Property;
};

struct CustomAttributeType_t
{
    //union
    MethodDef_t* MethodDef;
    MethodRef_t* MethodRef;
};

typedef void *voidp;
typedef struct _Unused_t { } *Unused_t;

struct HasDeclSecurity_t
{
    //union
    TypeDef_t* TypeDef;
    MethodDef_t* MethodDef;
    Assembly_t *Assembly;
};

union HasCustomAttribute_t
{
    voidp TODO;
};

#if HAS_TYPED_ENUM
#define BEGIN_ENUM(name, type) enum name : type
#define END_ENUM(name, type) ;
#else
#define BEGIN_ENUM(name, type) enum _ ## name
#define END_ENUM(name, type) ; typedef type name;
#endif

BEGIN_ENUM(MethodDefFlags_t, uint16) // table0x06
{
//TODO bitfields (need to test little and big endian)
//TODO or bitfield decoder
    // member access mask - Use this mask to retrieve accessibility information.
    MethodDefFlags_MemberAccessMask          =   0x0007,
    MethodDefFlags_PrivateScope              =   0x0000,     // Member not referenceable.
    MethodDefFlags_Private                   =   0x0001,     // Accessible only by the parent type.
    MethodDefFlags_FamANDAssem               =   0x0002,     // Accessible by sub-types only in this Assembly.
    MethodDefFlags_Assem                     =   0x0003,     // Accessibly by anyone in the Assembly.
    MethodDefFlags_Family                    =   0x0004,     // Accessible only by type and sub-types.
    MethodDefFlags_FamORAssem                =   0x0005,     // Accessibly by sub-types anywhere, plus anyone in assembly.
    MethodDefFlags_Public                    =   0x0006,     // Accessibly by anyone who has visibility to this scope.
    // end member access mask

    // method contract attributes.
    MethodDefFlags_Static                    =   0x0010,     // Defined on type, else per instance.
    MethodDefFlags_Final                     =   0x0020,     // Method may not be overridden.
    MethodDefFlags_Virtual                   =   0x0040,     // Method virtual.
    MethodDefFlags_HideBySig                 =   0x0080,     // Method hides by name+sig, else just by name.

    // vtable layout mask - Use this mask to retrieve vtable attributes.
    MethodDefFlags_VtableLayoutMask          =   0x0100,
    MethodDefFlags_ReuseSlot                 =   0x0000,     // The default.
    MethodDefFlags_NewSlot                   =   0x0100,     // Method always gets a new slot in the vtable.
    // end vtable layout mask

    // method implementation attributes.
    MethodDefFlags_CheckAccessOnOverride     =   0x0200,     // Overridability is the same as the visibility.
    MethodDefFlags_Abstract                  =   0x0400,     // Method does not provide an implementation.
    MethodDefFlags_SpecialName               =   0x0800,     // Method is special. Name describes how.

    // interop attributes
    MethodDefFlags_PinvokeImpl               =   0x2000,     // Implementation is forwarded through pinvoke.
    MethodDefFlags_UnmanagedExport           =   0x0008,     // Managed method exported via thunk to unmanaged code.

    // Reserved flags for runtime use only.
    MethodDefFlags_ReservedMask              =   0xd000,
    MethodDefFlags_RTSpecialName             =   0x1000,     // Runtime should check name encoding.
    MethodDefFlags_HasSecurity               =   0x4000,     // Method has security associate with it.
    MethodDefFlags_RequireSecObject          =   0x8000      // Method calls another method containing security code.
}
END_ENUM(MethodDefFlags_t, uint16) // table0x06

BEGIN_ENUM(MethodDefImplFlags_t, uint16) // table0x06
{
    // code impl mask
    MethodDefImplFlags_CodeTypeMask      =   0x0003,   // Flags about code type.
    MethodDefImplFlags_IL                =   0x0000,   // Method impl is IL.
    MethodDefImplFlags_Native            =   0x0001,   // Method impl is native.
    MethodDefImplFlags_OPTIL             =   0x0002,   // Method impl is OPTIL
    MethodDefImplFlags_Runtime           =   0x0003,   // Method impl is provided by the runtime.
    // end code impl mask

    // managed mask
    MethodDefImplFlags_ManagedMask       =   0x0004,   // Flags specifying whether the code is managed or unmanaged.
    MethodDefImplFlags_Unmanaged         =   0x0004,   // Method impl is unmanaged, otherwise managed.
    MethodDefImplFlags_Managed           =   0x0000,   // Method impl is managed.
    // end managed mask

    // implementation info and interop
    MethodDefImplFlags_ForwardRef        =   0x0010,   // Indicates method is defined; used primarily in merge scenarios.
    MethodDefImplFlags_PreserveSig       =   0x0080,   // Indicates method sig is not to be mangled to do HRESULT conversion.

    MethodDefImplFlags_InternalCall      =   0x1000,   // Reserved for internal use.

    MethodDefImplFlags_Synchronized      =   0x0020,   // Method is single threaded through the body.
    MethodDefImplFlags_NoInlining        =   0x0008,   // Method may not be inlined.
    MethodDefImplFlags_MaxMethodImplVal  =   0xffff    // Range check value
}
END_ENUM(MethodDefImplFlags_t, uint16) // table0x06

struct Method_t : Member_t
{
    bool operator<(const Method_t&) const; // support for old compiler/library
    bool operator==(const Method_t&) const; // support for old compiler/library
};

BEGIN_ENUM(TypeFlags_t, uint32)
{
    //TODO bitfields (need to test little and big endian)
    //TODO or bitfield decoder
    // Use this mask to retrieve the type visibility information.
    TypeFlags_VisibilityMask        =   0x00000007,
    TypeFlags_NotPublic             =   0x00000000,     // Class is not public scope.
    TypeFlags_Public                =   0x00000001,     // Class is public scope.
    TypeFlags_NestedPublic          =   0x00000002,     // Class is nested with public visibility.
    TypeFlags_NestedPrivate         =   0x00000003,     // Class is nested with private visibility.
    TypeFlags_NestedFamily          =   0x00000004,     // Class is nested with family visibility.
    TypeFlags_NestedAssembly        =   0x00000005,     // Class is nested with assembly visibility.
    TypeFlags_NestedFamANDAssem     =   0x00000006,     // Class is nested with family and assembly visibility.
    TypeFlags_NestedFamORAssem      =   0x00000007,     // Class is nested with family or assembly visibility.

    // Use this mask to retrieve class layout information
    TypeFlags_LayoutMask            =   0x00000018,
    TypeFlags_AutoLayout            =   0x00000000,     // Class fields are auto-laid out
    TypeFlags_SequentialLayout      =   0x00000008,     // Class fields are laid out sequentially
    TypeFlags_ExplicitLayout        =   0x00000010,     // Layout is supplied explicitly
    // end layout mask

    // Use this mask to retrieve class semantics information.
    TypeFlags_ClassSemanticsMask    =   0x00000060,
    TypeFlags_Class                 =   0x00000000,     // Type is a class.
    TypeFlags_Interface             =   0x00000020,     // Type is an interface.
    // end semantics mask

    // Special semantics in addition to class semantics.
    TypeFlags_Abstract              =   0x00000080,     // Class is abstract
    TypeFlags_Sealed                =   0x00000100,     // Class is concrete and may not be extended
    TypeFlags_SpecialName           =   0x00000400,     // Class name is special. Name describes how.

    // Implementation attributes.
    TypeFlags_Import                =   0x00001000,     // Class / interface is imported
    TypeFlags_Serializable          =   0x00002000,     // The class is Serializable.

    // Use StringFormatMask to retrieve string information for native interop
    TypeFlags_StringFormatMask      =   0x00030000,
    TypeFlags_AnsiClass             =   0x00000000,     // LPTSTR is interpreted as ANSI in this class
    TypeFlags_UnicodeClass          =   0x00010000,     // LPTSTR is interpreted as UNICODE
    TypeFlags_AutoClass             =   0x00020000,     // LPTSTR is interpreted automatically
    TypeFlags_CustomFormatClass     =   0x00030000,     // A non-standard encoding specified by CustomFormatMask
    TypeFlags_CustomFormatMask      =   0x00C00000,     // Use this mask to retrieve non-standard encoding information for native interop. The meaning of the values of these 2 bits is unspecified.

    // end string format mask

    TypeFlags_BeforeFieldInit       =   0x00100000,     // Initialize the class any time before first static field access.
    TypeFlags_Forwarder             =   0x00200000,     // This ExportedType is a type forwarder.

    // Flags reserved for runtime use.
    TypeFlags_ReservedMask          =   0x00040800,
    TypeFlags_RTSpecialName         =   0x00000800,     // Runtime should check name encoding.
    TypeFlags_HasSecurity           =   0x00040000      // Class has security associate with it.
}
END_ENUM(TypeFlags_t, uint32)

struct Type_t // class, valuetype, delegate, inteface, not char, short, int, long, float
{
};

BEGIN_ENUM(EventFlags_t, uint16)
{
    EventFlags_SpecialName           =   0x0200,     // event is special. Name describes how.
    // Reserved flags for Runtime use only.
    EventFlags_RTSpecialName         =   0x0400      // Runtime(metadata internal APIs) should check name encoding.
}
END_ENUM(EventFlags_t, uint16)

struct Event_t : Member_t // table0x14
{
    bool operator<(const Event_t&) const; // support for old compiler/library
    bool operator==(const Event_t&) const; // support for old compiler/library
    EventFlags_t Flags;
    String_t Name;
    Type_t* EventType;
};

struct Property_t : Member_t
{
    bool operator<(const Property_t&) const; // support for old compiler/library
    bool operator==(const Property_t&) const; // support for old compiler/library
};

BEGIN_ENUM(FieldFlags_t, uint16)
{
//TODO bitfields (need to test little and big endian)
//TODO or bitfield decoder
// member access mask - Use this mask to retrieve accessibility information.
    FieldFlags_FieldAccessMask           =   0x0007,
    FieldFlags_PrivateScope              =   0x0000,     // Member not referenceable.
    FieldFlags_Private                   =   0x0001,     // Accessible only by the parent type.
    FieldFlags_FamANDAssem               =   0x0002,     // Accessible by sub-types only in this Assembly.
    FieldFlags_Assembly                  =   0x0003,     // Accessibly by anyone in the Assembly.
    FieldFlags_Family                    =   0x0004,     // Accessible only by type and sub-types.
    FieldFlags_FamORAssem                =   0x0005,     // Accessibly by sub-types anywhere, plus anyone in assembly.
    FieldFlags_Public                    =   0x0006,     // Accessibly by anyone who has visibility to this scope.
    // end member access mask

    // field contract attributes.
    FieldFlags_Static                    =   0x0010,     // Defined on type, else per instance.
    FieldFlags_InitOnly                  =   0x0020,     // Field may only be initialized, not written to after init.
    FieldFlags_Literal                   =   0x0040,     // Value is compile time constant.
    FieldFlags_NotSerialized             =   0x0080,     // Field does not have to be serialized when type is remoted.

    FieldFlags_SpecialName               =   0x0200,     // field is special. Name describes how.

    // interop attributes
    FieldFlags_PinvokeImpl               =   0x2000,     // Implementation is forwarded through pinvoke.

    // Reserved flags for runtime use only.
    FieldFlags_ReservedMask              =   0x9500,
    FieldFlags_RTSpecialName             =   0x0400,     // Runtime(metadata internal APIs) should check name encoding.
    FieldFlags_HasFieldMarshal           =   0x1000,     // Field has marshalling information.
    FieldFlags_HasDefault                =   0x8000,     // Field has default.
    FieldFlags_HasFieldRVA               =   0x0100      // Field has RVA.
}
END_ENUM(FieldFlags_t, uint16)

BEGIN_ENUM(DeclSecurityAction_t, uint16)
{
    // TODO values?
    DeclSecurityAction_Assert,
    DeclSecurityAction_Demand,
    DeclSecurityAction_Deny,
    DeclSecurityAction_InheritanceDemand,
    DeclSecurityAction_LinkDemand,
    DeclSecurityAction_NonCasDemand,
    DeclSecurityAction_NonCasLinkDemand,
    DeclSecurityAction_PrejitGrant,
    DeclSecurityAction_PermitOnlh,
    DeclSecurityAction_RequestMinimum,
    DeclSecurityAction_RequestOptional,
    DeclSecurityAction_RequestRefuse
}
END_ENUM(DeclSecurityAction_t, uint16) // TODO get the values


struct Interface_t
{
    Interface_t () { }
    Interface_t (const Interface_t&) { }
    bool operator<(const Interface_t&) const; // support for old compiler/library
    bool operator==(const Interface_t&) const; // support for old compiler/library

    std::vector<Method_t*> methods;
};

struct FieldOrParam_t
{
};

struct Signature_t
{
};

struct AssemblyRef_t;
struct File_t;

struct Implementation_t
{
    //union
        AssemblyRef_t* AssemblyRef;
        File_t* File;
};

struct Class_t
{
    Class_t* base;
    std::string name;
    vector<Interface_t> interfaces;
    vector<Method_t> methods;
    vector<Field_t> fields;
    vector<Event_t> events;
    vector<Property_t> properties;
};

class MethodBody_t
{
};

class MethodDeclaration_t
{
};

struct TypeOrMethodDef_t
{
    //union
    Type_t* Type;
    Method_t* Method;
};

BEGIN_ENUM(MethodSemanticsFlags_t, uint16)
{
    MethodSemanticsFlags_Setter = 1, // msSetter
    MethodSemanticsFlags_Getter = 2,
    MethodSemanticsFlags_Other = 4,
    MethodSemanticsFlags_AddOn = 8,
    MethodSemanticsFlags_RemoveOn = 0x10,
    MethodSemanticsFlags_Fire = 0x20
}
END_ENUM(MethodSemanticsFlags_t, uint16)

union MethodSemanticsAssociation_t // table0x18
{
    Event_t* Event;
    Property_t* Property;
};

// TODO enum
typedef uint16 PInvokeAttributes;

// TODO enum or bitfield
//const uint COMIMAGE_FLAGS_ILONLY = 1;
//const uint COMIMAGE_FLAGS_32BITREQUIRED = 2;
//const uint COMIMAGE_FLAGS_IL_LIBRARY = 4;
//const uint COMIMAGE_FLAGS_STRONGNAMESIGNED = 8;
//const uint COMIMAGE_FLAGS_NATIVE_ENTRYPOINT = 0x10;
//const uint COMIMAGE_FLAGS_TRACKDEBUGDATA = 0x10000;

// TODO enum or bitfield
// Bit clear: 16 bit; bit set: 32 bit
const uint8 HeapOffsetSize_String = 1;
const uint8 HeapOffsetSize_Guid = 2;
const uint8 HeapOffsetSize_Blob = 4;

struct Guid_t
{
    uint8 bytes [16];
};

// TODO enum
const int8 Module = 0;
const int8 TypeRef = 1;
const int8 TypeDef = 2;
//const int8 FieldPtr = 3; // nonstandard
const int8 Field = 4;
//const int8 MethodPtr = 5; // nonstandard
const int8 MethodDef = 6;
//const int8 ParamPtr = 7; // nonstandard
const int8 Param = 8;
const int8 InterfaceImpl = 9;
const int8 MemberRef = 10;
const int8 MethodRef = MemberRef;
//const int8 FieldRef = MemberRef;
//const int8 Constant = 11;
//const int8 CustomAttribute = 12;
//const int8 FieldMarshal = 13;
const int8 DeclSecurity = 14;
//const int8 ClassLayout = 15;
//const int8 FieldLayout = 16;
const int8 StandAloneSig = 17;
//const int8 EventMap = 18;
//const int8 EventPtr = 19; // nonstandard
const int8 Event = 20;
const int8 PropertyMap = 21;
//const int8 ProperyPtr = 22; // nonstandard
const int8 Property = 23;
const int8 MethodSemantics = 24; // 0x18
const int8 MethodImpl = 25; // 0x19
const int8 ModuleRef = 26;
const int8 TypeSpec = 27;
//const int8 ImplMap = 28;
//const int8 FieldRVA = 29;
//const int8 ENCLog = 30; // nonstandard
//const int8 ENCMap = 31; // nonstandard
const int8 Assembly = 32;
//const int8 AssemblyProcessor = 33;
//const int8 AssemblyOS = 34;
const int8 AssemblyRef = 35;
//const int8 AssemblyRefProcessor = 36;
//const int8 AssemblyRefOS = 37;
const int8 File = 38;
const int8 ExportedType = 39;
const int8 ManifestResource = 40; // 0x28
//const int8 NestedClass = 41;
const int8 GenericParam = 42; // 0x2A
const int8 MethodSpec = 0x2B;
const int8 GenericParamConstraint = 44; // 0x2C

// TODO enum/bitfield
const uint CorILMethod_TinyFormat = 2;
const uint CorILMethod_FatFormat = 3;
const uint CorILMethod_MoreSects = 8;
const uint CorILMethod_InitLocals = 0x10;

struct method_header_tiny_t
// no locals
// no exceptions
// no extra data sections (what does this mean?)
// maxstack=8
{
    uint8 Value;
    uint8 GetFlags () { return (uint8)(Value & 3); }
    uint8 GetSize () { return (uint8)(Value >> 2); }
};

struct method_header_fat_t
{
    uint16 FlagsAndHeaderSize;
    uint16 MaxStack;
    uint32 CodeSize;
    uint32 LocalVarSigTok;

    bool MoreSects () { return !!(GetFlags () & CorILMethod_MoreSects); }
    bool InitLocals () { return !!(GetFlags () & CorILMethod_InitLocals); }
    uint16 GetFlags () { return (uint16)(FlagsAndHeaderSize & 0xFFF); }
    uint16 GetHeaderSize () { return (uint16)(FlagsAndHeaderSize >> 12); } // should be 3
};

// CLR Metadata often has indices into one of a few tables.
// The indices are tagged, in a scheme specific to the possibilities
// of that index.
// As well, such indices have variable size depending on the maximum index.

struct CodedIndex_t
{
    //const char name [24];
    uint8 tag_size;
    uint8 count;
    uint8 map;
    //uint tag_mask; // TODO
    //uint table_index_mask; // TODO
};

#define COMMA ,

// Coded indices are into one of a few tables.
// A few bits indicate which table -- log base of the number of possibilities.
// The remaining bits either fill out a total of 16 bits
// or 32 bits, depending on the maximum index of the possible tables.
//
// To determine the size of a coded index therefore, check the number
// of rows of each possible referenced table, get the maximum, see
// if it fits 16 - n bits, where n = log base of the number of possible tables.
// If so, the coded index is 16 bits, else 32 bits.
#define CODED_INDICES                                                                               \
CODED_INDEX (TypeDefOrRef,      3, {TypeDef COMMA TypeRef COMMA TypeSpec})                          \
CODED_INDEX (HasConstant,       3, {Field COMMA Param COMMA Property})                              \
CODED_INDEX (HasFieldMarshal,   2, {Field COMMA Param})                                             \
CODED_INDEX (HasDeclSecurity,   3, {TypeDef COMMA MethodDef COMMA Assembly})                        \
CODED_INDEX (MemberRefParent,   5, {TypeDef COMMA TypeRef COMMA ModuleRef COMMA MethodDef COMMA TypeSpec})      \
CODED_INDEX (HasSemantics,      2, {Event COMMA Property})                                          \
CODED_INDEX (MethodDefOrRef,    3, {MethodDef COMMA MethodRef})                                     \
CODED_INDEX (MemberForwarded,   2, {Field COMMA MethodDef})                                         \
CODED_INDEX (Implementation,    3, {File COMMA AssemblyRef COMMA ExportedType})                     \
/* CodeIndex(CustomAttributeType) has a range of 5 values but only 2 are valid. */                  \
CODED_INDEX (CustomAttributeType, 5, {-1 COMMA -1 COMMA MethodDef COMMA MethodRef COMMA -1})        \
CODED_INDEX (ResolutionScope, 4, {Module COMMA ModuleRef COMMA AssemblyRef COMMA TypeRef})          \
CODED_INDEX (TypeOrMethodDef, 2, {TypeDef COMMA MethodDef})                                         \
CODED_INDEX (HasCustomAttribute, 22,                                                                \
      {MethodDef COMMA     Field COMMA        TypeRef COMMA      TypeDef COMMA          Param COMMA          /* HasCustomAttribute */ \
      InterfaceImpl COMMA MemberRef COMMA     Module COMMA       DeclSecurity COMMA     Property COMMA       /* HasCustomAttribute */ \
      Event COMMA         StandAloneSig COMMA ModuleRef COMMA    TypeSpec COMMA         Assembly COMMA       /* HasCustomAttribute */ \
      AssemblyRef COMMA   File COMMA          ExportedType COMMA ManifestResource COMMA GenericParam COMMA   /* HasCustomAttribute */ \
      GenericParamConstraint COMMA MethodSpec                                          }) /* HasCustomAttribute */

#define CODED_INDEX(a, n, values) CodedIndex_ ## a,
BEGIN_ENUM(CodedIndex, uint8)
{
CODED_INDICES
#undef CODED_INDEX
    CodedIndex_Count
}
END_ENUM(CodedIndex, uint8)


struct CodedIndexMap_t // TODO array and named
{
#define CODED_INDEX(a, n, values) int8 a [n];
CODED_INDICES
#undef CODED_INDEX
};

const CodedIndexMap_t CodedIndexMap = {
#define CODED_INDEX(a, n, values) values,
CODED_INDICES
#undef CODED_INDEX
};

#define LOG_BASE2_X(a, x) (a) > (1u << x) ? (x + 1) :
#define LOG_BASE2(a)                                                                                \
    (LOG_BASE2_X(a, 31) LOG_BASE2_X(a, 30)                                                          \
     LOG_BASE2_X(a, 29) LOG_BASE2_X(a, 28) LOG_BASE2_X(a, 27) LOG_BASE2_X(a, 26) LOG_BASE2_X(a, 25) \
     LOG_BASE2_X(a, 24) LOG_BASE2_X(a, 23) LOG_BASE2_X(a, 22) LOG_BASE2_X(a, 21) LOG_BASE2_X(a, 20) \
     LOG_BASE2_X(a, 19) LOG_BASE2_X(a, 18) LOG_BASE2_X(a, 17) LOG_BASE2_X(a, 16) LOG_BASE2_X(a, 15) \
     LOG_BASE2_X(a, 14) LOG_BASE2_X(a, 13) LOG_BASE2_X(a, 12) LOG_BASE2_X(a, 11) LOG_BASE2_X(a, 10) \
     LOG_BASE2_X(a,  9) LOG_BASE2_X(a,  8) LOG_BASE2_X(a,  7) LOG_BASE2_X(a,  6) LOG_BASE2_X(a,  5) \
     LOG_BASE2_X(a,  4) LOG_BASE2_X(a,  3) LOG_BASE2_X(a,  2) LOG_BASE2_X(a,  1) LOG_BASE2_X(a,  0) 0)

//#define CountOf(x) (std::size(x)) // C++17
#define CountOf(x) (sizeof (x) / sizeof ((x)[0])) // TODO
#define CountOfField(x, y) (CountOf(x().y))
#define CODED_INDEX(a, n, values) CodedIndex_t a;

union CodedIndices_t
{
    CodedIndex_t array [CodedIndex_Count];
    struct {
CODED_INDICES
#undef CODED_INDEX
    } name;
};

const CodedIndices_t CodedIndices = {{
#define CODED_INDEX(x, n, values) {LOG_BASE2 (CountOfField (CodedIndexMap_t, x)), CountOfField(CodedIndexMap_t, x), offsetof(CodedIndexMap_t, x) },
CODED_INDICES
#undef CODED_INDEX
}};

#undef CodedIndex
#define CodedIndex(x) CodedIndex_ ## x

struct HeapIndex_t
{
    const char *heap_name; // string, guid, etc.
    uint8 heap_index; // dynamic?
};

struct metadata_guid_t
{
    uint32 index;
    const char* pointer;
};

struct MetadataString_t
{
    uint32 offset;
    uint32 size;
    const char* pointer;
};

struct metadata_unicode_string_t
{
    uint32 offset;
    uint32 size;
    const char16* pointer;
};

struct metadata_blob_t
{
    uint32 offset;
    uint32 size;
    const void* pointer;
};

struct MetadataToken_t
{
    uint8 table;
    uint32 index; // uint24
};

struct MetadataTokenList_t
{
    uint8 table;
    uint32 count;
    uint32 index;
};

struct MetadataTablesHeader // tilde stream
{
    uint32 reserved;    // 0
    uint8 MajorVersion;
    uint8 MinorVersion;
    union {
        uint8 HeapOffsetSizes;
        uint8 HeapSizes;
    };
    uint8 reserved2;    // 1
    uint64 Valid;       // metadata_typedef etc.
    uint64 Sorted;      // metadata_typedef etc.
    // uint32 NumberOfRows [];
};

struct MetadataRoot
{
    enum { SIGNATURE = 0x424A5342 };
    /* 0 */ uint32 Signature;
    /* 4 */ uint16 MajorVersion; // 1, ignore
    /* 6 */ uint16 MinorVersion; // 1, ignore
    /* 8 */ uint32 Reserved;     // 0
    /* 12 */ uint32 VersionLength; // VersionLength null, round up to 4
    /* 16 */ char Version [1];
    // uint16 Flags; // 0
    // uint16 NumberOfStreams;
    // MetadataStreamHeader stream_headers [NumberOfStreams];
};

struct MetadataStreamHeader // see mono verify_metadata_header
{
    uint32 offset;
    uint32 Size; // multiple of 4
    char   Name [32]; // multiple of 4, null terminated, max 32
};

BEGIN_ENUM(ParamFlags_t, uint16)
{
    ParamFlags_In                        =   0x0001,     // Param is [In]
    ParamFlags_Out                       =   0x0002,     // Param is [out]
    ParamFlags_Optional                  =   0x0010,     // Param is optional

    // Reserved flags for Runtime use only.
    ParamFlags_ReservedMask              =   0xf000,
    ParamFlags_HasDefault                =   0x1000,     // Param has default value.
    ParamFlags_HasFieldMarshal           =   0x2000,     // Param has FieldMarshal.

    ParamFlags_Unused                    =   0xcfe0
}
END_ENUM(ParamFlags_t, uint16)

BEGIN_ENUM(AssemblyFlags, uint32)
{
    AssemblyFlags_PublicKey             =   0x0001,     // The assembly ref holds the full (unhashed) public key.

    AssemblyFlags_PA_None               =   0x0000,     // Processor Architecture unspecified
    AssemblyFlags_PA_MSIL               =   0x0010,     // Processor Architecture: neutral (PE32)
    AssemblyFlags_PA_x86                =   0x0020,     // Processor Architecture: x86 (PE32)
    AssemblyFlags_PA_IA64               =   0x0030,     // Processor Architecture: Itanium (PE32+)
    AssemblyFlags_PA_AMD64              =   0x0040,     // Processor Architecture: AMD X64 (PE32+)
    AssemblyFlags_PA_Specified          =   0x0080,     // Propagate PA flags to AssemblyRef record
    AssemblyFlags_PA_Mask               =   0x0070,     // Bits describing the processor architecture
    AssemblyFlags_PA_FullMask           =   0x00F0,     // Bits describing the PA incl. Specified
    AssemblyFlags_PA_Shift              =   0x0004,     // NOT A FLAG, shift count in PA flags <--> index conversion

    AssemblyFlags_EnableJITcompileTracking  =   0x8000, // From "DebuggableAttribute".
    AssemblyFlags_DisableJITcompileOptimizer=   0x4000, // From "DebuggableAttribute".

    AssemblyFlags_Retargetable          =   0x0100,     // The assembly can be retargeted (at runtime) to an
                                                        // assembly from a different publisher.
}
END_ENUM(AssemblyFlags, uint32)

BEGIN_ENUM(FileFlags_t, uint32)
{
    FileFlags_ContainsMetaData      =   0x0000,     // This is not a resource file
    FileFlags_ContainsNoMetaData    =   0x0001,     // This is a resource file or other non-metadata-containing file
}
END_ENUM(FileFlags_t, uint32)

BEGIN_ENUM(ManifestResourceFlags_t, uint32)
{
    ManifestResourceFlags_VisibilityMask        =   0x0007,
    ManifestResourceFlags_Public                =   0x0001,     // The Resource is exported from the Assembly.
    ManifestResourceFlags_Private               =   0x0002,     // The Resource is private to the Assembly.
}
END_ENUM(ManifestResourceFlags_t, uint32)

BEGIN_ENUM(GenericParamFlags_t, uint16)
{
    // Variance of type parameters, only applicable to generic parameters
    // for generic interfaces and delegates
    GenericParamFlags_VarianceMask          =   0x0003,
    GenericParamFlags_NonVariant            =   0x0000,
    GenericParamFlags_Covariant             =   0x0001,
    GenericParamFlags_Contravariant         =   0x0002,

    // Special constraints, applicable to any type parameters
    GenericParamFlags_SpecialConstraintMask =  0x001C,
    GenericParamFlags_NoSpecialConstraint   =   0x0000,
    GenericParamFlags_ReferenceTypeConstraint = 0x0004,      // type argument must be a reference type
    GenericParamFlags_NotNullableValueTypeConstraint   =   0x0008, // type argument must be a value type but not Nullable
    GenericParamFlags_DefaultConstructorConstraint = 0x0010, // type argument must have a public default constructor
}
END_ENUM(GenericParamFlags_t, uint16)

#if 0 // TODO This is copy/pasted from the web and should be gradually
      // worked into types/defines/enums, or deleted because it already was. And cross check with ECMA .pdf.

12 - CustomAttribute Table

//I think the best description is given by the SDK: "The CustomAttribute table stores data that can be used to instantiate a
// Custom Attribute (more precisely, an object of the specified Custom Attribute class) at runtime. The column called Type
//is slightly misleading - it actually indexes a constructor method - the owner of that constructor method is the Type of the Custom Attribute."

// Columns:

// - Parent (index into any metadata table, except the CustomAttribute table itself; more precisely, a HasCustomAttribute coded index)
// - Type (index into the MethodDef or MethodRef table; more precisely, a CustomAttributeType coded index)
// - Value (index into Blob heap)

13 - FieldMarshal Table

// Each row tells the way a Param or Field should be threated when called from/to unmanaged code.

// Columns:

// - Parent (index into Field or Param table; more precisely, a HasFieldMarshal coded index)
// - NativeType (index into the Blob heap)

14 - DeclSecurity Table

Security attributes attached to a class, method or assembly.

Columns:

- Action (2-byte value)
- Parent (index into the TypeDef, MethodDef or Assembly table; more precisely, a HasDeclSecurity coded index)
- PermissionSet (index into Blob heap)

15 - ClassLayout Table

//Remember "#pragma pack(n)" for VC++? Well, this is kind of the same thing for .NET. It's
// useful when handing something from managed to unmanaged code.

Columns:

- PackingSize (a 2-byte constant)
- ClassSize (a 4-byte constant)
- Parent (index into TypeDef table)

16 - FieldLayout Table

Related with the ClassLayout.

Columns:

- Offset (a 4-byte constant)
- Field (index into the Field table)

18 - EventMap Table

List of events for a specific class.

Columns:

- Parent (index into the TypeDef table)
- EventList (index into Event table). It marks the first of a contiguous run of Events owned by this Type. The run continues to the smaller of:
        o the last row of the Event table
        o the next run of Events, found by inspecting the EventList of the next row in the EventMap table

20 - Event Table

Each row represents an event.

Columns:

- EventFlags (a 2-byte bitmask of type EventAttribute)
- Name (index into String heap)
- EventType (index into TypeDef, TypeRef or TypeSpec tables; more precisely, a TypeDefOrRef coded index) [this corresponds to the Type of the Event; it is not the Type that owns this event]

Available flags are:

typedef enum CorEventAttr
{
    evSpecialName           =   0x0200,     // event is special. Name describes how.

    // Reserved flags for Runtime use only.
    evReservedMask          =   0x0400,
    evRTSpecialName         =   0x0400      // Runtime(metadata internal APIs) should check name encoding.
} CorEventAttr;

23 - Property Table

Each row represents a property.

Columns:

- Flags (a 2-byte bitmask of type PropertyAttributes)
- Name (index into String heap)
- Type (index into Blob heap) [the name of this column is misleading. It does not index a TypeDef or TypeRef table – instead it indexes the signature in the Blob heap of the Property)

Available flags are:

typedef enum CorPropertyAttr
{
    prSpecialName           =   0x0200,     // property is special. Name describes how.

    // Reserved flags for Runtime use only.
    prReservedMask          =   0xf400,
    prRTSpecialName         =   0x0400,     // Runtime(metadata internal APIs) should check name encoding.
    prHasDefault            =   0x1000,     // Property has default

    prUnused                =   0xe9ff
} CorPropertyAttr;


25 - MethodImpl Table

// I quote: "MethodImpls let a compiler override the default inheritance rules provided by the CLI.
// Their original use was to allow a class “C”, that inherited method “Foo” from interfaces I and J,
// to provide implementations for both methods (rather than have only one slot for “Foo” in its vtable).
// But MethodImpls can be used for other reasons too, limited only by the compiler writer’s ingenuity
// within the constraints defined in the Validation rules below.".

Columns:

- Class (index into TypeDef table)
- MethodBody (index into MethodDef or MemberRef table; more precisely, a MethodDefOrRef coded index)
- MethodDeclaration (index into MethodDef or MemberRef table; more precisely, a MethodDefOrRef coded index)

26 - ModuleRef Table

Each row represents a reference to an external module.

Columns:

- Name (index into String heap)

27 - TypeSpec Table

Each row represents a specification for a TypeDef or TypeRef. The only column indexes a token in the #Blob stream.

Columns:

- Signature (index into the Blob heap)

28 - ImplMap Table

// I quote: "The ImplMap table holds information about unmanaged methods that can be reached from managed code,
// using PInvoke dispatch. Each row of the ImplMap table associates a row in the MethodDef table (MemberForwarded)
//  with the name of a routine (ImportName) in some unmanaged DLL (ImportScope).". This means all the unmanaged functions used by the assembly are listed here.

// Columns:

// - MappingFlags (a 2-byte bitmask of type PInvokeAttributes)
// - MemberForwarded (index into the Field or MethodDef table; more precisely, a MemberForwarded coded index.
//  However, it only ever indexes the MethodDef table, since Field export is not supported)
// - ImportName (index into the String heap)
// - ImportScope (index into the ModuleRef table)

Available flags are:

typedef enum  CorPinvokeMap
{
    pmNoMangle          = 0x0001,   // Pinvoke is to use the member name as specified.

    // Use this mask to retrieve the CharSet information.
    pmCharSetMask       = 0x0006,
    pmCharSetNotSpec    = 0x0000,
    pmCharSetAnsi       = 0x0002,
    pmCharSetUnicode    = 0x0004,
    pmCharSetAuto       = 0x0006,


    pmBestFitUseAssem   = 0x0000,
    pmBestFitEnabled    = 0x0010,
    pmBestFitDisabled   = 0x0020,
    pmBestFitMask       = 0x0030,

    pmThrowOnUnmappableCharUseAssem   = 0x0000,
    pmThrowOnUnmappableCharEnabled    = 0x1000,
    pmThrowOnUnmappableCharDisabled   = 0x2000,
    pmThrowOnUnmappableCharMask       = 0x3000,

    pmSupportsLastError = 0x0040,   // Information about target function. Not relevant for fields.

    // None of the calling convention flags is relevant for fields.
    pmCallConvMask      = 0x0700,
    pmCallConvWinapi    = 0x0100,   // Pinvoke will use native callconv appropriate to target windows platform.
    pmCallConvCdecl     = 0x0200,
    pmCallConvStdcall   = 0x0300,
    pmCallConvThiscall  = 0x0400,   // In M9, pinvoke will raise exception.
    pmCallConvFastcall  = 0x0500,

    pmMaxValue          = 0xFFFF
} CorPinvokeMap;

29 - FieldRVA Table

Each row is an extension for a Field table. The RVA in this table gives the location of the inital value for a Field.

Columns:

- RVA (a 4-byte constant)
- Field (index into Field table)

32 - Assembly Table

//It's a one-row table. It stores information about the current assembly.

Columns:

- HashAlgId (a 4-byte constant of type AssemblyHashAlgorithm)
- MajorVersion, MinorVersion, BuildNumber, RevisionNumber (2-byte constants)
- Flags (a 4-byte bitmask of type AssemblyFlags)
- PublicKey (index into Blob heap)
- Name (index into String heap)
- Culture (index into String heap)

Available flags are:

The PublicKey is != 0, only if the StrongName Signature is present and the afPublicKey flag is set.

35 - AssemblyRef Table

Each row references an external assembly.

Columns:

- MajorVersion, MinorVersion, BuildNumber, RevisionNumber (2-byte constants)
- Flags (a 4-byte bitmask of type AssemblyFlags)
- PublicKeyOrToken (index into Blob heap – the public key or token that identifies the author of this Assembly)
- Name (index into String heap)
- Culture (index into String heap)
- HashValue (index into Blob heap)

The flags are the same ones of the Assembly table.

36 - AssemblyRefProcessor Table

//This table is ignored by the CLI and shouldn't be present in an assembly.

Columns:

- Processor (4-byte constant)
- AssemblyRef (index into the AssemblyRef table)

37 - AssemblyRefOS Table

//This table is ignored by the CLI and shouldn't be present in an assembly.

Columns:

- OSPlatformId (4-byte constant)
- OSMajorVersion (4-byte constant)
- OSMinorVersion (4-byte constant)
- AssemblyRef (index into the AssemblyRef table)

38 - File Table

Each row references an external file.

Columns:

- Flags (a 4-byte bitmask of type FileAttributes)
- Name (index into String heap)
- HashValue (index into Blob heap)

Available flags are:


39 - ExportedType Table

//I quote: "The ExportedType table holds a row for each type, defined within other modules of this Assembly,
//that is exported out of this Assembly. In essence, it stores TypeDef row numbers of all types that are marked
//public in other modules that this Assembly comprises.". Be careful, this doesn't mean that when an assembly
// uses a class contained in my assembly I export that type. In fact, I haven't seen yet this table in an assembly.

Columns:

- Flags (a 4-byte bitmask of type TypeAttributes)
- TypeDefId (4-byte index into a TypeDef table of another module in this Assembly). This field is used as a
// hint only. If the entry in the target TypeDef table matches the TypeName and TypeNamespace entries in
//this table, resolution has succeeded. But if there is a mismatch, the CLI shall fall back to a search of the target TypeDef table
- TypeName (index into the String heap)
- TypeNamespace (index into the String heap)
- Implementation. This can be an index (more precisely, an Implementation coded index) into one of 2 tables, as follows:
        o File table, where that entry says which module in the current assembly holds the TypeDef
        o ExportedType table, where that entry is the enclosing Type of the current nested Type

The flags are the same ones of the TypeDef.

40 - ManifestResource Table

Each row references an internal or external resource.

Columns:

- Offset (a 4-byte constant)
- Flags (a 4-byte bitmask of type ManifestResourceAttributes)
- Name (index into the String heap)
- Implementation (index into File table, or AssemblyRef table, or null; more precisely, an Implementation coded index)

Available flags are:


//If the Implementation index is 0, then the referenced resource is internal. We obtain the File Offset of
// the resource by adding the converted Resources RVA (the one in the CLI Header) to the offset present in
//this table. I wrote an article you can either find on NTCore or codeproject about Manifest Resources,
// anyway I quote some parts from the other article to give at least a brief explanation, since this section
//is absolutely undocumented. There are different kinds of resources referenced by this table, and not all
//of them can be threated in the same way. Reading a bitmap, for example, is very simple: every Manifest
//Resource begins with a dword that tells us the size of the actual embedded resource... And that's it...
//After that, we have our bitmap. Ok, but what about those ".resources" files? For every dialog in a .NET
// Assembly there is one, this means every resource of a dialog is contained in the dialog's own ".resources" file.

//A very brief description of ".resources" files format: "The first dword is a signature which has to be
//0xBEEFCACE, otherwise the resources file has to be considered as invalid. Second dword contains the
//number of readers for this resources file, don't worry, it's something we don't have to talk about...
//Framework stuff. Third dword is the size of reader types This number is only good for us to skip
// the string (or strings) that follows, which is something like: "System.Resources.ResourceReader,
//mscorlibsSystem.Resources.RuntimeResourceSet, mscorlib, Version=1.0.5000.0, Culture=neutral, PublicKeyToken=b77a5c561934e089".
//It tells the framework the reader to use for this resources file.

// Ok, now we got to the interesting part. The next dword tells us the version of the resources file
// (existing versions are 1 and 2). After the version, another dword gives the number of actual resources in the
// file. Another dword follows and gives the number of resource types.

//To gather the additional information we need, we have to skip the resource types. For each type
//there's a 7bit encoded integer who gives the size of the string that follows. To decode these kind of
// integers you have to read every byte until you find one which hasn't the highest bit set and make some
// additional operations to obtain the final value... For the moment let's just stick to the format. After
// having skipped the types we have to align our position to an 8 byte base. Then we have a
// dword * NumberOfResources and each dword contains the hash of a resource. Then we have the same amount of
// dwords, this time with the offsets of the resource names. Another important dword follows: the Data Section
// Offset. We need this offset to retrieve resources offsets. After this dword we have the resources names.
// Well, actually it's not just the names (I just call it this way), every name  (7bit encoded integer + unicode
// string) is followed by a dword, an offset which you can add to the DataSection offset to retrieve the resource offset.
// The first thing we find, given a resource offset, is a 7bit encoded integer, which is the type index for the current resource.".

// If you are interested in this subject, check out that other article I wrote, since there you
// can find code that maybe helps you understand better.

42 - GenericParam Table

I quote: The GenericParam table stores the generic parameters used in generic type definitions
and generic methoddefinitions. These generic parameters can be constrained (i.e., generic arguments
shall extend some class and/or implement certain interfaces) or unconstrained..

Columns:

- Number (the 2-byte index of the generic parameter, numbered left-to-right, from zero)
- Flags (a 2-byte bitmask of type GenericParamAttributes)
- Owner (an index into the TypeDef or MethodDef table, specifying the Type or Method to which this generic parameter applies; more precisely, a TypeOrMethodDef coded index)
- Name (a non-null index into the String heap, giving the name for the generic parameter. This is purely descriptive and is used only by source language compilers and by Reflection)

Available flags are:

typedef enum CorGenericParamAttr
{
    // Variance of type parameters, only applicable to generic parameters
    // for generic interfaces and delegates
    gpVarianceMask          =   0x0003,
    gpNonVariant            =   0x0000,
    gpCovariant             =   0x0001,
    gpContravariant         =   0x0002,

    // Special constraints, applicable to any type parameters
    gpSpecialConstraintMask =  0x001C,
    gpNoSpecialConstraint   =   0x0000,
    gpReferenceTypeConstraint = 0x0004,      // type argument must be a reference type
    gpNotNullableValueTypeConstraint   =   0x0008, // type argument must be a value type but not Nullable
    gpDefaultConstructorConstraint = 0x0010, // type argument must have a public default constructor
} CorGenericParamAttr;

44 - GenericParamConstraint Table

// I quote: "The GenericParamConstraint table records the constraints for each generic parameter.
// Each generic parameter can be constrained to derive from zero or one class. Each generic parameter
// can be constrained to implement zero or more interfaces. Conceptually, each row in the GenericParamConstraint
//  table is ‘owned’ by a row in the GenericParam table. All rows in the GenericParamConstraint table for a
// given Owner shall refer to distinct constraints.".

The columns needed are, of course, only two

Columns:

// - Owner (an index into the GenericParam table, specifying to which generic parameter this row refers)
// - Constraint (an index into the TypeDef, TypeRef, or TypeSpec tables, specifying from which class this
// generic parameter is constrained to derive; or which interface this generic parameter is constrained
// to implement; more precisely, a TypeDefOrRef coded index)

// Ok that's all about MetaData tables. The last thing I have to explain, as I promised, is the Method format.

// Methods
// Every method contained in an assembly is referenced in the MethodDef table, the RVA tells us where
// the method is. The method body is made of three or at least two parts:

// - A header, which can be a Fat or a Tiny one.

// - The code. The code size is specified in the header.

// - Extra Sections. These sections are not always present, the header tells us if they are. Those sections
//  can store different kinds of data, but for now they are only used to store Exception Sections.
// Those sections sepcify try/catch handlers in the code.

// The first byte of the method tells us the type of header used. If the method uses a tiny header the CorILMethod_TinyFormat (0x02)
//  flag will be set otherwise the CorILMethod_FatFormat (0x03) flag. If the tiny header is used, the 2 low bits are
// reserved for flags (header type) and the rest specify the code size. Of course a tiny header can only be used if
// the code size is less than 64 bytes. In addition it cannot be used if maxstack > 8 or local variables or exceptions
//  (extra sections) are present. In all these other cases the fat header is used:

Offset Size Field Description

0 12 (bits) Flags Flags (CorILMethod_FatFormat shall be set in bits 0:1).
12 (bits) 4 (bits) Size Size of this header expressed as the count of 4-byte integers occupied (currently 3).

2 2 MaxStack Maximum number of items on the operand stack.
4 4 CodeSize Size in bytes of the actual method body
8 4 LocalVarSigTok Meta Data token for a signature describing the layout of the local variables for the method.  0 means there are no local variables present. This field references a stand-alone signature in the MetaData tables, which references an entry in the #Blob stream.

The available flags are:

Flag Value Description
CorILMethod_FatFormat 0x3 Method header is fat.
CorILMethod_TinyFormat 0x2 Method header is tiny.
CorILMethod_MoreSects 0x8 More sections follow after this header.
CorILMethod_InitLocals 0x10 Call default constructor on all local variables.
This means that when the CorILMethod_MoreSects is set, extra sections follow the method. To reach the first extra section we have to add the size of the header to the code size and to the file offset of the method, then aligne to the next 4-byte boundary.

Extra sections can have a Fat (1 byte flags, 3 bytes size) or a Small header (1 byte flags, 1 byte size); the size includes the header size. The type of header and the type of section is specified in the first byte, of course:

Flag Value Description

CorILMethod_Sect_EHTable 0x1
Exception handling data. CorILMethod_Sect_OptILTable 0x2 Reserved, shall be 0.
CorILMethod_Sect_FatFormat 0x40 Data format is of the fat variety, meaning there is a 3-byte length.  If not set, the header is small with a  1-byte length
CorILMethod_Sect_MoreSects 0x80 Another data section occurs after this current section

// No other types than the exception handling sections are declared (this doesn't mean
// you shouldn't check the CorILMethod_Sect_EHTable flag). So if the section is small it will be:

Offset Size Field Description
0 1 Kind Flags as described above.
1 1 DataSize Size of the data for the block, including the header, say n*12+4.
2 2 Reserved Padding, always 0.
4 n Clauses n small exception clauses.

Otherwise:

Offset Size Field Description

0 1 Kind Which type of exception block is being used
1 3 DataSize Size of the data for the block, including the header, say n*24+4.
4 n Clauses n fat exception clauses.

The number of the clauses is given byte the DataSize. I mean you have to subtract the size of the header and then divide by the size of a Fat/Small exception clause (this, of course, depends on the kind of header). The small one:

Offset Size Field Description

0 2 Flags Flags, see below.
2 2 TryOffset Offset in bytes of try block from start of the header.
4 1 TryLength Length in bytes of the try block
5 2 HandlerOffset Location of the handler for this try block
7 1 HandlerLength Size of the handler code in bytes
8 4 ClassToken Meta data token for a type-based exception handler
8 4 FilterOffset Offset in method body for filter-based exception handler
And the fat one:

Offset Size Field Description
0 4 Flags Flags, see below.
4 4 TryOffset Offset in bytes of  try block from start of the header.
8 4 TryLength Length in bytes of the try block
12 4 HandlerOffset Location of the handler for this try block
16 4 HandlerLength Size of the handler code in bytes
20 4 ClassToken Meta data token for a type-based exception handler
20 4 FilterOffset Offset in method body for filter-based exception handler

Available flags are:

Flag Value Description

const uint COR_ILEXCEPTION_CLAUSE_EXCEPTION = 0; // A typed exception clause
const uint COR_ILEXCEPTION_CLAUSE_FILTER = 1; // An exception filter and handler clause
const uint COR_ILEXCEPTION_CLAUSE_FINALLY = 2; // A finally clause
const uint COR_ILEXCEPTION_CLAUSE_FAULT = 4; // Fault clause (finally that is called on exception only)

//The #Blob Stream
// This stream contains different things as you might have already noticed going through the MetaData tables,
// but the only thing in this stream which is a bit difficult to understand are signatures. Every signature
// referenced by the MetaData tables is contained in this stream. What does a signature stand for? For instance,
// it could tell us the declaration of a method (be it a defined method or a referenced one), this
// meaning the parameters and the return type of that method. The various kind of signatures are:
// MethodDefSig, MethodRefSig, FieldSig, PropertySig, LocalVarSig, TypeSpec, MethodSpec.

typedef enum CorElementType
{
    ELEMENT_TYPE_END            = 0x0,
    ELEMENT_TYPE_VOID           = 0x1,
    ELEMENT_TYPE_BOOLEAN        = 0x2,
    ELEMENT_TYPE_CHAR           = 0x3,
    ELEMENT_TYPE_I1             = 0x4,
    ELEMENT_TYPE_U1             = 0x5,
    ELEMENT_TYPE_I2             = 0x6,
    ELEMENT_TYPE_U2             = 0x7,
    ELEMENT_TYPE_I4             = 0x8,
    ELEMENT_TYPE_U4             = 0x9,
    ELEMENT_TYPE_I8             = 0xa,
    ELEMENT_TYPE_U8             = 0xb,
    ELEMENT_TYPE_R4             = 0xc,
    ELEMENT_TYPE_R8             = 0xd,
    ELEMENT_TYPE_STRING         = 0xe,

    // every type above PTR will be simple type

    ELEMENT_TYPE_PTR            = 0xf,      // PTR
    ELEMENT_TYPE_BYREF          = 0x10,     // BYREF

    // Please use ELEMENT_TYPE_VALUETYPE. ELEMENT_TYPE_VALUECLASS is deprecated.
    ELEMENT_TYPE_VALUETYPE      = 0x11,     // VALUETYPE

    ELEMENT_TYPE_CLASS          = 0x12,     // CLASS
    ELEMENT_TYPE_VAR            = 0x13,     // a class type variable VAR
    ELEMENT_TYPE_ARRAY          = 0x14,     // MDARRAY     ...   ...

    ELEMENT_TYPE_GENERICINST    = 0x15,     // GENERICINST    ...
    ELEMENT_TYPE_TYPEDBYREF     = 0x16,     // TYPEDREF  (it takes no args) a typed referece to some other type

    ELEMENT_TYPE_I              = 0x18,     // native integer size
    ELEMENT_TYPE_U              = 0x19,     // native unsigned integer size
    ELEMENT_TYPE_FNPTR          = 0x1B,     // FNPTR

    ELEMENT_TYPE_OBJECT         = 0x1C,     // Shortcut for System.Object
    ELEMENT_TYPE_SZARRAY        = 0x1D,     // Shortcut for single dimension zero lower bound array
                                            // SZARRAY
    ELEMENT_TYPE_MVAR           = 0x1e,     // a method type variable MVAR

    // This is only for binding
    ELEMENT_TYPE_CMOD_REQD      = 0x1F,     // required C modifier : E_T_CMOD_REQD
    ELEMENT_TYPE_CMOD_OPT       = 0x20,     // optional C modifier : E_T_CMOD_OPT

    // This is for signatures generated internally (which will not be persisted in any way).
    ELEMENT_TYPE_INTERNAL       = 0x21,     // INTERNAL

    // Note that this is the max of base type excluding modifiers
    ELEMENT_TYPE_MAX            = 0x22,     // first invalid element type
    ELEMENT_TYPE_MODIFIER       = 0x40,
    ELEMENT_TYPE_SENTINEL       = 0x01 | ELEMENT_TYPE_MODIFIER, // sentinel for varargs
    ELEMENT_TYPE_PINNED         = 0x05 | ELEMENT_TYPE_MODIFIER,
    ELEMENT_TYPE_R4_HFA         = 0x06 | ELEMENT_TYPE_MODIFIER, // used only internally for R4 HFA types
    ELEMENT_TYPE_R8_HFA         = 0x07 | ELEMENT_TYPE_MODIFIER, // used only internally for R8 HFA types

} CorElementType;

#endif

// COM+ 2.0 header structure.
struct image_clr_header_t // data_directory [15]
{
    uint32 cb; // count of bytes
    uint16 MajorRuntimeVersion;
    uint16 MinorRuntimeVersion;
    image_data_directory_t MetaData;
    uint32 Flags;
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set, EntryPointToken represents a managed entrypoint.
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set, EntryPointRVA represents an RVA to a native entrypoint.
    union {
        uint32 EntryPointToken;
        uint32 EntryPointRVA;
    };
    image_data_directory_t Resources;
    image_data_directory_t StrongNameSignature;
    image_data_directory_t CodeManagerTable;
    image_data_directory_t VTableFixups;
    image_data_directory_t ExportAddressTableJumps;
    image_data_directory_t ManagedNativeHeader;
};

struct MetadataType_t;

struct Image;

struct MetadataTypeFunctions_t
{
    // Virtual functions, but allowing for static construction.
    void (*decode)(const MetadataType_t*, void*);
    uint (*size)(const MetadataType_t*, Image*);
    //void (*to_string)(const MetadataType_t*, void*);
    void (*print)(const MetadataType_t*, Image*, uint table, uint row, uint column, void* data, uint size);
};

int64
sign_extend(uint64 value, uint bits)
{
    // Extract lower bits from value and signextend.
    // From detour_sign_extend.
    const uint left = 64 - bits;
    const int64 m1 = -1;
    const int64 wide = (int64)(value << left);
    const int64 sign = (wide < 0) ? (m1 << left) : 0;
    return value | sign;
}

struct int_split_sign_magnitude_t
{
    int_split_sign_magnitude_t(int64 a)
      : neg(a < 0), 
        u((a < 0) ? (1 + (uint64)-(a + 1)) // Avoid negating most negative number.
                  : (uint64)a) { }
    uint neg;
    uint64 u;
};

uint
uint_get_precision(uint64 a)
{
    // How many bits needed to represent.
    uint len = 1;
    while ((len <= 64) && (a >>= 1)) ++len;
    return len;
}

uint
int_get_precision(int64 a)
{
    // How many bits needed to represent.
    // i.e. so leading bit is extendible sign bit, or 64
    return std::min(64u, 1 + uint_get_precision (int_split_sign_magnitude_t(a).u));
}

uint
uint_to_dec_getlen(uint64 b)
{
    uint len = 0;
    do ++len;
    while (b /= 10);
    return len;
}

uint
uint_to_dec(int64 a, char* buf)
{
    const uint len = uint_to_dec_getlen(a);
    for (uint i = 0; i < len; ++i, a /= 10)
        buf[i] = "0123456789"[a % 10];
    return len;
}

uint
int_to_dec(int64 a, char* buf)
{
    const int_split_sign_magnitude_t split(a);
    if (split.neg)
        *buf++ = '-';
    return split.neg + uint_to_dec(split.u, buf);
}

uint
int_to_dec_getlen(int64 a)
{
    const int_split_sign_magnitude_t split(a);
    return split.neg + uint_to_dec_getlen(split.u);
}

uint
int_to_hex_getlen(int64 a)
{
    // If negative and first digit is <8, add one to induce leading 8-F
    // so that sign extension of most significant bit will work.
    // This might be a bad idea. TODO.
    uint64 b = a;
    uint len = 0;
    uint64 most_significant;
    do ++len;
    while ((most_significant = b), b >>= 4);
    return len + (a < 0 && most_significant < 8);
}

void
int_to_hexlen(int64 a, uint len, char *buf)
{
    buf += len;
    for (uint i = 0; i < len; ++i, a >>= 4)
        *--buf = "0123456789ABCDEF"[a & 0xF];
}

uint
int_to_hex(int64 a, char *buf)
{
    const uint len = int_to_hex_getlen (a);
    int_to_hexlen(a, len, buf);
    return len;
}

uint
int_to_hex8(int64 a, char *buf)
{
    int_to_hexlen(a, 8, buf);
    return 8;
}

uint
int_to_hex_getlen_atleast8(int64 a)
{
    const uint len = int_to_hex_getlen (a);
    return std::max(len, 8u);
}

uint
int_to_hex_atleast8(int64 a, char *buf)
{
    const uint len = int_to_hex_getlen_atleast8 (a);
    int_to_hexlen(a, len, buf);
    return len;
}

struct stream
{
    virtual void write(const void* bytes, size_t count) = 0;
    void prints(const char* a) { write (a, strlen(a)); }
    void prints(const std::string& a) { prints(a.c_str()); }
    void printc(char a) { write (&a, 1); }
    void printf(const char* format, ...)
    {
        va_list va;
        va_start (va, format);
        printv (format, va);
        va_end (va);
    }

    void
    printv(const char *format, va_list va)
    {
        prints(string_vformat(format, va));
    }
};

struct stdout_stream : stream
{
    virtual void write(const void* bytes, size_t count)
    {
        fflush(stdout);
        const char* pc = (const char*)bytes;
        while (count > 0)
        {
            uint32 const n = (uint32)std::min(count, ((size_t)1024) * 1024 * 1024);
#if _MSC_VER
            ::_write(_fileno(stdout), pc, n);
#else
            ::write(fileno(stdout), pc, n);
#endif
            count -= n;
            pc += n;
        }
    }
};

struct stderr_stream : stream
{
    virtual void write(const void* bytes, size_t count)
    {
        fflush(stderr);
        const char* pc = (const char*)bytes;
        while (count > 0)
        {
            uint32 const n = (uint32)std::min(count, ((size_t)1024) * 1024 * 1024);
#if _MSC_VER
            ::_write(_fileno(stderr), pc, n);
#else
            ::write(fileno(stderr), pc, n);
#endif
            count -= n;
            pc += n;
        }
    }
};

void
print_fixed(const MetadataType_t*, Image*, uint table, uint row, uint column, void* data, uint size)
{
    char buf[64];
    uint64 i = 0;
    buf[0] = '0';
    buf[1] = 'x';

    switch (size)
    {
    case 1: i = *(uint8*)data;
            break;
    case 2: i =  *(uint16*)data;
            break;
    case 4: i =  *(uint32*)data;
            break;
    case 8: i =  *(uint64*)data;
            break;
    }
    uint len = 2 + int_to_hex_atleast8(i, &buf[2]);;
    buf[len++] = ' ';
    buf[len++] = 0;
    fputs(buf, stdout);
}

struct MetadataType_t
{
    const char *name;
    MetadataTypeFunctions_t const * functions;
    union {
        uint8 fixed_size;
        uint8 table_index;
        CodedIndex coded_index;
   };
};

uint32
loadedimage_metadata_size_codedindex_get (Image* image, CodedIndex coded_index);

//#define CODED_INDEX(x, n, values) uint32 metadata_size_codedindex_ ## x (MetadataType_t* type, Image* image) { return loadedimage_metadata_size_codedindex_get (image, type->coded_index); }
//CODED_INDICES
#undef CODED_INDEX

void
MetadataDecode_fixed (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_blob (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_string (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_ustring (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_codedindex (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_index (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_index_list (const MetadataType_t* type, void* output)
{
}

void
MetadataDecode_guid (const MetadataType_t* type, void* output)
{
}

uint
metadata_size_fixed (const MetadataType_t* type, Image* image)
{
    return type->fixed_size; // TODO? change from int8 to uint8? or uint to int?
}

uint
metadata_size_blob (const MetadataType_t* type, Image* image);

uint
metadata_size_string (const MetadataType_t* type, Image* image);

uint
metadata_size_ustring (const MetadataType_t* type, Image* image)
{
    return 4u;
}

uint32
metadata_size_codedindex (const MetadataType_t* type, Image* image)
{
    return loadedimage_metadata_size_codedindex_get(image, type->coded_index);
}

uint8
image_metadata_size_index (Image* image, uint /* todo enum */ table_index);

uint
metadata_size_index (const MetadataType_t* type, Image* image)
{
    return image_metadata_size_index (image, type->table_index);
}

uint
metadata_size_index_list (const MetadataType_t* type, Image* image)
{
    return metadata_size_index (type, image);
}

uint
metadata_size_guid (const MetadataType_t* type, Image* image);

const MetadataTypeFunctions_t MetadataType_Fixed =
{
    MetadataDecode_fixed,
    metadata_size_fixed,
    print_fixed,
};

const MetadataTypeFunctions_t MetadataType_blob_functions =
{
    MetadataDecode_blob,
    metadata_size_blob,
};

// TODO should format into memory
void
print_string(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size);

// TODO should format into memory
void
print_guid(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size);

// TODO should format into memory
void
print_index(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size);

// TODO should format into memory
void
print_indexlist(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size);

// TODO should format into memory
void
print_codedindex(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size);

const MetadataTypeFunctions_t MetadataType_string_functions =
{
    MetadataDecode_string,
    metadata_size_string,
    print_string,
};

const MetadataTypeFunctions_t MetadataType_guid_functions =
{
    MetadataDecode_guid,
    metadata_size_guid,
    print_guid,
};

const MetadataTypeFunctions_t MetadataType_ustring_functions =
{
    MetadataDecode_ustring,
    metadata_size_ustring,
};

const MetadataTypeFunctions_t MetadataType_Index =
{
    MetadataDecode_index,
    metadata_size_index,
    print_index,
};

const MetadataTypeFunctions_t MetadataType_IndexList =
{
    MetadataDecode_index_list,
    metadata_size_index, // TODO?
    print_indexlist,
};

const MetadataTypeFunctions_t MetadataType_CodedIndex =
{
    MetadataDecode_codedindex,
    metadata_size_codedindex,
    print_codedindex,
};

const MetadataType_t MetadataType_int8 = { "int8", &MetadataType_Fixed, {1} };
const MetadataType_t MetadataType_int16 = { "int16", &MetadataType_Fixed, {2} };
const MetadataType_t MetadataType_int32 = { "int32", &MetadataType_Fixed, {4} };
const MetadataType_t MetadataType_int64 = { "int64", &MetadataType_Fixed, {8} };
const MetadataType_t MetadataType_uint8 = { "uint8", &MetadataType_Fixed, {1} };
const MetadataType_t MetadataType_uint16 = { "uint16", &MetadataType_Fixed, {2} };
const MetadataType_t MetadataType_uint32 = { "uint32", &MetadataType_Fixed, {4} };
const MetadataType_t MetadataType_uint64 = { "uint64", &MetadataType_Fixed, {8} };

// heap indices or offsets
const MetadataType_t MetadataType_string = { "string", &MetadataType_string_functions };
const MetadataType_t MetadataType_guid = { "guid", &MetadataType_guid_functions };
const MetadataType_t MetadataType_blob = { "blob",  &MetadataType_blob_functions };
// table indices
const MetadataType_t MetadataType_ResolutionScope = { "ResolutionScope", &MetadataType_CodedIndex, {(int8)CodedIndex(ResolutionScope)} };
const MetadataType_t MetadataType_Field = { "Field", &MetadataType_Index, {Field} };
const MetadataType_t MetadataType_HasCustomAttribute = { "HasCustomAttribute", &MetadataType_CodedIndex, {(int8)CodedIndex(HasCustomAttribute)} };
const MetadataType_t MetadataType_HasFieldMarshal = { "HasFieldMarshal", &MetadataType_CodedIndex, {(int8)CodedIndex(HasFieldMarshal)} };
const MetadataType_t MetadataType_HasSemantics = {" HasSemantics", &MetadataType_CodedIndex, {(int8)CodedIndex(HasSemantics)} };
const MetadataType_t MetadataType_HasDeclSecurity = { "HasDeclSecurity", &MetadataType_CodedIndex, {(int8)CodedIndex(HasDeclSecurity)} };
const MetadataType_t MetadataType_GenericParam = { "GenericParam", &MetadataType_Index, {GenericParam} };
const MetadataType_t MetadataType_MemberForwarded = { "MemberForwarded", &MetadataType_CodedIndex, {(int8)CodedIndex(MemberForwarded)} };
const MetadataType_t MetadataType_MemberRefParent = { "MemberRefParent", &MetadataType_CodedIndex, {(int8)CodedIndex(MemberRefParent)} };
const MetadataType_t MetadataType_MethodDef = { "MethodDef", &MetadataType_Index, {MethodDef} };
const MetadataType_t MetadataType_MethodDefOrRef = { "MethodDefOrRef", &MetadataType_CodedIndex, {(int8)CodedIndex(MethodDefOrRef)} };
const MetadataType_t MetadataType_Property = { "Property", &MetadataType_Index, {Property} };
const MetadataType_t MetadataType_TypeDefOrRef = { "TypeDefOrRef", &MetadataType_CodedIndex, {(int8)CodedIndex(TypeDefOrRef)} };
const MetadataType_t MetadataType_TypeDef = { "TypeDef", &MetadataType_Index, {TypeDef} };
const MetadataType_t MetadataType_CustomAttributeType = { "CustomAttributeType", &MetadataType_CodedIndex, {(int8)CodedIndex(CustomAttributeType)} };
const MetadataType_t MetadataType_Implementation = { "Implementation", &MetadataType_CodedIndex, {(int8)CodedIndex(Implementation)} };
const MetadataType_t MetadataType_TypeOrMethodDef = { "TypeOrMethodDef", &MetadataType_CodedIndex, {(int8)CodedIndex(TypeOrMethodDef)} };
const MetadataType_t MetadataType_ModuleRef = { "TypeDef", &MetadataType_Index, {ModuleRef} };

// Lists go to end of table, or start of next list, referenced from next element of same table
const MetadataType_t MetadataType_EventList = { "EventList", &MetadataType_IndexList, {Event} };
const MetadataType_t MetadataType_FieldList = { "FieldList", &MetadataType_IndexList, {Field} };
const MetadataType_t MetadataType_MethodList = { "MethodList", &MetadataType_IndexList, {MethodDef} };
const MetadataType_t MetadataType_ParamList = { "ParamList", &MetadataType_IndexList, {Param} };
const MetadataType_t MetadataType_PropertyList = { "PropertyList", &MetadataType_IndexList, {Property} };
const MetadataType_t MetadataType_Unused = { "Unused" /* TODO runtime error */ };
const MetadataType_t MetadataType_NotStored = { "NotStored", &MetadataType_Fixed };

// enums/flags
#define MetadataType_TypeFlags          MetadataType_uint32 /* TODO? */
#define MetadataType_FieldFlags         MetadataType_uint16 /* TODO? */
#define MetadataType_MethodDefFlags     MetadataType_uint16 /* TODO? */
#define MetadataType_MethodDefImplFlags MetadataType_uint16 /* TODO? */
#define MetadataType_Interface          MetadataType_TypeDefOrRef /* or spec, TODO? creater? */

#define MetadataType_Signature          MetadataType_blob /* TODO? decode and maybe creater */
#define MetadataType_Name               MetadataType_string
#define MetadataType_RVA                MetadataType_uint32
#define MetadataType_Extends            MetadataType_TypeDefOrRef
#define MetadataType_Sequence           MetadataType_uint16
#define MetadataType_Mvid               MetadataType_guid
#define MetadataType_TypeName           MetadataType_string
#define MetadataType_TypeNameSpace      MetadataType_string
#define MetadataType_Unused             MetadataType_Unused
#define MetadataType_Parent             MetadataType_MemberRefParent

struct MetadataTableSchemaColumn
{
    const char* name;
    const MetadataType_t* type; // old compilers do not allow reference here, with initialization
};

struct MetadataTableSchema
{
    const char *name;
    uint8 count;
    const MetadataTableSchemaColumn* fields;
    //void (*unpack)();
};

struct EmptyBase_t
{
};

#define NOTHING /* nothing */

// The ordering of the tables here is important -- it assigns their enums.
// The ordering of the columns within the tables is also important.
// The second parameter to METADATA_COLUMN can go away.
#define METADATA_TABLES                                                 \
/* table0x00*/ METADATA_TABLE (Module, NOTHING,                         \
    METADATA_COLUMN2 (Generation, uint16) /* ignore */                  \
    METADATA_COLUMN (Name)                                              \
    METADATA_COLUMN (Mvid)                                              \
    METADATA_COLUMN2 (EncId, guid) /* ignore */                         \
    METADATA_COLUMN2 (EncBaseId, guid)) /* ignore */                    \
                                                                        \
/*table0x00*/ METADATA_TABLE (TypeRef, NOTHING,                         \
    METADATA_COLUMN (ResolutionScope)                                   \
    METADATA_COLUMN (TypeName)                                          \
    METADATA_COLUMN (TypeNameSpace))                                    \
                                                                        \
/*table0x01*/ METADATA_TABLE (TypeDef, NOTHING,                         \
    METADATA_COLUMN3 (Flags, uint32, TypeFlags_t)                       \
    METADATA_COLUMN (TypeName)                                          \
    METADATA_COLUMN (TypeNameSpace)                                     \
    METADATA_COLUMN (Extends)                                           \
    METADATA_COLUMN (FieldList)                                         \
    METADATA_COLUMN (MethodList))                                       \
                                                                        \
/*table0x03*/ METADATA_TABLE_UNUSED(3)                                  \
                                                                        \
/*table0x04*/ METADATA_TABLE (Field, : Member_t,                        \
    METADATA_COLUMN2 (Flags, FieldFlags)                                \
    METADATA_COLUMN (Name)                                              \
    METADATA_COLUMN (Signature))                                        \
                                                                        \
/*table0x05*/ METADATA_TABLE_UNUSED(5) /*MethodPtr nonstandard*/        \
                                                                        \
/*table0x06*/METADATA_TABLE (MethodDef, NOTHING,                        \
    METADATA_COLUMN (RVA)                                               \
    METADATA_COLUMN2 (ImplFlags, MethodDefImplFlags) /* TODO higher level support */     \
    METADATA_COLUMN2 (Flags, MethodDefFlags) /* TODO higher level support */             \
    METADATA_COLUMN (Name)                                              \
    METADATA_COLUMN (Signature)      /* Blob heap, 7 bit encode/decode */         \
    METADATA_COLUMN (ParamList)) /* Param table, start, until table end, or start of next MethodDef; index into Param table, 2 or 4 bytes */ \
                                                                        \
/*table0x07*/ METADATA_TABLE_UNUSED(7) /*ParamPtr nonstandard*/         \
                                                                        \
/*table0x08*/METADATA_TABLE (Param, NOTHING,                            \
    METADATA_COLUMN2 (Flags, uint16)                                    \
    METADATA_COLUMN (Sequence)                                          \
    METADATA_COLUMN (Name))                                             \
                                                                        \
/*table0x09*/METADATA_TABLE (InterfaceImpl, NOTHING,                    \
    METADATA_COLUMN2 (Class, TypeDef)                                   \
    METADATA_COLUMN (Interface)) /* TypeDefOrRef or Spec */             \
                                                                        \
/*table0x0A*/METADATA_TABLE (MemberRef, NOTHING,                        \
    METADATA_COLUMN2 (Class, MemberRefParent)                           \
    METADATA_COLUMN (Name)       /*string*/                             \
    METADATA_COLUMN (Signature)) /*blob*/                               \
                                                                        \
/*table0x0B*/METADATA_TABLE (Constant, NOTHING,                         \
    METADATA_COLUMN2 (Type, uint8)                                      \
    METADATA_COLUMN2 (Pad, uint8)                                       \
    METADATA_COLUMN (Parent)                                            \
    METADATA_COLUMN2 (Value, blob)                                      \
    METADATA_COLUMN2 (IsNull, NotStored))                               \
                                                                        \
/*table0x0C*/METADATA_TABLE (CustomAttribute, NOTHING,                  \
    METADATA_COLUMN2 (Parent, HasCustomAttribute)                       \
    METADATA_COLUMN2 (Type, CustomAttributeType)                        \
    METADATA_COLUMN2 (Value, blob))                                     \
                                                                        \
/*table0x0D*/METADATA_TABLE (FieldMarshal, NOTHING,                     \
    METADATA_COLUMN3 (Parent, HasFieldMarshal, FieldOrParam_t*)         \
    METADATA_COLUMN2 (NativeType, blob))                                \
                                                                        \
/*table0x0E*/METADATA_TABLE (DeclSecurity, NOTHING,                     \
    METADATA_COLUMN3 (Action, uint16, DeclSecurityAction_t)             \
    METADATA_COLUMN3 (Parent_or_Type_TODO, HasDeclSecurity, HasDeclSecurity_t)  \
    METADATA_COLUMN2 (PermissionSet_or_Value_TODO, blob))                       \
                                                                        \
/*table0x0F*/ METADATA_TABLE (ClassLayout, NOTHING,                     \
    METADATA_COLUMN2 (TODO, uint32))                                    \
                                                                        \
/*table0x10*/ METADATA_TABLE (FieldLayout, NOTHING,                     \
    METADATA_COLUMN2 (Offset, uint32)                                   \
    METADATA_COLUMN3 (Field, Field, Field_t*))                          \
                                                                        \
/*table0x11*/ METADATA_TABLE (StandaloneSig, NOTHING,                   \
    METADATA_COLUMN2 (Signature, blob))                                 \
                                                                        \
/*table0x12*/ METADATA_TABLE (EventMap, NOTHING,                        \
    METADATA_COLUMN2 (Parent, TypeDef)                                  \
    METADATA_COLUMN (EventList))                                        \
                                                                        \
/*table0x13*/ METADATA_TABLE_UNUSED(13)                                 \
                                                                        \
/*table0x14*/ METADATA_TABLE (metadata_Event, NOTHING,                  \
    METADATA_COLUMN3 (Flags, uint16, EventFlags_t)                      \
    METADATA_COLUMN2 (Name, string)                                     \
    METADATA_COLUMN2 (EventType, TypeDefOrRef))                         \
                                                                        \
/*table0x15*/ METADATA_TABLE (PropertyMap, NOTHING,                     \
    METADATA_COLUMN2 (Parent, TypeDef)                                  \
    METADATA_COLUMN (PropertyList))                                     \
                                                                        \
/*table0x16*/ METADATA_TABLE_UNUSED(16)                                 \
                                                                        \
/*table0x17*/ METADATA_TABLE (metadata_Property, NOTHING,               \
    METADATA_COLUMN2 (Flags, uint16)                                    \
    METADATA_COLUMN2 (Name, string)                                     \
    METADATA_COLUMN2 (Type, blob))                                      \
                                                                        \
/* .property and .event                                                 \
   Links Events and Properties to specific methods.                     \
   For example one Event can be associated to more methods.             \
   A property uses this table to associate get/set methods. */          \
/*table0x18*/ METADATA_TABLE (metadata_MethodSemantics, NOTHING,        \
    METADATA_COLUMN3 (Semantics, uint16, MethodSemanticsFlags_t)        \
    METADATA_COLUMN3 (Method, MethodDef, MethodDef_t*) /* index into MethodDef table, 2 or 4 bytes */ \
    METADATA_COLUMN3 (Association, HasSemantics, MethodSemanticsAssociation_t)) /* Event or Property, CodedIndex */ \
                                                                                \
/*table0x19*/ METADATA_TABLE (MethodImpl, NOTHING,                              \
    METADATA_COLUMN3 (Class, TypeDef, Class_t*)                                 \
    METADATA_COLUMN3 (MethodBody, MethodDefOrRef, MethodBody_t*)                \
    METADATA_COLUMN3 (MethodDeclaration, MethodDefOrRef, MethodDeclaration_t*)) \
                                                                                \
/*table0x1A*/ METADATA_TABLE (ModuleRef, NOTHING,                               \
    METADATA_COLUMN2 (Name, string))                                            \
                                                                                \
/*table0x1B*/ METADATA_TABLE (TypeSpec, NOTHING,                                \
    METADATA_COLUMN2 (Signature, blob))                                         \
                                                                                \
/*table0x1C*/ METADATA_TABLE (ImplMap, NOTHING,                                 \
    METADATA_COLUMN3 (MappingFlags, uint16, PInvokeAttributes)                  \
    METADATA_COLUMN3 (MemberForwarded, MemberForwarded, MethodDef_t)            \
    METADATA_COLUMN2 (ImportName, string)                                       \
    METADATA_COLUMN3 (ImportScope, ModuleRef, ModuleRef_t*))                    \
                                                                                \
/*table0x1D*/ METADATA_TABLE (FieldRVA, NOTHING,                                \
    METADATA_COLUMN2 (RVA, uint32)                                              \
    METADATA_COLUMN3 (Field, Field, Field_t*))                                  \
                                                                                \
/*table0x1E*/ METADATA_TABLE_UNUSED(1E)                                         \
                                                                                \
/*table0x1F*/ METADATA_TABLE_UNUSED(1F)                                         \
                                                                                \
/*table0x20*/ METADATA_TABLE (Assembly, NOTHING,                                \
    METADATA_COLUMN2 (HashAlgId, uint32)                                        \
    METADATA_COLUMN2 (MajorVersion, uint16)                                     \
    METADATA_COLUMN2 (MinorVersion, uint16)                                     \
    METADATA_COLUMN2 (BuildNumber, uint16)                                      \
    METADATA_COLUMN2 (RevisionNumber, uint16)                                   \
    METADATA_COLUMN3 (Flags, uint32, AssemblyFlags)                             \
    METADATA_COLUMN2 (PublicKey, blob)                                          \
    METADATA_COLUMN2 (Name, string)                                             \
    METADATA_COLUMN2 (Culture, string))                                         \
                                                                                \
/*table0x21*/ METADATA_TABLE (AssemblyProcessor, NOTHING,                       \
    METADATA_COLUMN2 (Processor, uint32))                                       \
                                                                                \
/*table0x22*/ METADATA_TABLE (AssemblyOS, NOTHING,                              \
    METADATA_COLUMN2 (OSPlatformID, uint32)                                     \
    METADATA_COLUMN2 (OSMajorVersion, uint32)                                   \
    METADATA_COLUMN2 (OSMinorVersion, uint32))                                  \
                                                                                \
/*table0x23*/ METADATA_TABLE (AssemblyRef, NOTHING,                             \
    METADATA_COLUMN2 (MajorVersion, uint16)                                     \
    METADATA_COLUMN2 (MinorVersion, uint16)                                     \
    METADATA_COLUMN2 (BuildNumber, uint16)                                      \
    METADATA_COLUMN2 (RevisionNumber, uint16)                                   \
    METADATA_COLUMN3 (Flags, uint32, AssemblyFlags))                            \
                                                                                \
/*table0x24*/ METADATA_TABLE (AssemblyRefProcessor, NOTHING,                    \
    METADATA_COLUMN2 (Processor, uint32)                                        \
    METADATA_COLUMN2 (AssemblyRef, uint32 /* index into AssemblyRef table but ignored */)) \
                                                                                \
/*table0x25*/ METADATA_TABLE (AssemblyRefOS, NOTHING,                           \
    METADATA_COLUMN2 (OSPlatformID, uint32)                                     \
    METADATA_COLUMN2 (MajorVersion, uint16)                                     \
    METADATA_COLUMN2 (MinorVersion, uint16)                                     \
    METADATA_COLUMN2 (RevisionNumber, uint16)                                   \
    METADATA_COLUMN2 (AssemblyRef, uint32 /* index into AssemblyRef table but ignored */)) \
                                                                                \
/*table0x26*/ METADATA_TABLE (File, NOTHING,                                    \
    METADATA_COLUMN3 (Flags, uint32, FileFlags_t)                               \
    METADATA_COLUMN2 (Name, string)                                             \
    METADATA_COLUMN2 (HashValue, blob))                                         \
                                                                                \
/*table0x27*/ METADATA_TABLE (ExportedType, NOTHING,                            \
    METADATA_COLUMN3 (Flags, uint32, TypeFlags_t)                               \
    METADATA_COLUMN3 (TypeDefId, uint32, uint32)                                \
    METADATA_COLUMN2 (TypeName, string)                                         \
    METADATA_COLUMN2 (TypeNameSpace, string)                                    \
    METADATA_COLUMN3 (Implementation, Implementation, MetadataToken_t))         \
                                                                                \
/*table0x28*/ METADATA_TABLE (ManifestResource, NOTHING,                        \
    METADATA_COLUMN2 (Offset, uint32)                                           \
    METADATA_COLUMN3 (Flags, uint32, ManifestResourceFlags_t)                   \
    METADATA_COLUMN2 (Name, string)                                             \
    METADATA_COLUMN3 (Implementation, Implementation, Implementation_t))        \
                                                                                \
/*table0x29*/ METADATA_TABLE (NestedClass, NOTHING,                             \
    METADATA_COLUMN2 (NestedClass, TypeDef)                                     \
    METADATA_COLUMN2 (EnclosingClass, TypeDef))                                 \
                                                                                \
/*table0x2A*/ METADATA_TABLE (GenericParam, NOTHING,                            \
    METADATA_COLUMN2 (Number, uint16)                                           \
    METADATA_COLUMN3 (Flags, uint16, GenericParamFlags_t)                       \
    METADATA_COLUMN3 (Owner, TypeOrMethodDef, TypeOrMethodDef_t)                \
    METADATA_COLUMN2 (Name, string))                                            \
                                                                                \
/*table0x2B*/ METADATA_TABLE (MethodSpec, NOTHING,                              \
    METADATA_COLUMN3 (Method, MethodDefOrRef, Method_t)                         \
    METADATA_COLUMN2 (Instantiation, blob))                                     \
                                                                                \
/*table0x2C*/ METADATA_TABLE (GenericParamConstraint, NOTHING,                  \
    METADATA_COLUMN3 (Owner, GenericParam, GenericParam_t)                      \
    METADATA_COLUMN2 (Constraint, TypeDefOrRef))                                \

// Every table has two maybe three maybe four sets of types/data/forms.
// 1. A very typed form. Convenient to work with. Does the most work to form.
// 2. A very tokeny form. Coded index become tokens. Does minimal work to form. Needed?
// 3. An indirect constant form that describes it enough to size and extract.
// 4. An indirect dynamic per-module form with column sizes and offsets.
//    This is just, again, column sizes, and offsets. Used in conjunction
//    with form 3. While a row of all fixed size/offset type is possible,
//    this is simply correctly sized arrays of size/offset and maybe link to form 3.
#define metadata_schema_TYPED_blob              Blob_t
#define metadata_schema_TYPED_guid              Guid_t
#define metadata_schema_TYPED_string            String_t
#define metadata_schema_TYPED_uint16            uint16
#define metadata_schema_TYPED_uint32            uint32
#define metadata_schema_TYPED_uint8             uint8
#define metadata_schema_TYPED_Class             Class_t*
#define metadata_schema_TYPED_Extends           voidp /* TODO union? */
#define metadata_schema_TYPED_FieldFlags        FieldFlags_t
#define metadata_schema_TYPED_FieldList         std::vector<Field_t*>
#define metadata_schema_TYPED_Interface         Interface_t*
#define metadata_schema_TYPED_MemberRefParent   Parent_t
#define metadata_schema_TYPED_MethodDefFlags    MethodDefFlags_t
#define metadata_schema_TYPED_MethodDefImplFlags  MethodDefImplFlags_t
#define metadata_schema_TYPED_MethodList        std::vector<Method_t*>
#define metadata_schema_TYPED_Mvid              Guid_t
#define metadata_schema_TYPED_Name              String_t
#define metadata_schema_TYPED_ParamList         std::vector<Param_t*>
#define metadata_schema_TYPED_PropertyList      std::vector<Property_t*>
#define metadata_schema_TYPED_Parent            Parent_t
#define metadata_schema_TYPED_RVA               uint32
#define metadata_schema_TYPED_ResolutionScope   voidp /* TODO union? */
#define metadata_schema_TYPED_Sequence          uint16
#define metadata_schema_TYPED_Signature         Signature_t
#define metadata_schema_TYPED_TypeDef           Type_t*
#define metadata_schema_TYPED_TypeDefOrRef      voidp /* TODO union? */
#define metadata_schema_TYPED_TypeFlags         TypeFlags_t
#define metadata_schema_TYPED_TypeName          String_t
#define metadata_schema_TYPED_TypeNameSpace     String_t
#define metadata_schema_TYPED_Unused            Unused_t
#define metadata_schema_TYPED_NotStored         Unused_t
#define metadata_schema_TYPED_CustomAttributeType CustomAttributeType_t
#define metadata_schema_TYPED_HasCustomAttribute HasCustomAttribute_t
#define metadata_schema_TYPED_EventList         std::vector<Event_t*>

// needed?
//#define metadata_schema_TOKENED_string           MetadataString_t
//#define metadata_schema_TOKENED_uint16           uint16
//#define metadata_schema_TOKENED_guid             Guid_t
//#define metadata_schema_TOKENED_ResolutionScope  MetadataToken_t /* TODO */
//#define metadata_schema_TOKENED_TypeDefOrRef     MetadataToken_t /* TODO */
//#define metadata_schema_TOKENED_FieldList        MetadataTokenList_t
//#define metadata_schema_TOKENED_MethodList       MetadataTokenList_t

#define METADATA_COLUMN(name) METADATA_COLUMN2 (name, name)

#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_TABLE(name, base, columns)                         name, // TODO longer name?
#define METADATA_COLUMN2(name, type)                                /* nothing */
#define METADATA_COLUMN3(name, persistant_type, pointerful_type)    /* nothing */
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) 0
//BEGIN_ENUM MetadataTableIndex {
//    METADATA_TABLES
//};

#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_TABLE(name, base, columns)  struct name ##  _t base { name ##  _t ( ) { } columns };
#define METADATA_COLUMN2(name, type) metadata_schema_TYPED_ ## type name;
#define METADATA_COLUMN3(name, persistant_type, pointerful_type) pointerful_type name;
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
METADATA_TABLES

#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_TABLE(name, base, columns) \
const MetadataTableSchemaColumn metadata_column_ ## name [ ] = { columns }; \
const MetadataTableSchema metadata_row_schema_ ## name = { #name, CountOf (metadata_column_ ## name), metadata_column_ ## name };
#define METADATA_COLUMN2(name, type) { # name, &MetadataType_  ## type },
#define METADATA_COLUMN3(name, persistant_type, pointerful_type) { # name, &MetadataType_  ## persistant_type },
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
METADATA_TABLES

/*
const int8 ClassLayout = 15;
const int8 MemberRef = 10;
const int8 MethodRef = MemberRef;
const int8 FieldRef = MemberRef;
const int8 Constant = 11;
const int8 CustomAttribute = 12;
const int8 FieldMarshal = 13;
const int8 DeclSecurity = 14;
const int8 FieldLayout = 16;
const int8 EventMap = 18;
const int8 Event = 20;
const int8 PropertyMap = 21;
const int8 Property = 23;
const int8 MethodSemantics = 24; // 0x18
const int8 MethodImpl = 25; // 0x19
const int8 ModuleRef = 26;
const int8 TypeSpec = 27;
const int8 AssemblyRef = 35;
const int8 AssemblyRefProcessor = 36;
const int8 AssemblyRefOS = 37;
const int8 MethodSpec = 0x2B;
*/


struct MetadataColumn;
struct DynamicTableColumnFunctions_t;

struct DynamicTableColumnFunctions_t
{
};

struct MetadataColumn // dynamic
{
    MetadataColumn()
    {
        memset (this, 0, sizeof (*this));
    }

    uint size;
    uint offset;
};

struct MetadataTable // dynamic
{
    MetadataTable() { memset (this, 0, sizeof (*this)); }

    void* base;
    MetadataColumn* column;
    uint row_count;
    uint row_size;
    uint8 index_size; // 2 or 4
    uint8 column_count;
    bool present;
    int8 name_column;
    bool name_column_valid;
};

struct Metadata // dynamic
{
    union {
        MetadataTable array[
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, columns) 1+
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) 1+
    METADATA_TABLES
            0];
        struct
        {
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) MetadataTable unused_ ## name;
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, columns) MetadataTable name;
METADATA_TABLES
        } name;
    };

    MetadataColumn columns[
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_COLUMN2(name, type)                                1+
#define METADATA_COLUMN3(name, persistant_type, pointerful_type)    1+
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
#define METADATA_TABLE(name, base, columns) columns
METADATA_TABLES
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
    0];

    Metadata()
    {
        memset (this, 0, sizeof (*this));
        MetadataColumn* c = columns;
        MetadataTable* t = array;
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_COLUMN2(name, type)                                1+
#define METADATA_COLUMN3(name, persistant_type, pointerful_type)    1+
#define METADATA_TABLE(name, base, columns) t->column = c; t->column_count = columns 0; c += columns 0; ++t;
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) ++t;
METADATA_TABLES
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
    }
};
#undef METADATA_TABLE

static const char * const table_names [ ] =
{
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_TABLE(name, base, columns) #name,
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) "unused_" #name,
METADATA_TABLES
#undef METADATA_TABLE
};

const char * GetTableName (uint a)
{
    if (a < CountOf (table_names))
        return table_names [a];
    return "unknown";
}

static const MetadataTableSchema *  const metadata_int_to_table_schema [ ] =
{
#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_COLUMN2(name, type)                                /* nothing */
#define METADATA_COLUMN3(name, persistant_type, pointerful_type)    /* nothing */
#define METADATA_TABLE(name, base, columns) &metadata_row_schema_ ## name,
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) 0,
METADATA_TABLES
};

struct ImageZero // zero-inited part of Image
{
    ImageZero() { memset (this, 0, sizeof (*this)); }

    uint8 coded_index_size [CodedIndex_Count]; // 2 or 4
    uint64 file_size;
    struct
    {
        MetadataStreamHeader* tables;
        MetadataStreamHeader* guid;
        MetadataStreamHeader* string; // utf8
        MetadataStreamHeader* ustring; // unicode/user strings
        MetadataStreamHeader* blob;
    } streams;
    MetadataRoot* metadata_root;
    void* base;
    image_dos_header_t* dos;
    uchar* pe;
    image_nt_headers_t* nt;
    image_optional_header32_t* opt32;
    image_optional_header64_t* opt64;
    char* strings;
    char* guids;
    uint pe_offset;
    uint32 opt_magic;
    uint32 NumberOfRvaAndSizes;
    uint16 number_of_sections;
    uint16 number_of_streams;
    uint8 blob_size; // 2 or 4
    uint8 string_size; // 2 or 4
    uint8 guid_size; // 2 or 4
};

struct Image : ImageZero
{
    memory_mapped_file_t mmf;

    std::vector<image_section_header_t*> section_headers;

    Metadata metadata;

    char* get_string(int a)
    {
        return streams.string->offset + a + (char*)metadata_root;
    }

    Guid_t* get_guid(int a)
    {
        return a + (Guid_t*)(streams.guid->offset + (char*)metadata_root);
    }

    uint LayoutTable (uint table_index)
    {
        const MetadataTableSchema* schema = metadata_int_to_table_schema [table_index];
        if (!schema)
            return 0;
        MetadataTable* table = &metadata.array[table_index];
        const uint count = schema->count;
        uint size = 0;
        for (uint i = 0; i < count; ++i)
        {
            const MetadataTableSchemaColumn* field = &schema->fields [i];
            uint field_size = field->type->functions->size (field->type, this);
            table->column[i].offset = size;
            table->column[i].size = field_size;
            size += field_size;
            if (!table->name_column_valid)
            {
                if (strcmp(field->name, "TypeName") == 0 || strcmp(field->name, "Name") == 0)
                {
                    table->name_column = (int8)i; // TODO
                    table->name_column_valid = true;
                }
                else if (strcmp(field->name, "name") == 0)
                {
                    abort();
                }
            }
        }
        table->row_size = size;
        return size;
    }

    uint GetRowSize (uint table_index)
    {
        uint row_size = metadata.array [table_index].row_size;
        return row_size ? row_size : LayoutTable (table_index);
    }

    void DumpTable (uint table_index)
    {
        stdout_stream out;
        stderr_stream err;

        // TODO less printf
        std::string prefix = string_format ("table 0x%08X (%s)", table_index, GetTableName (table_index));
        const char* prefix_cstr = prefix.c_str();

        const MetadataTableSchema* schema = metadata_int_to_table_schema [table_index];
        const MetadataTableSchemaColumn *fields = schema ? schema->fields : 0;
        const uint count = schema ? schema->count : 0;

        //err.printf ("%s\n", prefix_cstr);
        err.printf ("%s present:0x%08X\n", prefix_cstr, metadata.array [table_index].present);
        err.printf ("%s row_size:0x%08X\n", prefix_cstr, GetRowSize (table_index));
        err.printf ("%s column_count:0x%08X\n", prefix_cstr, count);
        err.printf ("%s row_count:0x%08X\n", prefix_cstr, metadata.array [table_index].row_count);

        uint i;
        for (i = 0; i < count; ++i)
        {
            const MetadataTableSchemaColumn *field = &fields[i];
            out.printf("%s layout type:%s name:%s offset:0x%08X size:0x%08X\n", GetTableName (table_index), field->type->name, field->name, metadata.array[table_index].column[i].offset, metadata.array[table_index].column[i].size);
        }
        char* p = (char*)metadata.array[table_index].base;
        for (uint r = 0; r < metadata.array [table_index].row_count; ++r)
        {
            out.printf("%s[0x%08X] ", GetTableName (table_index), r);
            for (i = 0; i < count; ++i)
            {
                const MetadataTableSchemaColumn *field = &fields[i];
                const uint column_size = metadata.array[table_index].column[i].size;
                out.printf("col[%X]%s:%s:", i, field->type->name, field->name);
                if (field->type->functions->print)
                    field->type->functions->print(field->type, this, table_index, r, i, p, column_size);
                p += column_size;
            }
            out.prints("\n");
        }
        out.prints("\n");
    }

    void init (const char *file_name)
    {
        mmf.read (file_name);
        base = mmf.base;
        file_size = mmf.file.get_file_size ();
        dos = (image_dos_header_t*)base;
        printf ("mz: %02x%02x\n", ((uchar*)dos) [0], ((uchar*)dos) [1]);
        if (memcmp (base, "MZ", 2))
            throw_string (string_format ("incorrect MZ signature %s", file_name));
        printf ("mz: %c%c\n", ((char*)dos) [0], ((char*)dos) [1]);
        pe_offset = dos->get_pe ();
        printf ("pe_offset: %#x\n", pe_offset);
        pe = (pe_offset + (uchar*)base);
        printf ("pe: %02x%02x%02x%02x\n", pe [0], pe [1], pe [2], pe [3]);
        if (memcmp (pe, "PE\0\0", 4))
            throw_string (string_format ("incorrect PE00 signature %s", file_name));
        printf ("pe: %c%c\\0x%08X\\0x%08X\n", pe [0], pe [1], pe [2], pe [3]);
        nt = (image_nt_headers_t*)pe;
        printf ("Machine:0x%08X\n", nt->FileHeader.Machine);
        printf ("NumberOfSections:0x%08X\n", nt->FileHeader.NumberOfSections);
        printf ("TimeDateStamp:0x%08X\n", nt->FileHeader.TimeDateStamp);
        printf ("PointerToSymbolTable:0x%08X\n", nt->FileHeader.PointerToSymbolTable);
        printf ("NumberOfSymbols:0x%08X\n", nt->FileHeader.NumberOfSymbols);
        printf ("SizeOfOptionalHeader:0x%08X\n", nt->FileHeader.SizeOfOptionalHeader);
        printf ("Characteristics:0x%08X\n", nt->FileHeader.Characteristics);
        opt32 = (image_optional_header32_t*)(&nt->OptionalHeader);
        opt64 = (image_optional_header64_t*)(&nt->OptionalHeader);
        opt_magic = opt32->Magic;
        release_assertf ((opt_magic == 0x10b && !(opt64 = 0)) || (opt_magic == 0x20b && !(opt32 = 0)), ("file:%s opt_magic:%x", file_name, opt_magic));
        printf ("opt.magic:%x opt32:%p opt64:%p\n", opt_magic, (void*)opt32, (void*)opt64);
        NumberOfRvaAndSizes = opt32 ? opt32->NumberOfRvaAndSizes : opt64->NumberOfRvaAndSizes;
        printf ("opt.rvas:0x%08X\n", NumberOfRvaAndSizes);
        number_of_sections = nt->FileHeader.NumberOfSections;
        printf ("number_of_sections:0x%08X\n", number_of_sections);
        image_section_header_t* section_header = nt->first_section_header ();
        uint i = 0;
        for (i = 0; i < number_of_sections; ++i, ++section_header)
            printf ("section [%02X].Name: %.8s\n", i, section_header->Name);
        image_data_directory_t* DataDirectory = opt32 ? opt32->DataDirectory : opt64->DataDirectory;
        for (i = 0; i < NumberOfRvaAndSizes; ++i)
        {
            printf ("DataDirectory [%02X].Offset: 0x%08X\n", i, DataDirectory[i].VirtualAddress);
            printf ("DataDirectory [%02X].Size: 0x%08X\n", i, DataDirectory[i].Size);
        }
        release_assertf (DataDirectory [14].VirtualAddress, ("Not a .NET image? %x", DataDirectory [14].VirtualAddress));
        release_assertf (DataDirectory [14].Size, ("Not a .NET image? %x", DataDirectory [14].Size));
        image_clr_header_t* clr = (image_clr_header_t*)rva_to_p(DataDirectory [14].VirtualAddress);
        printf ("clr.cb:0x%08X\n", clr->cb);
        printf ("clr.MajorRuntimeVersion:0x%08X\n", clr->MajorRuntimeVersion);
        printf ("clr.MinorRuntimeVersion:0x%08X\n", clr->MinorRuntimeVersion);
        printf ("clr.MetaData.Offset:0x%08X\n", clr->MetaData.VirtualAddress);
        printf ("clr.MetaData.Size:0x%08X\n", clr->MetaData.Size);
        release_assertf (clr->MetaData.Size, ("0x%08X", clr->MetaData.Size));
        release_assertf (clr->cb >= sizeof (image_clr_header_t), ("0x%08X 0x%08X", clr->cb, (uint)sizeof (image_clr_header_t)));
        metadata_root = (MetadataRoot*)rva_to_p(clr->MetaData.VirtualAddress);
        printf ("metadata_root.Signature:0x%08X\n", metadata_root->Signature);
        printf ("metadata_root.MajorVersion:0x%08X\n", metadata_root->MajorVersion);
        printf ("metadata_root.MinorVersion:0x%08X\n", metadata_root->MinorVersion);
        printf ("metadata_root.Reserved:0x%08X\n", metadata_root->Reserved);
        printf ("metadata_root.VersionLength:0x%08X\n", metadata_root->VersionLength);
        release_assertf ((metadata_root->VersionLength % 4) == 0, ("0x%08X", metadata_root->VersionLength));
        size_t VersionLength = strlen(metadata_root->Version);
        release_assertf (VersionLength < metadata_root->VersionLength, ("0x%08X 0x%08X", VersionLength, metadata_root->VersionLength));
        // TODO bounds checks throughout
        uint16* pflags = (uint16*)&metadata_root->Version[metadata_root->VersionLength];
        uint16* pnumber_of_streams = 1 + pflags;
        number_of_streams = *pnumber_of_streams;
        printf ("metadata_root.Version:%s\n", metadata_root->Version);
        printf ("flags:0x%08X\n", *pflags);
        printf ("number_of_streams:0x%08X\n", number_of_streams);
        MetadataStreamHeader* stream = (MetadataStreamHeader*)(pnumber_of_streams + 1);
        for (i = 0; i < number_of_streams; ++i)
        {
            printf ("stream[0x%08X].Offset:0x%08X\n", i, stream->offset);
            printf ("stream[0x%08X].Size:0x%08X\n", i, stream->Size);
            release_assertf ((stream->Size % 4) == 0, ("0x%08X", stream->Size));
            const char* name = stream->Name;
            size_t length = strlen (name);
            release_assertf (length <= 32, ("0x%08X:%s", length, name));
            printf ("stream[0x%08X].Name:0x%08X:%.*s\n", i, (int)length, (int)length, name);
            if (length >= 2 && name [0] == '#')
            {
                ++name;
                if (length == 2 && name [0] == '~')
                    streams.tables = stream;
                else if (length == 5 && memcmp (name, "GUID", 4) == 0)
                    streams.guid = stream;
                else if (length == 5 && memcmp (name, "Blob", 4) == 0)
                    streams.blob = stream;
                else if (length == 8 && memcmp (name, "Strings", 7) == 0)
                    streams.string = stream;
                else if (length == 3 && name [0] == 'U' && name [1] == 'S')
                    streams.ustring = stream;
                else
                    goto unknown_stream;
            }
            else {
unknown_stream:
                fprintf (stderr, "unknown stream %s\n", stream->Name);
                abort ();
            }
            length = (length + 4) & -4;
            stream = (MetadataStreamHeader*)(stream->Name + length);
        }
        MetadataTablesHeader* metadata_tables_header = (MetadataTablesHeader*)(streams.tables->offset + (char*)metadata_root);
        printf ("metadata_tables_header.reserved:0x%08X\n", metadata_tables_header->reserved);
        printf ("metadata_tables_header.MajorVersion:0x%08X\n", metadata_tables_header->MajorVersion);
        printf ("metadata_tables_header.MinorVersion:0x%08X\n", metadata_tables_header->MinorVersion);
        printf ("metadata_tables_header.HeapSizes:0x%08X\n", metadata_tables_header->HeapSizes);
        uint const HeapSize = metadata_tables_header->HeapSizes;
        string_size = (HeapSize & HeapOffsetSize_String) ? 4u : 2u;
        guid_size = (HeapSize & HeapOffsetSize_Guid) ? 4u: 2u;
        blob_size = (HeapSize & HeapOffsetSize_Blob) ? 4u : 2u;
        printf ("metadata_tables_header.reserved2:0x%08X\n", metadata_tables_header->reserved2);
        uint64 valid = metadata_tables_header->Valid;
        uint64 sorted = metadata_tables_header->Sorted;
        uint64 unsorted = valid & ~sorted;
        uint64 invalidSorted = sorted & ~valid;
        printf ("metadata_tables_header.        Valid:0x%08X`0x%08X\n", (uint32)(valid >> 32), (uint32)valid);
        // Mono does not use sorted, and there are bits set beyond Valid.
        // I suspect this has to do with writing/appending, and occasionally sorting.
        printf ("metadata_tables_header.       Sorted:0x%08X`0x%08X\n", (uint32)(sorted >> 32), (uint32)sorted);
        printf ("metadata_tables_header.     Unsorted:0x%08X`0x%08X\n", (uint32)(unsorted >> 32), (uint32)unsorted);
        printf ("metadata_tables_header.InvalidSorted:0x%08X`0x%08X\n", (uint32)(invalidSorted >> 32), (uint32)invalidSorted);
        uint64 mask = 1;
        uint32* prow_count = (uint32*)(metadata_tables_header + 1);
        char* table_base = (char*)prow_count;

        for (mask = 1, i = 0; i < CountOf (metadata.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (present)
                table_base += 4;
        }

        for (mask = 1, i = 0; i < CountOf (metadata.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (!present)
                continue;
            metadata.array [i].present = true;
            const uint row_count = *prow_count;
            metadata.array [i].row_count = row_count;
            metadata.array [i].base = table_base;
            LayoutTable(i);
            table_base += metadata.array[i].row_size * row_count;
            ++prow_count;
        }
#if 0
        DumpTable (0x0B);
        DumpTable (0);
        DumpTable (1);
        DumpTable (2);
        DumpTable (3);
        DumpTable (4);
        DumpTable (5);
        DumpTable (5);
        DumpTable (6);
        DumpTable (7);
        DumpTable (8);
        DumpTable (9);
        DumpTable (0x0A);
        DumpTable (0x0C);
        DumpTable (0x0D);
        DumpTable (0x0E);
        DumpTable (0x0F);
        DumpTable (0x10);
        DumpTable (0x11);
        DumpTable (0x12);
        DumpTable (0x13);
        DumpTable (0x14);
        DumpTable (0x15);
        DumpTable (0x16);
        DumpTable (0x17);
        DumpTable (0x18);
        DumpTable (0x19);
        DumpTable (0x1A);
        DumpTable (0x1B);
        DumpTable (0x1C);
        DumpTable (0x1D);
#endif

        if (0) for (mask = 1, i = 0; i < CountOf (metadata.array); ++i, mask <<= 1)
        {
#if 1
            bool present = (valid & mask) != 0;
            if (present)
            {
                fprintf (stderr, "table 0x%08X (%s) has 0x%08X rows (%s)\n", i, GetTableName (i), metadata.array [i].row_count, (sorted & mask) ? "sorted" : "unsorted");
                [&] {
                    __try
                    {
                        DumpTable (i);
                    }
                    __except(1)
                    {
                        fprintf(stderr, "table 0x%X failed\n", i);
                    }
                }();
            }
            else
            {
                fprintf (stderr, "table 0x%08X (%s) is absent\n", i, GetTableName (i));
            }
#endif
        }
    }

    void* rva_to_p (uint32 rva)
    {
        rva = rva_to_file_offset (rva);
        return rva ? (((char*)base) + rva) : 0;
    }

    uint32 rva_to_file_offset (uint32 rva)
    {
        // TODO binary search and/or cache
        image_section_header_t* section_header = nt->first_section_header ();
        for (uint i = 0; i < number_of_sections; ++i, ++section_header)
        {
            uint32 va = section_header->VirtualAddress;
            if (rva >= va && rva < (va + section_header->SizeOfRawData))
                return section_header->PointerToRawData + (rva - va);
        }
        return 0;
    }
};

void
print_stringx(const char* x, const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
    //__debugbreak();
    uint a = unpack_2_or_4le (data, image->string_size);
    //fputs(image->get_string(a), stdout);
    printf(" print_string:x:%s %X %p %s ", x, a, image->get_string(a), image->get_string(a));
}

void
print_string(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
    print_stringx("", type, image, table, row, column, data, size);
}

// TODO should format into memory
void
print_index(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
    //__debugbreak();
    uint index = unpack_2_or_4le (data, size);
    uint table_index = type->table_index;
    MetadataTable* t = &image->metadata.array[table_index];
    void* p = ((char*)t->base) + t->row_size * index;
    printf(" print_index:%s[%X][%X] => %s/%p ", GetTableName (table), row, column, GetTableName (table_index), p);

    if (t->name_column_valid)
        print_stringx ("xindex", 0, image, 0, 0, 0, t->column[t->name_column].offset + (char*)p, 0);
}

// TODO should format into memory
void
print_indexlist(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
}

// TODO should format into memory
void
print_codedindex(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
    //__debugbreak();
    uint32 code = unpack_2_or_4le (data, size);
    CodedIndex_t const * const coded_index = &CodedIndices.array[type->coded_index];

    uint index       = (code >> coded_index->tag_size);
    uint table_index = ((int8_t*)&CodedIndexMap)[coded_index->map + (code & ~(~0u << coded_index->tag_size))]; // TODO precompute

    MetadataTable* t = &image->metadata.array[table_index];
    void* p = ((char*)t->base) + t->row_size * index;
    printf(" print_codedindex:%s[%X][%X] => %s/%p ", GetTableName (table), row, column, GetTableName (table_index), p);

    if (t->name_column_valid)
        print_stringx ("xcodedindex", 0, image, 0, 0, 0, t->column[t->name_column].offset + (char*)p, 0);
}

#define GUID_FORMAT "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}"
#define GUID_BYTES(g) \
(g)->bytes[ 3], \
(g)->bytes[ 2], \
(g)->bytes[ 1], \
(g)->bytes[ 0], \
(g)->bytes[ 5], \
(g)->bytes[ 4], \
(g)->bytes[ 7], \
(g)->bytes[ 6], \
(g)->bytes[ 8], \
(g)->bytes[ 9], \
(g)->bytes[10], \
(g)->bytes[11], \
(g)->bytes[12], \
(g)->bytes[13], \
(g)->bytes[14], \
(g)->bytes[15] \

void
print_guid(const MetadataType_t* type, Image* image, uint table, uint row, uint column, void* data, uint size)
{
    uint a = unpack_2_or_4le (data, image->guid_size);
    //fputs(image->get_string(a), stdout);
    Guid_t* b = image->get_guid(a);
    printf("print_guid:%X %p " GUID_FORMAT, a, b, GUID_BYTES(b));
}

uint
metadata_size_blob (const MetadataType_t* type, Image* image)
{
    return image->blob_size;
}

uint
metadata_size_string (const MetadataType_t* type, Image* image)
{
    return image->string_size;
}

uint
metadata_size_guid (const MetadataType_t* type, Image* image)
{
    return image->guid_size;
}

uint32
metadata_size_codedindex_compute (Image* image, CodedIndex coded_index)
{
    const CodedIndex_t* data = &CodedIndices.array [coded_index];
    uint max_rows = 0;
    uint const map = data->map;
    uint const count = data->count;
    int8_t* Map = (int8_t*)&CodedIndexMap;
    for (uint i = 0; i < count; ++i)
    {
        int m = Map [map + i];
        if (m < 0)
            continue;
        max_rows = std::max (max_rows, image->metadata.array[m].row_count);
    }
    return (max_rows <= (0xFFFFu >> data->tag_size)) ? 2u : 4u;
}

uint32
loadedimage_metadata_size_codedindex_get (Image* image, CodedIndex coded_index)
{
    const uint a = image->coded_index_size [coded_index];
    if (a)
        return a;
    return metadata_size_codedindex_compute (image, coded_index);
}

uint8
metadata_size_index_compute (Image* image, uint /* todo enum */ table_index)
{
    const uint row_count = image->metadata.array [table_index].row_count;
    return (row_count <= 0xFFFFu) ? 2u : 4u;
}

uint8
image_metadata_size_index (Image* image, uint /* todo enum */ table_index)
{
    uint8& ra = image->metadata.array [table_index].index_size;
    uint8 a = ra;
    if (a)
        return a;
    return ra = metadata_size_index_compute (image, table_index);
}

}

using namespace m2;

int
main (int argc, char** argv)
{
#if 0 // test code
    char buf [99] = { 0 };
    uint len;
#define Xd(x) printf("%s %I64d\n", #x, x);
#define Xx(x) printf("%s %I64x\n", #x, x);
#define Xs(x) len = x; buf[len] = 0; printf("%s %s\n", #x, buf);
    Xd(uint_get_precision(0));
    Xd(uint_get_precision(1));
    Xd(uint_get_precision(0x2));
    Xd(uint_get_precision(0x2));
    Xd(uint_get_precision(0x7));
    Xd(uint_get_precision(0x8));
    Xd(int_get_precision(0));
    Xd(int_get_precision(1));
    Xd(int_get_precision(0x2));
    Xd(int_get_precision(0x2));
    Xd(int_get_precision(0x7));
    Xd(int_get_precision(0x8));
    Xd(int_get_precision(0));
    Xd(int_get_precision(-1));
    Xd(int_get_precision(-0x2));
    Xd(int_get_precision(-0x2));
    Xd(int_get_precision(-0x7));
    Xd(int_get_precision(-0x8));
    Xd(int_to_dec_getlen(0))
    Xd(int_to_dec_getlen(1))
    Xd(int_to_dec_getlen(2))
    Xd(int_to_dec_getlen(300))
    Xd(int_to_dec_getlen(-1))
    Xx(sign_extend(0xf, 0));
    Xx(sign_extend(0xf, 1));
    Xx(sign_extend(0xf, 2));
    Xx(sign_extend(0xf, 3));
    Xx(sign_extend(0xf, 4));
    Xx(sign_extend(0xf, 5));
    Xd(int_to_hex_getlen(0xffffffffa65304e4));
    Xd(int_to_hex_getlen(0xfffffffa65304e4));
    Xd(int_to_hex_getlen(-1));
    Xd(int_to_hex_getlen(-1ui64>>4));
    Xd(int_to_hex_getlen(0xf));
    Xd(int_to_hex_getlen(32767));
    Xd(int_to_hex_getlen(-32767));
    Xs(int_to_hex(32767, buf));
    Xs(int_to_hex(-32767, buf));
    Xs(int_to_hex8(0x123, buf));
    Xs(int_to_hex8(0xffffffffa65304e4, buf));
    Xs(int_to_hex8(-1, buf));
    Xs(int_to_hex(0x1, buf));
    Xs(int_to_hex(0x12, buf));
    Xs(int_to_hex(0x123, buf));
    Xs(int_to_hex(0x12345678, buf));
    Xs(int_to_hex(-1, buf));
    Xd(int_to_hex_getlen(0x1));
    Xd(int_to_hex_getlen(0x12));
    Xd(int_to_hex_getlen(0x12345678));
    Xd(int_to_hex_getlen(0x01234567));
    Xd(int_to_hex_getlen(-1));
    Xd(int_to_hex_getlen(~0u >> 1));
    Xd(int_to_hex_getlen(~0u >> 2));
    Xd(int_to_hex_getlen(~0u >> 4));
    exit(0);
#endif
    Image im;
#define X(x) printf ("%s %#x\n", #x, (int)x)
X (sizeof (image_dos_header_t));
X (sizeof (image_file_header_t));
X (sizeof (image_nt_headers_t));
X (sizeof (image_section_header_t));
X (CodedIndices.array[(uint)CodedIndex(TypeDefOrRef)].tag_size);
X (CodedIndices.array[(uint)CodedIndex(ResolutionScope)].tag_size);
X (CodedIndices.array[(uint)CodedIndex(HasConstant)].tag_size);
X (CodedIndices.array[(uint)CodedIndex(HasCustomAttribute)].tag_size);
X (CodedIndices.array[(uint)CodedIndex(HasFieldMarshal)].tag_size);
X (CodedIndices.array[(uint)CodedIndex(HasDeclSecurity)].tag_size);
#undef X
    try
    {
        im.init (argv [1]);
    }
    catch (int er)
    {
        fprintf (stderr, "error 0x%08X\n", er);
    }
    catch (const std::string& er)
    {
        fprintf (stderr, "%s", er.c_str());
    }
    return 0;
}
