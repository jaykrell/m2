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

#ifdef _MSC_VER

#pragma warning(disable:4100) // unused parameter
#pragma warning(disable:4510) // function could not be generated
#pragma warning(disable:4512) // function could not be generated
#pragma warning(disable:4514) // unused function
#pragma warning(disable:4610) // cannot be instantiated
#pragma warning(disable:4616) // not a valid warning
#pragma warning(disable:4619) // not a valid warning
#pragma warning(disable:4623) // default constructor deleted
#pragma warning(disable:4626) // assignment implicitly deleted
#pragma warning(disable:4706) // assignment within conditional
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4820) // padding
#pragma warning(disable:5027) // move assignment implicitly deleted
#pragma warning(push)
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
#include <algorithm>
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
#include <vector>
#include <string>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#if !defined(PRIX64)
#if defined(_WIN32)
#define PRIX64 "I64X"
#elif defined(_ILP64)
#else
#define PRIX64 "llX"
#endif
#endif

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

bool
string_vformat_internal (const char *format, std::vector<char>& s, va_list va)
{
#ifndef _MSC_VER
    va_list va2;
#ifdef __va_copy
    __va_copy (va2, va);
#else
    va_copy (va2, va);
#endif
    const int size = 2 + vsnprintf (0, 0, format, va);
#else
    va_list va2 = va;
    const int size = 2 + _vscprintf (format, va);
#endif
    s.resize ((size_t)size);
    return vsnprintf (&s [0], (size_t)size, format, va2) < size;
}

std::string
string_vformat (const char *format, va_list va)
{
    std::vector<char> s;
    while (!string_vformat_internal (format, s, va)) ;
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

#define not_implemented_yet() (assertf (0, ("not yet implemented %s %d ", __func__, __LINE__)))

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
    throw_string (string_format ("error %d %s\n", i, a));
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
    // TODO handle_t vs. win32file_t, etc.

    uint64 get_file_size (const char * file_name = "")
    {
        LARGE_INTEGER a = { };
        if (!GetFileSizeEx (h, &a)) // TODO NT4
            throw_LastError (string_format ("GetFileSizeEx(%s)", file_name).c_str());
        return (uint64)a.QuadPart;
    }

    void * h;

    handle_t (void *a) : h (a) { }
    handle_t () : h (0) { }

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

#ifndef _WIN32
    uint64 get_file_size (const char * file_name = "")
    {
#ifdef __CYGWIN__
        struct stat st = { 0 }; // TODO
        if (fstat (fd, &st))
#else
        struct stat64 st = { 0 }; // TODO
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
// TODO allow for redirection to built-in data
// TODO allow for systems that must read, not mmap
    void * base;
    size_t size;
#ifdef _WIN32
    handle_t file;
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
        if (!file) throw_LastError (string_format ("CreateFileA(%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size(a);
        handle_t h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
        if (!h2) throw_LastError (string_format ("CreateFileMapping(%s)", a).c_str ());
        base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
        if (!base) throw_LastError (string_format ("MapViewOfFile(%s)", a).c_str ());
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
    int64 offset;
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

enum _MethodDefFlags_t // table0x06
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
};
typedef uint16 MethodDefFlags_t; // table0x06

enum _MethodDefImplFlags_t // table0x06
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
};
typedef uint16 MethodDefImplFlags_t; // table0x06

struct Method_t : Member_t
{
};

enum _TypeFlags_t
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
};
typedef uint32 TypeFlags_t;

struct Type_t // class, valuetype, delegate, inteface, not char, short, int, long, float
{
};

enum _EventFlags_t
{
    EventFlags_SpecialName           =   0x0200,     // event is special. Name describes how.
    // Reserved flags for Runtime use only.
    EventFlags_RTSpecialName         =   0x0400      // Runtime(metadata internal APIs) should check name encoding.
};
typedef uint16 EventFlags_t;

struct Event_t : Member_t // table0x14
{
    EventFlags_t Flags;
    String_t Name;
    Type_t* EventType;
};

struct Property_t : Member_t
{
};

enum _FieldFlags_t
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
};
typedef uint16 FieldFlags_t;

enum _DeclSecurityAction_t
{
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
};
typedef uint16 DeclSecurityAction_t; // TODO get the values


struct Interface_t
{
    std::vector<Method_t*> methods;
};

struct FieldOrParam_t
{
};

struct Signature_t
{
};

struct Class_t
{
    Class_t* base;
    std::string name;
    std::vector<Interface_t> interfaces;
    std::vector<Method_t> methods;
    std::vector<Field_t> fields;
    std::vector<Event_t> events;
    std::vector<Property_t> properties;
};

class MethodBody_t
{
};

class MethodDeclaration_t
{
};

enum _MethodSemanticsFlags_t // CorMethodSemanticsAttr
{
    MethodSemanticsFlags_Setter = 1, // msSetter
    MethodSemanticsFlags_Getter = 2,
    MethodSemanticsFlags_Other = 4,
    MethodSemanticsFlags_AddOn = 8,
    MethodSemanticsFlags_RemoveOn = 0x10,
    MethodSemanticsFlags_Fire = 0x20
};
typedef uint16 MethodSemanticsFlags_t;

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
const uint Module = 0;
const uint TypeRef = 1;
const uint TypeDef = 2;
//const uint FieldPtr = 3; // nonstandard
const uint Field = 4;
//const uint MethodPtr = 5; // nonstandard
const uint MethodDef = 6;
//const uint ParamPtr = 7; // nonstandard
const uint Param = 8;
const uint InterfaceImpl = 9;
const uint MemberRef = 10;
const uint MethodRef = MemberRef;
const uint FieldRef = MemberRef;
const uint Constant = 11;
const uint CustomAttribute = 12;
const uint FieldMarshal = 13;
const uint DeclSecurity = 14;
const uint ClassLayout = 15;
const uint FieldLayout = 16;
const uint StandAloneSig = 17;
const uint EventMap = 18;
//const uint EventPtr = 19; // nonstandard
const uint Event = 20;
const uint PropertyMap = 21;
//const uint ProperyPtr = 22; // nonstandard
const uint Property = 23;
const uint MethodSemantics = 24; // 0x18
const uint MethodImpl = 25; // 0x19
const uint ModuleRef = 26;
const uint TypeSpec = 27;
const uint ImplMap = 28;
const uint FieldRVA = 29;
//const uint ENCLog = 30; // nonstandard
//const uint ENCMap = 31; // nonstandard
const uint Assembly = 32;
const uint AssemblyProcessor = 33;
const uint AssemblyOS = 34;
const uint AssemblyRef = 35;
const uint AssemblyRefProcessor = 36;
const uint AssemblyRefOS = 37;
const uint File = 38;
const uint ExportedType = 39;
const uint ManifestResource = 40; // 0x28
const uint NestedClass = 41;
const uint GenericParam = 42; // 0x2A
const uint MethodSpec = 0x2B;
const uint GenericParamConstraint = 44; // 0x2C

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
      {MethodDef COMMA     Field COMMA         TypeRef COMMA      TypeDef COMMA          Param COMMA          /* HasCustomAttribute */ \
      InterfaceImpl COMMA MemberRef COMMA     Module COMMA       DeclSecurity COMMA     Property COMMA       /* HasCustomAttribute */ \
      Event COMMA         StandAloneSig COMMA ModuleRef COMMA    TypeSpec COMMA         Assembly COMMA       /* HasCustomAttribute */ \
      AssemblyRef COMMA   File COMMA          ExportedType COMMA ManifestResource COMMA GenericParam COMMA   /* HasCustomAttribute */ \
      GenericParamConstraint COMMA MethodSpec                                          }) /* HasCustomAttribute */

#define CODED_INDEX(a, n, values) CodedIndex_ ## a,
enum _CodedIndex
{
CODED_INDICES
#undef CODED_INDEX
    CodedIndex_Count
};
typedef uint8 CodedIndex;


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
    uint32 index;
};

struct MetadataTokenList_t
{
    uint8 table;
    uint32 count;
    uint32 index;
};

struct MetadataTablesHeader_t // tilde stream
{
    uint32 reserved; // 0
    uint8 MajorVersion;
    uint8 MinorVersion;
    union {
        uint8 HeapOffsetSizes;
        uint8 HeapSizes;
    };
    uint8 reserved2; // 1
    uint64 Valid; // metadata_typedef etc.
    uint64 Sorted; // metadata_typedef etc.
    // uint32 NumberOfRows [];
};

struct MetadataRoot_t
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
    // MetadataStreamHeader_t stream_headers [NumberOfStreams];
};

struct MetadataStreamHeader_t // see mono verify_metadata_header
{
    uint32 Offset;
    uint32 Size; // multiple of 4
    char   Name [32]; // multiple of 4, null terminated, max 32
};

enum _ParamFlags_t // ParamAttributes
{
    ParamFlags_In                        =   0x0001,     // Param is [In]
    ParamFlags_Out                       =   0x0002,     // Param is [out]
    ParamFlags_Optional                  =   0x0010,     // Param is optional

    // Reserved flags for Runtime use only.
    ParamFlags_ReservedMask              =   0xf000,
    ParamFlags_HasDefault                =   0x1000,     // Param has default value.
    ParamFlags_HasFieldMarshal           =   0x2000,     // Param has FieldMarshal.

    ParamFlags_Unused                    =   0xcfe0
};
typedef uint16 ParamFlags_t; // ParamAttributes

enum _AssemblyFlags
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
};
typedef uint32 AssemblyFlags;

enum _FileFlags
{
    FileFlags_ContainsMetaData      =   0x0000,     // This is not a resource file
    FileFlags_ContainsNoMetaData    =   0x0001,     // This is a resource file or other non-metadata-containing file
};
typedef uint32 FileFlags;

#if 0 // todo

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

typedef enum CorManifestResourceFlags
{
    mrVisibilityMask        =   0x0007,
    mrPublic                =   0x0001,     // The Resource is exported from the Assembly.
    mrPrivate               =   0x0002,     // The Resource is private to the Assembly.
} CorManifestResourceFlags;

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

Offset

Size

Field

Description

0 12 (bits) Flags Flags (CorILMethod_FatFormat shall be set in bits 0:1).
12 (bits) 4 (bits) Size Size of this header expressed as the count of 4-byte integers occupied (currently 3).

2 2 MaxStack Maximum number of items on the operand stack.
4 4 CodeSize Size in bytes of the actual method body
8 4 LocalVarSigTok Meta Data token for a signature describing the layout of the local variables for the method.  0 means there are no local variables present. This field references a stand-alone signature in the MetaData tables, which references an entry in the #Blob stream.

The available flags are:

Flag Value Description CorILMethod_FatFormat

0x3 Method header is fat.
CorILMethod_TinyFormat

0x2

Method header is tiny.

CorILMethod_MoreSects

0x8

More sections follow after this header.

CorILMethod_InitLocals

0x10

Call default constructor on all local variables.

This means that when the CorILMethod_MoreSects is set, extra sections follow the method. To reach the first extra section we have to add the size of the header to the code size and to the file offset of the method, then aligne to the next 4-byte boundary.

Extra sections can have a Fat (1 byte flags, 3 bytes size) or a Small header (1 byte flags, 1 byte size); the size includes the header size. The type of header and the type of section is specified in the first byte, of course:

Flag

Value

Description

CorILMethod_Sect_EHTable

0x1

Exception handling data.

CorILMethod_Sect_OptILTable

0x2

Reserved, shall be 0.

CorILMethod_Sect_FatFormat

0x40

Data format is of the fat variety, meaning there is a 3-byte length.  If not set, the header is small with a  1-byte length

CorILMethod_Sect_MoreSects

0x80

Another data section occurs after this current section

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

const uint8 COR_ILEXCEPTION_CLAUSE_EXCEPTION = 0; // A typed exception clause
const uint8 COR_ILEXCEPTION_CLAUSE_FILTER = 1; // An exception filter and handler clause
const uint8 COR_ILEXCEPTION_CLAUSE_FINALLY = 2; // A finally clause
const uint8 COR_ILEXCEPTION_CLAUSE_FAULT = 4; // Fault clause (finally that is called on exception only)

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

struct loaded_image_t;

struct MetadataTypeFunctions_t
{
    // Virtual functions, but allowing for static construction.
    void (*decode)(const MetadataType_t*, void*);
    uint (*size)(const MetadataType_t*, loaded_image_t*);
    void (*to_string)(const MetadataType_t*, void*);
};

struct MetadataType_t
{
    const char *name;
    MetadataTypeFunctions_t const * functions;
    union {
        int8 fixed_size;
        int8 table_index;
        CodedIndex coded_index;
   };
};

uint
loadedimage_metadata_size_codedindex_get (loaded_image_t* image, CodedIndex coded_index);

#define CODED_INDEX(x, n, values) uint metadata_size_codedindex_ ## x (MetadataType_t* type, loaded_image_t* image) \
{ return loadedimage_metadata_size_codedindex_get (image, type->coded_index); }
CODED_INDICES
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
metadata_size_fixed (const MetadataType_t* type, loaded_image_t* image)
{
    return (uint)type->fixed_size; // TODO? change from int8 to uint8? or uint to int?
}

uint
metadata_size_blob (const MetadataType_t* type, loaded_image_t* image);

uint
metadata_size_string (const MetadataType_t* type, loaded_image_t* image);

uint
metadata_size_ustring (const MetadataType_t* type, loaded_image_t* image)
{
    return 4;
}

uint
metadata_size_codedindex (const MetadataType_t* type, loaded_image_t* image)
{
    return 0;
}

uint
image_metadata_size_index (loaded_image_t* image, uint /* todo enum */ table_index);

uint
metadata_size_index (const MetadataType_t* type, loaded_image_t* image)
{
    return image_metadata_size_index (image, (uint)type->table_index);
}

uint
metadata_size_index_list (const MetadataType_t* type, loaded_image_t* image)
{
    return metadata_size_index (type, image);
}

uint
metadata_size_guid (const MetadataType_t* type, loaded_image_t* image);

const MetadataTypeFunctions_t MetadataType_Fixed =
{
    MetadataDecode_fixed,
    metadata_size_fixed,
};

const MetadataTypeFunctions_t MetadataType_blob_functions =
{
    MetadataDecode_blob,
    metadata_size_blob,
};

const MetadataTypeFunctions_t MetadataType_string_functions =
{
    MetadataDecode_string,
    metadata_size_string,
};

const MetadataTypeFunctions_t MetadataType_guid_functions =
{
    MetadataDecode_guid,
    metadata_size_guid,
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
};

const MetadataTypeFunctions_t MetadataType_IndexList =
{
    MetadataDecode_index_list,
    metadata_size_index, // TODO?
};

const MetadataTypeFunctions_t MetadataType_CodedIndex =
{
    MetadataDecode_codedindex,
    metadata_size_codedindex,
};

const MetadataType_t MetadataType_int8 = { "int8", &MetadataType_Fixed, {1} };
const MetadataType_t MetadataType_int16 = { "int16", &MetadataType_Fixed, {2} };
const MetadataType_t MetadataType_int32 = { "int32", &MetadataType_Fixed, {4} };
const MetadataType_t MetadataType_int64 = { "int64", &MetadataType_Fixed, {8} };
const MetadataType_t MetadataType_uint8 = { "uint8", &MetadataType_Fixed, {1} };
const MetadataType_t MetadataType_uint16 = { "uint16", &MetadataType_Fixed, {2} };
const MetadataType_t MetadataType_uint32 = { "uint32", &MetadataType_Fixed, {4} };
const MetadataType_t MetadataType_uint64 = { "uint64", &MetadataType_Fixed, {8} };
const MetadataType_t MetadataType_ResolutionScope = { "ResolutionScope", &MetadataType_CodedIndex, {(int8)CodedIndex(ResolutionScope)} };
// heap indices or offsets
const MetadataType_t MetadataType_string = { "string", &MetadataType_string_functions };
const MetadataType_t MetadataType_guid = { "guid", &MetadataType_Fixed, {16} };
const MetadataType_t MetadataType_blob = { "blob",  &MetadataType_blob_functions };
// table indices
const MetadataType_t MetadataType_Field = { "FieldList", &MetadataType_Index, {Field} };
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

struct metadata_schema_column_t
{
    const char* name;
    const MetadataType_t& type;
};

struct metadata_table_schema_t
{
    const char *name;
    uint8 count;
    const metadata_schema_column_t* fields;
    void (*unpack)();
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
    METADATA_COLUMN2 (Flags, TypeFlags)                                 \
    METADATA_COLUMN (TypeName)                                          \
    METADATA_COLUMN (TypeNameSpace)                                     \
    METADATA_COLUMN (Extends)                                           \
    METADATA_COLUMN (FieldList)                                         \
    METADATA_COLUMN (MethodList))                                       \
                                                                        \
/*table0x03*/ METADATA_TABLE (Table3, NOTHING,                          \
    METADATA_COLUMN (Unused))                                           \
                                                                        \
/*table0x04*/ METADATA_TABLE (Field, : Member_t,                        \
    METADATA_COLUMN2 (Flags, FieldFlags)                                \
    METADATA_COLUMN (Name)                                              \
    METADATA_COLUMN (Signature))                                        \
                                                                        \
/*table0x05*/METADATA_TABLE (Table5 /*MethodPtr nonstandard*/, NOTHING, \
    METADATA_COLUMN (Unused))                                           \
                                                                        \
/*table0x06*/METADATA_TABLE (MethodDef, NOTHING,                        \
    METADATA_COLUMN (RVA)                                               \
    METADATA_COLUMN2 (ImplFlags, MethodDefImplFlags) /* TODO higher level support */     \
    METADATA_COLUMN2 (Flags, MethodDefFlags) /* TODO higher level support */             \
    METADATA_COLUMN (Name)                          \
    METADATA_COLUMN (Signature)      /* Blob heap, 7 bit encode/decode */         \
    METADATA_COLUMN (ParamList)) /* Param table, start, until table end, or start of next MethodDef; index into Param table, 2 or 4 bytes */ \
                                                                        \
/*table0x07*/METADATA_TABLE (Table7 /*ParamPtr nonstandard*/, NOTHING,  \
    METADATA_COLUMN (Unused))                                           \
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
    )/* TODO METADATA_COLUMN (Field))*/                                 \
                                                                        \
/*table0x11*/ METADATA_TABLE (StandaloneSig, NOTHING,                   \
    METADATA_COLUMN2 (Signature, blob))                                 \
                                                                        \
/*table0x12*/ METADATA_TABLE (EventMap, NOTHING,                        \
    METADATA_COLUMN2 (Parent, TypeDef)                                  \
    METADATA_COLUMN (EventList))                                        \
                                                                        \
/*table0x13*/ METADATA_TABLE (Table13, NOTHING, METADATA_COLUMN (Unused)) \
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
/*table0x16*/ METADATA_TABLE (Table16, NOTHING, METADATA_COLUMN (Unused))  \
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
/*table0x1E*/ METADATA_TABLE (Table1E, NOTHING, METADATA_COLUMN (Unused))       \
                                                                                \
/*table0x1F*/ METADATA_TABLE (Table1F, NOTHING, METADATA_COLUMN (Unused))       \
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
    METADATA_COLUMN2 (Processor , uint32))                                      \
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
    METADATA_COLUMN2 (Processor , uint32)                                       \
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
    METADATA_COLUMN3 (Flags, uint32, FileFlags)                                 \
    METADATA_COLUMN2 (Name, string)                                             \
    METADATA_COLUMN2 (HashValue, blob))                                         \

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
#define METADATA_TABLE(name, base, columns)  struct name ##  _t base { columns };
#define METADATA_COLUMN2(name, type) metadata_schema_TYPED_ ## type name;
#define METADATA_COLUMN3(name, persistant_type, pointerful_type) pointerful_type name;
METADATA_TABLES

#undef METADATA_TABLE
#undef METADATA_COLUMN2
#undef METADATA_COLUMN3
#define METADATA_TABLE(name, base, columns) \
const metadata_schema_column_t metadata_column_ ## name [ ] = { columns }; \
const metadata_table_schema_t metadata_row_schema_ ## name = { #name, CountOf (metadata_column_ ## name), metadata_column_ ## name };
#define METADATA_COLUMN2(name, type) { # name, MetadataType_  ## type },
#define METADATA_COLUMN3(name, persistant_type, pointerful_type) { # name, MetadataType_  ## persistant_type },
METADATA_TABLES

const metadata_schema_column_t metadata_columns_MethodSpec [ ] = // table0x2B
{
    { "Method", MetadataType_MethodDefOrRef },
    { "Instantiation", MetadataType_blob },
};
const metadata_table_schema_t metadata_row_schema_MethodSpec = { "MethodSpec", CountOf (metadata_columns_MethodSpec), metadata_columns_MethodSpec };

struct NestedClass_t // table0x29
{
    Type_t* NestedClass;
    Type_t* EnclosingClass;
};
struct metadata_NestedClass_t // table0x29
{
    MetadataToken_t NestedClass;
    MetadataToken_t EnclosingClass;
};
const metadata_schema_column_t metadata_columns_NestedClass [ ] = // table0x29
{
    { "NestedClass", MetadataType_TypeDef },
    { "EnclosingClass", MetadataType_TypeDef },
};
const metadata_table_schema_t metadata_row_schema_NestedClass = { "NestedClass", CountOf (metadata_columns_NestedClass), metadata_columns_NestedClass };

struct ExportedType_t // table0x27
{
};
struct metadata_ExportedType_t // table0x27
{
    TypeFlags_t Flags;
    uint32 TypeDefId; // index into TypeDef table of another module in this assembly; hint only
    MetadataString_t TypeName;
    MetadataString_t TypeNameSpace;
    MetadataToken_t Implementation; // coded index Implementation
};
const metadata_schema_column_t metadata_columns_ExportedType [ ] = // table0x27
{
    { "Flags", MetadataType_uint32 },
    { "TypeDefId", MetadataType_uint32 },
    { "TypeName", MetadataType_string },
    { "TypeNameSpace", MetadataType_string },
    { "Implementation", MetadataType_Implementation },
};
const metadata_table_schema_t metadata_row_schema_ExportedType = { "ExportedType", CountOf (metadata_columns_ExportedType), metadata_columns_ExportedType };

struct MarshalSpec_t
{
    // TODO
};

struct GenericParam_t // table0x2A
{
    uint16 Number;
    uint16 Flags; // TODO enum
    union {
        //Type_t* Type;
        Method_t* Method;
    } Owner;
    String_t Name;
};
struct metadata_GenericParam_t // table0x2A
{
    uint16 Number;
    uint16 Flags; // TODO enum
    MetadataToken_t Owner;
    MetadataString_t Name;
};
const metadata_schema_column_t metadata_columns_GenericParam [ ] = // table0x2A
{
    { "Number", MetadataType_uint16 },
    { "Flags", MetadataType_uint16 },
    { "Owner", MetadataType_TypeOrMethodDef },
    { "Name", MetadataType_string },
};
const metadata_table_schema_t metadata_row_schema_GenericParam = { "GenericParam", CountOf (metadata_columns_GenericParam), metadata_columns_GenericParam };

struct GenericParamConstraint_t // table0x2C
{
    GenericParam_t* Owner;
    //TODO Type_t* Constraint;
};
struct metadata_GenericParamConstraint_t // table0x2C
{
    MetadataToken_t Owner;
    MetadataToken_t Constraint;
};
const metadata_schema_column_t metadata_columns_GenericParamConstraint [ ] = // table0x2C
{
    { "Owner", MetadataType_GenericParam },
    { "Constraint", MetadataType_TypeDefOrRef },
};
const metadata_table_schema_t metadata_row_schema_GenericParamConstraint = { "GenericParamConstraint", CountOf (metadata_columns_GenericParamConstraint), metadata_columns_GenericParamConstraint };

// TODO enum
typedef uint32 ManifestResourceFlags_t;

struct ManifestResource_t // table0x28
{
    uint32 Offset;
    ManifestResourceFlags_t Flags;
    String_t Name;
    union {
        //File_t* File;
        //Assembly_t* Assembly;
    } Implementation;
};
struct metadata_ManifestResource_t // table0x28
{
    uint32 Offset;
    ManifestResourceFlags_t Flags; // TODO enum
    MetadataString_t Name;
    MetadataToken_t Implementation;
};
const metadata_schema_column_t metadata_columns_ManifestResource [ ] = // table0x28
{
     { "Offset", MetadataType_uint32 },
     { "Flags", MetadataType_uint32 },
     { "Name", MetadataType_string },
     { "Implementation", MetadataType_Implementation },
};
const metadata_table_schema_t metadata_row_schema_ManifestResource = { "ManifestResource", CountOf (metadata_columns_ManifestResource), metadata_columns_ManifestResource };

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


struct DynamicTableColumn_t;
struct DynamicTableColumnFunctions_t;

struct DynamicTableColumnFunctions_t
{
};

struct DynamicTableColumn_t
{
    DynamicTableColumn_t()
    {
        memset (this, 0, sizeof (*this));
    }

    uint8 size;
    uint8 offset;
};

struct DynamicTableInfoElement_t
{
    //DynamicTableInfoElement_t() { memset (this, 0, sizeof (*this)); }

    uint32 RowCount;
    uint32 RowSize;
    void* Base;
    bool Present;
    uint32 ColumnCount;
    DynamicTableColumn_t* ColumnInfo;
};

union DynamicTableInfo_t
{
    DynamicTableInfo_t() { memset (this, 0, sizeof (*this)); }

    DynamicTableInfoElement_t array [
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, columns) 1+
METADATA_TABLES
        0];
    struct
    {
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, columns) DynamicTableInfoElement_t name;
METADATA_TABLES
    } name;
};

// TODO expand from METADATA_TABLES, i.e. for size and order and names
const char * GetTableName (uint a)
{
static const char * const table_names [ ] =
{
"Module",
"TypeRef",
"TypeDef",
"Table3",
"Field",
"Table5",
"MethodDef",
"Table7",
"Param",
"InterfaceImpl",
"MemberRef",
"Constant",
"CustomAttribute",
"FieldMarshal",
"DeclSecurity",
"ClassLayout",
"FieldLayout",
"StandAloneSig",
"EventMap",
"Table19",
"Event",
"PropertyMap",
"Table22",
"Property",
"MethodSemantics",
"MethodImpl",
"ModuleRef",
"TypeSpec",
"ImplMap",
"FieldRVA",
"Table30",
"Table31",
"Assembly",
"AssemblyProcessor",
"AssemblyOS",
"AssemblyRef",
"AssemblyRefProcessor",
"AssemblyRefOS",
"File",
"ExportedType",
"ManifestResource",
"NestedClass",
"GenericParam",
"MethodSpec",
"GenericParamConstraint",
};
    if (a < CountOf (table_names))
        return table_names [a];
    return "unknown";
}

struct metadata_table_t
{
    metadata_table_t() { memset (this, 0, sizeof (*this)); }

    void * base;
    uint index;
    uint row_size;
};

static const metadata_table_schema_t *  const metadata_int_to_table_schema [ ] =
{
    // TODO
    &metadata_row_schema_Module,
    &metadata_row_schema_TypeRef,
    &metadata_row_schema_TypeDef,
};

struct loaded_image_t_z // zeroed loaded_image_t
{
    loaded_image_t_z() { memset (this, 0, sizeof (*this)); }

    DynamicTableInfo_t table_info;
    bool table_present [64]; // index by metadata table, values are 0/false and 1/true
    uint row_size [64]; // index by metadata table
    uint8 coded_index_size [64]; // 2 or 4
    uint8 index_size [64]; // 2 or 4
    uint64 file_size;
    void * base;
    image_dos_header_t* dos;
    uint32 pe_offset;
    uchar* pe;
    image_nt_headers_t *nt;
    image_optional_header32_t *opt32;
    image_optional_header64_t *opt64;
    uint32 opt_magic;
    uint number_of_sections;
    char * strings;
    char * guids;
    uint string_index_size;
    uint guid_index_size;
    MetadataTablesHeader_t * metadata_tables;
    uint32 * number_of_rows; // points into metadata_tables_header
    uint NumberOfRvaAndSizes;
    uint number_of_streams;
    uint blob_size; // 2 or 4
    uint string_size; // 2 or 4
    uint ustring_size; // 2 or 4
    uint guid_size; // 2 or 4
    struct
    {
        MetadataStreamHeader_t* tables;
        MetadataStreamHeader_t* guid;
        MetadataStreamHeader_t* string; // utf8
        MetadataStreamHeader_t* ustring; // unicode/user strings
        MetadataStreamHeader_t* blob;
    } streams;
};

struct loaded_image_t : loaded_image_t_z
{
    uint compute_row_size (const metadata_table_schema_t* schema)
    {
        uint count = schema->count;
        uint size = 0;
        for (uint i = 0; i < count; ++i)
                size += schema->fields [i].type.functions->size (&schema->fields [i].type, this);
        return size;
    }

    uint get_row_size (uint a)
    {
        uint b = row_size [a];
        if (b)
                return b;
        b = compute_row_size (metadata_int_to_table_schema [a]);
        row_size [a] = b;
        return b;
    }

    void dump_table (uint a)
    {
        std::string prefix = string_format ("table %X (%s)", a, GetTableName (a));
        printf ("%s\n", prefix.c_str ());
        printf ("%s present:%u\n", prefix.c_str (), table_present [a]);
        printf ("%s row_size:%u\n", prefix.c_str (), get_row_size (a));
    }

    memory_mapped_file_t mmf;
    std::vector<image_section_header_t*> section_headers;

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
        printf ("pe: %c%c\\%d\\%d\n", pe [0], pe [1], pe [2], pe [3]);
        nt = (image_nt_headers_t*)pe;
        printf ("Machine:%X\n", nt->FileHeader.Machine);
        printf ("NumberOfSections:%X\n", nt->FileHeader.NumberOfSections);
        printf ("TimeDateStamp:%X\n", nt->FileHeader.TimeDateStamp);
        printf ("PointerToSymbolTable:%X\n", nt->FileHeader.PointerToSymbolTable);
        printf ("NumberOfSymbols:%X\n", nt->FileHeader.NumberOfSymbols);
        printf ("SizeOfOptionalHeader:%X\n", nt->FileHeader.SizeOfOptionalHeader);
        printf ("Characteristics:%X\n", nt->FileHeader.Characteristics);
        opt32 = (image_optional_header32_t*)(&nt->OptionalHeader);
        opt64 = (image_optional_header64_t*)(&nt->OptionalHeader);
        opt_magic = opt32->Magic;
        release_assertf ((opt_magic == 0x10b && !(opt64 = 0)) || (opt_magic == 0x20b && !(opt32 = 0)), ("file:%s opt_magic:%x", file_name, opt_magic));
        printf ("opt.magic:%x opt32:%p opt64:%p\n", opt_magic, (void*)opt32, (void*)opt64);
        NumberOfRvaAndSizes = opt32 ? opt32->NumberOfRvaAndSizes : opt64->NumberOfRvaAndSizes;
        printf ("opt.rvas:%X\n", NumberOfRvaAndSizes);
        number_of_sections = nt->FileHeader.NumberOfSections;
        printf ("number_of_sections:%X\n", number_of_sections);
        image_section_header_t* section_header = nt->first_section_header ();
        for (uint i = 0; i < number_of_sections; ++i, ++section_header)
            printf ("section [%02X].Name: %.8s\n", i, section_header->Name);
        image_data_directory_t* DataDirectory = opt32 ? opt32->DataDirectory : opt64->DataDirectory;
        for (uint i = 0; i < NumberOfRvaAndSizes; ++i)
        {
            printf ("DataDirectory [%02X].Offset: 0x%06X\n", i, DataDirectory[i].VirtualAddress);
            printf ("DataDirectory [%02X].Size: 0x%06X\n", i, DataDirectory[i].Size);
        }
        release_assertf (DataDirectory [14].VirtualAddress, ("Not a .NET image? %x", DataDirectory [14].VirtualAddress));
        release_assertf (DataDirectory [14].Size, ("Not a .NET image? %x", DataDirectory [14].Size));
        image_clr_header_t* clr = rva_to_p<image_clr_header_t>(DataDirectory [14].VirtualAddress);
        printf ("clr.cb:%X\n", clr->cb);
        printf ("clr.MajorRuntimeVersion:%X\n", clr->MajorRuntimeVersion);
        printf ("clr.MinorRuntimeVersion:%X\n", clr->MinorRuntimeVersion);
        printf ("clr.MetaData.Offset:%X\n", clr->MetaData.VirtualAddress);
        printf ("clr.MetaData.Size:%X\n", clr->MetaData.Size);
        release_assertf (clr->MetaData.Size, ("%X", clr->MetaData.Size));
        release_assertf (clr->cb >= sizeof (image_clr_header_t), ("%X %X", clr->cb, (uint)sizeof (image_clr_header_t)));
        MetadataRoot_t* metadata_root = rva_to_p<MetadataRoot_t>(clr->MetaData.VirtualAddress);
        printf ("metadata_root.Signature:%X\n", metadata_root->Signature);
        printf ("metadata_root.MajorVersion:%X\n", metadata_root->MajorVersion);
        printf ("metadata_root.MinorVersion:%X\n", metadata_root->MinorVersion);
        printf ("metadata_root.Reserved:%X\n", metadata_root->Reserved);
        printf ("metadata_root.VersionLength:%X\n", metadata_root->VersionLength);
        release_assertf ((metadata_root->VersionLength % 4) == 0, ("%X", metadata_root->VersionLength));
        size_t VersionLength = strlen(metadata_root->Version);
        release_assertf (VersionLength < metadata_root->VersionLength, ("%X %X", VersionLength, metadata_root->VersionLength));
        // TODO bounds checks throughout
        uint16* pflags = (uint16*)&metadata_root->Version[metadata_root->VersionLength];
        uint16* pnumber_of_streams = 1 + pflags;
        number_of_streams = *pnumber_of_streams;
        printf ("metadata_root.Version:%s\n", metadata_root->Version);
        printf ("flags:%X\n", *pflags);
        printf ("number_of_streams:%X\n", number_of_streams);
        MetadataStreamHeader_t* stream = (MetadataStreamHeader_t*)(pnumber_of_streams + 1);
        for (uint i = 0; i < number_of_streams; ++i)
        {
            printf ("stream[%X].Offset:%X\n", i, stream->Offset);
            printf ("stream[%X].Size:%X\n", i, stream->Size);
            release_assertf ((stream->Size % 4) == 0, ("%X", stream->Size));
            const char* name = stream->Name;
            size_t length = strlen (name);
            release_assertf (length <= 32, ("%X:%s", length, name));
            printf ("stream[%X].Name:%X:%.*s\n", i, (int)length, (int)length, name);
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
            stream = (MetadataStreamHeader_t*)(stream->Name + length);
        }
        metadata_tables = (MetadataTablesHeader_t*)(streams.tables->Offset + (char*)metadata_root);
        printf ("metadata_tables.reserved:%X\n", metadata_tables->reserved);
        printf ("metadata_tables.MajorVersion:%X\n", metadata_tables->MajorVersion);
        printf ("metadata_tables.MinorVersion:%X\n", metadata_tables->MinorVersion);
        printf ("metadata_tables.HeapSizes:%X\n", metadata_tables->HeapSizes);
        uint const HeapSize = metadata_tables->HeapSizes;
        string_size = (HeapSize & HeapOffsetSize_String) ? 4u : 2u;
        guid_size = (HeapSize & HeapOffsetSize_Guid) ? 4u: 2u;
        blob_size = (HeapSize & HeapOffsetSize_Blob) ? 4u : 2u;
        printf ("metadata_tables.reserved2:%X\n", metadata_tables->reserved2);
        uint64 valid = metadata_tables->Valid;
        uint64 sorted = metadata_tables->Sorted;
        uint64 unsorted = valid & ~sorted;
        uint64 invalidSorted = sorted & ~valid;
        printf ("metadata_tables.        Valid:%08X`%08X\n", (uint32)(valid >> 32), (uint32)valid);
        // Mono does not use sorted, and there are bits set beyond Valid.
        // I suspect this has to do with writing/appending, and occasionally sorting.
        printf ("metadata_tables.       Sorted:%08X`%08X\n", (uint32)(sorted >> 32), (uint32)sorted);
        printf ("metadata_tables.     Unsorted:%08X`%08X\n", (uint32)(unsorted >> 32), (uint32)unsorted);
        printf ("metadata_tables.InvalidSorted:%08X`%08X\n", (uint32)(invalidSorted >> 32), (uint32)invalidSorted);
        const uint64 one = 1;
        uint j = 0;
        uint32* RowCount = (uint32*)(metadata_tables + 1);
        for (uint i = 0; i < CountOf (table_info.array); ++i)
        {
            uint64 const mask = one << i;
            bool Present = (valid & mask) != 0;
            table_info.array [i].Present = Present;
            if (Present)
            {
                printf ("table 0x%X (%s) has %u rows (%s)\n", i, GetTableName (i), *RowCount, (sorted & mask) ? "sorted" : "unsorted");
                table_info.array [i].RowCount = *RowCount;
                ++j;
                ++RowCount;
            }
            else
            {
                printf ("table 0x%X (%s) is absent\n", i, GetTableName (i));
            }
        }
        dump_table (TypeDef);
    }

    template <typename T> T* rva_to_p (uint32 rva)
    {
        rva = rva_to_file_offset (rva);
        return rva ? (T*)(((char*)base) + rva) : 0;
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

uint
metadata_size_blob (const MetadataType_t* type, loaded_image_t* image)
{
    return image->blob_size;
}

uint
metadata_size_string (const MetadataType_t* type, loaded_image_t* image)
{
    return image->string_size;
}

uint
metadata_size_guid (const MetadataType_t* type, loaded_image_t* image)
{
    return image->blob_size;
}

uint
metadata_size_codedindex_compute (loaded_image_t* image, CodedIndex coded_index)
{
    const CodedIndex_t* data = &CodedIndices.array [(uint)coded_index];
    uint32 max_rows = 0;
    uint const map = data->map;
    uint const count = data->count;
    for (uint i = 0; i < count; ++i)
        max_rows = std::max (max_rows, image->table_info.array[((uint8*)&CodedIndexMap) [map + i]].RowCount);
    return (max_rows <= (0xFFFFu >> data->tag_size)) ? 2u : 4u;
}

uint
loadedimage_metadata_size_codedindex_get (loaded_image_t* image, CodedIndex coded_index)
{
    const uint a = image->coded_index_size [(uint)coded_index];
    if (a)
        return a;
    return metadata_size_codedindex_compute (image, coded_index);
}

uint
metadata_size_index_compute (loaded_image_t* image, uint /* todo enum */ table_index)
{
    uint row_count = image->table_info.array [table_index].RowCount;
    return (row_count <= 0xFFFF) ? 2u : 4u;
}

uint
image_metadata_size_index (loaded_image_t* image, uint /* todo enum */ table_index)
{
    const uint a = image->row_size [(uint)table_index];
    if (a)
        return a;
    return metadata_size_index_compute (image, table_index);
}

}

using namespace m2;

int
main (int argc, char** argv)
{
    loaded_image_t im;
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
        fprintf (stderr, "error %d\n", er);
    }
    catch (const std::string& er)
    {
        fprintf (stderr, "%s", er.c_str());
    }
    return 0;
}
