// 2-clause BSD license unless that does not suffice
// else MIT like mono. Need to research the difference.

// Implementation language is C++. At least C++11.
// The following features of C++ are desirable:
//   RAII (destructors, C++98)
//   enum class (C++11)
//   std::size (C++17)
//   std::string::data (direct sprintf into std::string) (C++17)
//   non-static member initialization (C++11)
//   thread safe static initializers, maybe (C++11)
//   char16_t (C++11, but could use C++98 unsigned short)
//   explicit operator bool (C++11 but easy to emulate in C++98)
//   variadic template (C++11, really needed?)
//   variadic macros (really needed?)
//   std::vector
//   std::string
//   Probably more of STL.
//
// C++ library dependencies are likely to be removed, but we'll see.

// Goals: clarity, simplicity, portability, size, interpreter, compile to C++, and maybe
// later some JIT

#if _MSC_VER && _MSC_VER <= 1500
#error This version of Visual C++ is too old. Known bad versions include 5.0 and 2008. Known good includes 1900/2017.
#endif

#define _CRT_SECURE_NO_WARNINGS 1

//#include "config.h"
#define _DARWIN_USE_64_BIT_INODE 1
//#define __DARWIN_ONLY_64_BIT_INO_T 1
// TODO cmake
// TODO big endian and packed I/O
//#define _LARGEFILE_SOURCE
//#define _LARGEFILE64_SOURCE

#ifndef HAS_TYPED_ENUM
#if 1 // __cplusplus >= 201103L || _MSC_VER >= 1500 // TODO test more compilers
#define HAS_TYPED_ENUM 1
#else
#define HAS_TYPED_ENUM 0
#endif
#endif

#if _MSC_VER
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4619) // invalid pragma warning disable
#pragma warning(disable:4100) // unused parameter
#pragma warning(disable:4505) // unused static function
#pragma warning(disable:4514) // unused function
#pragma warning(disable:4706) // assignment within conditional
#pragma warning(disable:4820) // padding
#pragma warning(push)
#pragma warning(disable:4710) // function not inlined
#pragma warning(disable:4626) // assignment implicitly deleted
#pragma warning(disable:5027) // move assignment implicitly deleted
#pragma warning(disable:4571) // catch(...)
#pragma warning(disable:4625) // copy constructor implicitly deleted
#pragma warning(disable:4668) // //#if not_defined is //#if 0
#pragma warning(disable:4774) // printf used without constant format
#pragma warning(disable:5026) // move constructor implicitly deleted
#pragma warning(disable:5039) // exception handling and function pointers
#endif

#if __GNUC__ || __clang__
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <memory.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <utility>
#include <memory>
using std::basic_string;
using std::string;
using std::vector;
#include <algorithm>
#if _WIN32
#define NOMINMAX 1
#include <io.h>
#include <windows.h>
#else
#define IsDebuggerPresent() (0)
#define __debugbreak() ((void)0)
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#endif
#if _MSC_VER
#include <malloc.h> // for _alloca
#pragma warning(pop)
#endif

using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using uint = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;

namespace w3
{

// Portable to old (and new) Visual C++ runtime.
uint
string_vformat_length (const char *format, va_list va)
{
#if !_MSC_VER
    return 2 + vsnprintf (0, 0, format, va);
#else
    // newer runtime: _vscprintf (format, va);
    // else loop until it fits, getting -1 while it does not.
    uint n = 0;
    for (;;)
    {
        uint inc = n ? n : 64;
        if (_vsnprintf ((char*)_alloca(inc), n += inc, format, va) != -1)
            return n + 2;
    }
#endif
}

string
StringFormatVa (const char *format, va_list va)
{
    // Some systems, including Linux/amd64, cannot consume a
    // va_list multiple times. It must be copied first.
    // Passing the parameter twice does not work.
#if !_WIN32
    va_list va2;
#ifdef __va_copy
    __va_copy (va2, va);
#else
    va_copy (va2, va); // C99
#endif
#endif
    vector<char> s((size_t)string_vformat_length(format, va));
#if _WIN32
    _vsnprintf (&s [0], s.size(), format, va);
#else
    vsnprintf (&s [0], s.size(), format, va2);
#endif
    return &s [0];
}

string
StringFormat (const char *format, ...)
{
    va_list va;
    va_start (va, format);
    string a = StringFormatVa (format, va);
    va_end (va);
    return a;
}

#define NotImplementedYed() (AssertFormat (0, ("not yet implemented %s 0x%08X ", __func__, __LINE__)))

void
ThrowString (const string& a)
{
    //fprintf (stderr, "%s\n", a.c_str());
    throw a + "\n";
    //abort ();
}

void
ThrowInt (int i, const char* a = "")
{
    ThrowString (StringFormat ("error 0x%08X %s", i, a));
}

void
ThrowErrno (const char* a = "")
{
    ThrowInt (errno, a);
}

#if _WIN32
void
throw_Win32Error (int err, const char* a = "")
{
    ThrowInt (err, a);

}
void
throw_GetLastError (const char* a = "")
{
    ThrowInt ((int)GetLastError (), a);

}
#endif

void
AssertFailedFormat (const char* condition, const string& extra)
{
    //fputs (("AssertFailedFormat:" + string (condition) + ":" + w3::StringFormatVa (format, args) + "\n").c_str (), stderr);
    //Assert (0);
    //abort ();
    if (IsDebuggerPresent ()) __debugbreak ();
    ThrowString ("AssertFailed:" + string (condition) + ":" + extra);
}

void
AssertFailed (const char * expr)
{
    fprintf (stderr, "AssertFailed:%s\n", expr);
    assert (0);
    abort ();
}

#define Assert(x)         ((x) || ( AssertFailed (#x), (int)0))
#define AssertFormat(x, extra) ((x) || (AssertFailedFormat (#x, StringFormat extra), 0))

static
uint
Unpack2 (const void *a)
{
    uint8* b = (uint8*)a;
    return ((b [1]) << 8) | (uint)b [0];
}

static
uint
Unpack4 (const void *a)
{
    return (Unpack2 ((char*)a + 2) << 16) | Unpack2 (a);
}

static
uint
Unpack (const void *a, uint size)
{
    switch (size)
    {
    case 2: return Unpack2 (a);
    case 4: return Unpack4 (a);
    }
    AssertFormat(size == 2 || size == 4, ("%X", size));
    return ~0u;
}

template <uint N> struct uintLEn_to_native_exact;
template <uint N> struct uintLEn_to_native_fast;

#if !_MSC_VER || _MSC_VER > 1000
template <> struct uintLEn_to_native_exact<16> { typedef uint16 T; };
template <> struct uintLEn_to_native_exact<32> { typedef uint T; };
template <> struct uintLEn_to_native_exact<64> { typedef uint64 T; };
template <> struct uintLEn_to_native_fast<16> { typedef uint T; };
template <> struct uintLEn_to_native_fast<32> { typedef uint T; };
template <> struct uintLEn_to_native_fast<64> { typedef uint64 T; };
#else
struct uintLEn_to_native_exact<16> { typedef uint16 T; };
struct uintLEn_to_native_exact<32> { typedef uint T; };
struct uintLEn_to_native_exact<64> { typedef uint64 T; };
struct uintLEn_to_native_fast<16> { typedef uint T; };
struct uintLEn_to_native_fast<32> { typedef uint T; };
struct uintLEn_to_native_fast<64> { typedef uint64 T; };
#endif

template <uint N>
struct uintLEn // unsigned little endian integer, size n bits
{
    union {
        typename uintLEn_to_native_exact<N>::T debug_n;
        unsigned char data [N / 8];
    };

    operator
#if !_MSC_VER || _MSC_VER > 1000
    typename
#endif
    uintLEn_to_native_fast<N>::T ()
    {
#if !_MSC_VER || _MSC_VER > 1000
        typename
#endif
        uintLEn_to_native_fast<N>::T a = 0;
        for (uint i = N / 8; i; )
            a = (a << 8) | data [--i];
        return a;
    }
    void operator=(uint);
};

typedef uintLEn<16> uintLE16;
typedef uintLEn<32> uintLE;
typedef uintLEn<64> uintLE64;

uint
Unpack (uintLE16& a)
{
    return (uint)a;
}

uint
Unpack (uintLE16* a)
{
    return (uint)*a;
}

uint
Unpack (uintLE& a)
{
    return (uint)a;
}

uint
Unpack (uintLE* a)
{
    return (uint)*a;
}

// C++98 workaround for what C++11 offers.
struct explicit_operator_bool
{
    typedef void (explicit_operator_bool::*T) () const;
    void True () const;
};

typedef void (explicit_operator_bool::*bool_type) () const;

#if _WIN32
struct Handle
{
    // TODO Handle vs. win32file_t, etc.

    uint64 get_file_size (const char * file_name = "")
    {
        DWORD hi = 0;
        DWORD lo = GetFileSize (h, &hi);
        if (lo == INVALID_FILE_SIZE)
        {
            DWORD err = GetLastError ();
            if (err != NO_ERROR)
                throw_Win32Error ((int)err, StringFormat ("GetFileSizeEx(%s)", file_name).c_str());
        }
        return (((uint64)hi) << 32) | lo;
    }

    void * h;

    Handle (void *a) : h (a) { }
    Handle () : h (0) { }

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

    Handle& operator= (void* a)
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

    ~Handle ()
    {
        if (valid ()) CloseHandle (h);
        h = 0;
    }
};
#endif

struct Fd
{
    int fd;

#ifndef _WIN32
    uint64 get_file_size (const char * file_name = "")
    {
#if __CYGWIN__
        struct stat st = { 0 }; // TODO test more systems
        if (fstat (fd, &st))
#else
        struct stat64 st = { 0 }; // TODO test more systems
        if (fstat64 (fd, &st))
#endif
            ThrowErrno (StringFormat ("fstat(%s)", file_name).c_str ());
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
#if _WIN32
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

    Fd (int a = -1) : fd (a) { }

    Fd& operator= (int a)
    {
        if (fd == a) return *this;
        cleanup ();
        fd = a;
        return *this;
    }

    ~Fd ()
    {
        cleanup ();
    }
};

struct MemoryMappedFile
{
// TODO allow for redirection to built-in data (i.e. filesystem emulation with builtin BCL)
// TODO allow for systems that must read, not mmap
    void * base;
    size_t size;
#if _WIN32
    Handle file;
#else
    Fd file;
#endif
    MemoryMappedFile () : base(0), size(0) { }

    ~MemoryMappedFile ()
    {
        if (!base)
            return;
#if _WIN32
        UnmapViewOfFile (base);
#else
        munmap (base, size);
#endif
        base = 0;
    }
    void read (const char* a)
    {
#if _WIN32
        file = CreateFileA (a, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (!file) throw_GetLastError (StringFormat ("CreateFileA(%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size(a);
        Handle h2 = CreateFileMappingW (file, 0, PAGE_READONLY, 0, 0, 0);
        if (!h2) throw_GetLastError (StringFormat ("CreateFileMapping(%s)", a).c_str ());
        base = MapViewOfFile (h2, FILE_MAP_READ, 0, 0, 0);
        if (!base)
            throw_GetLastError (StringFormat ("MapViewOfFile(%s)", a).c_str ());
#else
        file = open (a, O_RDONLY);
        if (!file) ThrowErrno (StringFormat ("open(%s)", a).c_str ());
        // FIXME check for size==0 and >4GB.
        size = (size_t)file.get_file_size(a);
        base = mmap (0, size, PROT_READ, MAP_PRIVATE, file, 0);
        if (base == MAP_FAILED)
            ThrowErrno (StringFormat ("mmap(%s)", a).c_str ());
#endif
    }
};

#if HAS_TYPED_ENUM
#define BEGIN_ENUM(name, type) enum name : type
#define END_ENUM(name, type) ;
#else
#define BEGIN_ENUM(name, type) enum _ ## name
#define END_ENUM(name, type) ; typedef type name;
#endif

#define NOTHING /* nothing */

static
uint64
SignExtend (uint64 value, uint bits)
{
    // Extract lower bits from value and signextend.
    // From detour_sign_extend.
    const uint left = 64 - bits;
    const uint64 m1 = (uint64)(int64)-1;
    const int64 wide = (int64)(value << left);
    const uint64 sign = (wide < 0) ? (m1 << left) : 0;
    return value | sign;
}

struct int_split_sign_magnitude_t
{
    int_split_sign_magnitude_t(int64 a)
    : neg((a < 0) ? 1u : 0u),
        u((a < 0) ? (1 + (uint64)-(a + 1)) // Avoid negating most negative number.
                  : (uint64)a) { }
    uint neg;
    uint64 u;
};

static
uint
UIntGetPrecision (uint64 a)
{
    // How many bits needed to represent.
    uint len = 1;
    while ((len <= 64) && (a >>= 1)) ++len;
    return len;
}

static
uint
IntGetPrecision (int64 a)
{
    // How many bits needed to represent.
    // i.e. so leading bit is extendible sign bit, or 64
    return std::min(64u, 1 + UIntGetPrecision (int_split_sign_magnitude_t(a).u));
}

static
uint
UIntToDec_GetLength (uint64 b)
{
    uint len = 0;
    do ++len;
    while (b /= 10);
    return len;
}

static
uint
UIntToDec (uint64 a, char* buf)
{
    uint const len = UIntToDec_GetLength(a);
    for (uint i = 0; i < len; ++i, a /= 10)
        buf [i] = "0123456789" [a % 10];
    return len;
}

static
uint
IntToDec (int64 a, char* buf)
{
    const int_split_sign_magnitude_t split(a);
    if (split.neg)
        *buf++ = '-';
    return split.neg + UIntToDec(split.u, buf);
}

static
uint
IntToDec_GetLength (int64 a)
{
    const int_split_sign_magnitude_t split(a);
    return split.neg + UIntToDec_GetLength(split.u);
}

static
uint
UIntToHex_GetLength (uint64 b)
{
    uint len = 0;
    do ++len;
    while (b >>= 4);
    return len;
}

static
uint
IntToHex_GetLength (int64 a)
{
    // If negative and first digit is <8, add one to induce leading 8-F
    // so that sign extension of most significant bit will work.
    // This might be a bad idea. TODO.
    uint64 b = (uint64)a;
    uint len = 0;
    uint64 most_significant;
    do ++len;
    while ((most_significant = b), b >>= 4);
    return len + (a < 0 && most_significant < 8);
}

static
void
UIntToHexLength (uint64 a, uint len, char *buf)
{
    buf += len;
    for (uint i = 0; i < len; ++i, a >>= 4)
        *--buf = "0123456789ABCDEF" [a & 0xF];
}

static
void
IntToHexLength (int64 a, uint len, char *buf)
{
    UIntToHexLength((uint64)a, len, buf);
}

static
uint
IntToHex (int64 a, char *buf)
{
    uint const len = IntToHex_GetLength (a);
    IntToHexLength(a, len, buf);
    return len;
}

static
uint
IntToHex8 (int64 a, char *buf)
{
    IntToHexLength(a, 8, buf);
    return 8;
}

static
uint
IntToHex_GetLength_AtLeast8 (int64 a)
{
    uint const len = IntToHex_GetLength (a);
    return std::max(len, 8u);
}

static
uint
UIntToHex_GetLength_AtLeast8 (uint64 a)
{
    uint const len = UIntToHex_GetLength (a);
    return std::max (len, 8u);
}

static
uint
IntToHex_AtLeast8 (int64 a, char *buf)
{
    uint const len = IntToHex_GetLength_AtLeast8 (a);
    IntToHexLength (a, len, buf);
    return len;
}

static
uint
UIntToHex_AtLeast8 (uint64 a, char *buf)
{
    uint const len = UIntToHex_GetLength_AtLeast8 (a);
    UIntToHexLength (a, len, buf);
    return len;
}

struct stream
{
    virtual void write (const void* bytes, size_t count) = 0;
    void prints (const char* a) { write (a, strlen(a)); }
    void prints (const string& a) { prints(a.c_str()); }
    void printc (char a) { write (&a, 1); }
    void printf (const char* format, ...)
    {
        va_list va;
        va_start (va, format);
        printv (format, va);
        va_end (va);
    }

    void
    printv(const char *format, va_list va)
    {
        prints(StringFormatVa(format, va));
    }
};

struct stdout_stream : stream
{
    virtual void write (const void* bytes, size_t count)
    {
        fflush (stdout);
        const char* pc = (const char*)bytes;
        while (count > 0)
        {
            uint const n = (uint)std::min(count, ((size_t)1024) * 1024 * 1024);
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
        fflush (stderr);
        const char* pc = (const char*)bytes;
        while (count > 0)
        {
            uint const n = (uint)std::min(count, ((size_t)1024) * 1024 * 1024);
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

uint
read_byte (uint8*& cursor, const uint8* end)
{
    if (cursor >= end)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return *cursor++;
}

uint
read_varuint32 (uint8*& cursor, const uint8* end)
{
    uint result = 0;
    uint shift = 0;
    while (true)
    {
        const uint byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        if ((byte & 0x80) == 0)
            break;
        shift += 7;
    }
    return result;
}

uint
read_varuint7 (uint8*& cursor, const uint8* end)
{
    const uint result = read_byte (cursor, end);
    if (result & 0x80)
        ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    return result;
}

int
read_varint32 (uint8*& cursor, const uint8* end)
{
    int result = 0;
    uint shift = 0;
    uint size = 32;
    uint byte = 0;
    do
    {
        byte = read_byte (cursor, end);
        result |= (byte & 0x7F) << shift;
        shift += 7;
    } while ((byte & 0x80) == 0);

    // sign bit of byte is second high order bit (0x40)
    if ((shift < size) && (byte & 0x40))
        result |= (~0 << shift); // sign extend
    
    return result;
}

typedef enum ValueType
{
    ValueType_I32 = 0x7F,
    ValueType_I64 = 0x7E,
    ValueType_F32 = 0x7D,
    ValueType_F64 = 0x7C,
} ValueType;

typedef enum ResultType
{
    ResultType_I32 = 0x7F,
    ResultType_I64 = 0x7E,
    ResultType_F32 = 0x7D,
    ResultType_F64 = 0x7C,
    ResultType_Empty = 0x40
} ResultType;

typedef enum TableElementType
{
    TableElementType_FuncRef = 0x70,
} TableElementType;

typedef enum LimitsTag // specific to tabletype?
{
    LimitsTag_Min = 0,
    Limits_MinMax = 1,
} LimitsTag;

struct Limits
{
    uint min;
    uint max;
    bool hasMax;
};

const uint FunctionTypeTag = 0x60;

struct TableType
{
    TableElementType element_type;
    Limits limits;
};

// Table types have an element type, funcref
const uint TableTypeFuncRef = 0x70;

// Globals are mutable or constant.
typedef enum Mutable
{
    Mutable_Constant = 0, // aka false
    Mutable_Variable = 1, // aka true
} Mutable;

struct Module;
struct SectionBase;

// science project?
typedef enum InstructionEncoding
{
    Reserved        = 0x00,
    FixedSize1      = 0x01,
    FixedSize2      = 0x02,
    End             = 0x03,
    ValueType       = 0x04,     // read_varuint32
    TypeIndex       = 0x05,     // read_varuint32
    FunctionIndex   = 0x06,     // read_varuint32
    MemoryIndex     = 0x07,     // read_varuint32
    GlobalIndex     = 0x08,     // read_varuint32
    LocalIndex      = 0x09,,    // read_varuint32
    LabelIndex      = 0x0A,     // read_varuint32
    InstructionEncoding_Sequence        = 0x0B,     // 0xB
    VectorLabels    = 0x0C,
} InstructionEncoding;

typedef enum InstructionGroup {
    InstructionGroup_Reserved   = 0,
    InstructionGroup_Control    = 1,
    InstructionGroup_Parametric = 2,
    InstructionGroup_Numeric    = 3,
    InstructionGroup_Memory     = 4,
} InstructionGroup;

struct Instruction
{
    const char* name;
    void (*interp)(Module*);
    int size;
    bool block;
    bool loop;
    int u32;
};

struct Instruction
{
    uint8 b0;
    int encoding[4];
    InstructionGroup group; // useful?
    uint16 name;
    uint8 stack_in;
    int8 stack_adjust; // push is negative, pop is positive
    ResultType stack_in0;
    ResultType stack_in1;
    ResultType stack_out0;
};

#define INSTRUCTIONS \
INSTRUCTION (0x00, { FixedSize1 }, Control, Unreach) \
INSTRUCTION (0x01, { FixedSize1 }, Control, Nop) \
INSTRUCTION (0x02, { FixedSize1 }, Control, Block) \
INSTRUCTION (0x03, { FixedSize1 }, Control, Loop) \
INSTRUCTION (0x04, { FixedSize1 }, Control, If) \
INSTRUCTION (0x05, { FixedSize1 }, Control, Else) \
\
INSTRUCTION (0x06, { Reserved }, Reserved, Reserved06, 0) \
INSTRUCTION (0x07, { Reserved }, Reserved, Reserved07, 0) \
INSTRUCTION (0x08, { Reserved }, Reserved, Reserved08, 0) \
INSTRUCTION (0x09, { Reserved }, Reserved, Reserved09, 0) \
INSTRUCTION (0x0A, { Reserved }, Reserved, Reserved0A, 0) \
\
INSTRUCTION (0x0B, { FixedSize1 }, Control, BlockEnd) \
INSTRUCTION (0x0C, { FixedSize1 }, Control, Br) \
INSTRUCTION (0x0D, { FixedSize1 }, Control, BrIf) \
INSTRUCTION (0x0E, { FixedSize1 }, Control, BrTable) \
INSTRUCTION (0x0F, { FixedSize1 }, Control, Ret) \
INSTRUCTION (0x10, { FixedSize1 }, Control, Call) \
INSTRUCTION (0x11, { FixedSize1 }, Control, Calli) \
\
INSTRUCTION (0x12, 0, Reserved, Reserved12, 0) \
INSTRUCTION (0x13, 0, Reserved, Reserved13, 0) \
INSTRUCTION (0x14, 0, Reserved, Reserved14, 0) \
INSTRUCTION (0x15, 0, Reserved, Reserved15, 0) \
INSTRUCTION (0x16, 0, Reserved, Reserved16, 0) \
INSTRUCTION (0x17, 0, Reserved, Reserved17, 0) \
INSTRUCTION (0x18, 0, Reserved, Reserved18, 0) \
INSTRUCTION (0x19, 0, Reserved, Reserved19, 0) \
\
INSTRUCTION (0x1A, { FixedSize1 }, Parametric, Drop) \
INSTRUCTION (0x1B, { FixedSize1 }, Parametric, Select) \
\
INSTRUCTION (0x1C, 0, Reserved1C, 0) \
INSTRUCTION (0x1D, 0, Reserved1D, 0) \
INSTRUCTION (0x1E, 0, Reserved1E, 0) \
INSTRUCTION (0x1F, 0, Reserved1F, 0) \
\
INSTRUCTION (0x20, { FixedSize1 }, Variable, Local_get) \
INSTRUCTION (0x21, { FixedSize1 }, Variable, Local_set) \
INSTRUCTION (0x22, { FixedSize1 }, Variable, Local_tee) \
INSTRUCTION (0x23, { FixedSize1 }, Variable, Global_get) \
INSTRUCTION (0x24, { FixedSize1 }, Variable, Global_set) \
\
INSTRUCTION (0x25, 0, Reserved, Reserved25, 0) \
INSTRUCTION (0x26, 0, Reserved, Reserved26, 0) \
INSTRUCTION (0x27, 0, Reserved, Reserved27, 0) \
\
INSTRUCTION (0x28, { FixedSize1 }, Memory, Load_i32) \
INSTRUCTION (0x29, { FixedSize1 }, Memory, Load_i64) \
INSTRUCTION (0x2A, { FixedSize1 }, Memory, Load_f32) \
INSTRUCTION (0x2B, { FixedSize1 }, Memory, Load_f64) \
\
INSTRUCTION (0x2C, { FixedSize1 }, Memory, Load_i32_8s) \
INSTRUCTION (0x2D, { FixedSize1 }, Memory, Load_i32_8u) \
INSTRUCTION (0x2E, { FixedSize1 }, Memory, Load_i32_16s) \
INSTRUCTION (0x2F, { FixedSize1 }, Memory, Load_i32_16u) \
\
INSTRUCTION (0x30, { FixedSize1 }, Memory, Load_i64_8s) \
INSTRUCTION (0x31, { FixedSize1 }, Memory, Load_i64_8u) \
INSTRUCTION (0x32, { FixedSize1 }, Memory, Load_i64_16s) \
INSTRUCTION (0x33, { FixedSize1 }, Memory, Load_i64_16u) \
INSTRUCTION (0x34, { FixedSize1 }, Memory, Load_i64_32s) \
INSTRUCTION (0x35, { FixedSize1 }, Memory, Load_i64_32u) \
\
INSTRUCTION (0x36, { FixedSize1 }, Memory, Store_i32) \
INSTRUCTION (0x37, { FixedSize1 }, Memory, Store_i64) \
INSTRUCTION (0x38, { FixedSize1 }, Memory, Store_f32) \
INSTRUCTION (0x39, { FixedSize1 }, Memory, Store_f64) \
\
INSTRUCTION (0x3A, { FixedSize1 }, Memory, Store_i32_8) \
INSTRUCTION (0x3B, { FixedSize1 }, Memory, Store_i32_16) \
INSTRUCTION (0x3C, { FixedSize1 }, Memory, Store_i64_8) \
INSTRUCTION (0x3D, { FixedSize1 }, Memory, Store_i64_16) \
INSTRUCTION (0x3E, { FixedSize1 }, Memory, Store_i64_32) \
INSTRUCTION (0x3F, { FixedSize1 }, Memory, MemSize, 2) \
INSTRUCTION (0x40, { FixedSize1 }, Memory, MemGrow, 2) \
\
INSTRUCTION (0x41, { FixedSize1 }, Numeric, Const_i32, n32) \
INSTRUCTION (0x42, { FixedSize1 }, Numeric, Const_i64, n32) \
INSTRUCTION (0x43, { FixedSize1 }, Numeric, Const_f32, z) \
INSTRUCTION (0x44, { FixedSize1 }, Numeric, Const_f64, z) \
\
INSTRUCTION (0x45, { FixedSize1 }, Numeric, Eqz_i32) \
INSTRUCTION (0x46, { FixedSize1 }, Numeric, Eq_i32) \
INSTRUCTION (0x47, { FixedSize1 }, Numeric, Ne_i32) \
INSTRUCTION (0x48, { FixedSize1 }, Numeric, Lt_i32s) \
INSTRUCTION (0x49, { FixedSize1 }, Numeric, Lt_i32u) \
INSTRUCTION (0x4A, { FixedSize1 }, Numeric, Gt_i32s) \
INSTRUCTION (0x4B, { FixedSize1 }, Numeric, Gt_i32u) \
INSTRUCTION (0x4C, { FixedSize1 }, Numeric, Le_i32s) \
INSTRUCTION (0x4D, { FixedSize1 }, Numeric, Le_i32u) \
INSTRUCTION (0x4E, { FixedSize1 }, Numeric, Ge_i32s) \
INSTRUCTION (0x4F, { FixedSize1 }, Numeric, Ge_i32u) \
\
INSTRUCTION (0x50, { FixedSize1 }, Numeric, Eqz_i64) \
INSTRUCTION (0x51, { FixedSize1 }, Numeric, Eq_i64) \
INSTRUCTION (0x52, { FixedSize1 }, Numeric, Ne_i64) \
INSTRUCTION (0x53, { FixedSize1 }, Numeric, Lt_i64s) \
INSTRUCTION (0x54, { FixedSize1 }, Numeric, Lt_i64u) \
INSTRUCTION (0x55, { FixedSize1 }, Numeric, Gt_i64s) \
INSTRUCTION (0x56, { FixedSize1 }, Numeric, Gt_i64u) \
INSTRUCTION (0x57, { FixedSize1 }, Numeric, Le_i64s) \
INSTRUCTION (0x58, { FixedSize1 }, Numeric, Le_i64u) \
INSTRUCTION (0x59, { FixedSize1 }, Numeric, Ge_i64s) \
INSTRUCTION (0x5A, { FixedSize1 }, Numeric, Ge_i64u) \
\
INSTRUCTION (0x5B, { FixedSize1 }, Eq_f32) \
INSTRUCTION (0x5C, { FixedSize1 }, Ne_f32) \
INSTRUCTION (0x5D, { FixedSize1 }, Lt_f32) \
INSTRUCTION (0x5E, { FixedSize1 }, Gt_f32) \
INSTRUCTION (0x5F, { FixedSize1 }, Le_f32) \
INSTRUCTION (0x60, { FixedSize1 }, Ge_f32) \
\
INSTRUCTION (0x61, { FixedSize1 }, Eq_f64) \
INSTRUCTION (0x62, { FixedSize1 }, Ne_f64) \
INSTRUCTION (0x63, { FixedSize1 }, Lt_f64) \
INSTRUCTION (0x64, { FixedSize1 }, Gt_f64) \
INSTRUCTION (0x65, { FixedSize1 }, Le_f64) \
INSTRUCTION (0x66, { FixedSize1 }, Ge_f64) \
\
INSTRUCTION (0x67, { FixedSize1 }, Clz_i32) \
INSTRUCTION (0x68, { FixedSize1 }, Ctz_i32) \
INSTRUCTION (0x69, { FixedSize1 }, Popcnt_i32) \
INSTRUCTION (0x6A, { FixedSize1 }, Add_i32) \
INSTRUCTION (0x6B, { FixedSize1 }, Sub_i32) \
INSTRUCTION (0x6C, { FixedSize1 }, Mul_i32) \
INSTRUCTION (0x6D, { FixedSize1 }, Div_s_i32) \
INSTRUCTION (0x6E, { FixedSize1 }, Div_u_i32) \
INSTRUCTION (0x6F, { FixedSize1 }, Rem_s_i32) \
INSTRUCTION (0x70, { FixedSize1 }, Rem_u_i32) \
INSTRUCTION (0x71, { FixedSize1 }, And_i32) \
INSTRUCTION (0x72, { FixedSize1 }, Or_i32) \
INSTRUCTION (0x73, { FixedSize1 }, Xor_i32) \
INSTRUCTION (0x74, { FixedSize1 }, Shl_i32) \
INSTRUCTION (0x75, { FixedSize1 }, Shr_s_i32) \
INSTRUCTION (0x76, { FixedSize1 }, Shr_u_i32) \
INSTRUCTION (0x77, { FixedSize1 }, Rotl_i32) \
INSTRUCTION (0x78, { FixedSize1 }, Rotr_i32) \
\
INSTRUCTION (0x79, { FixedSize1 }, Clz_i64) \
INSTRUCTION (0x7A, { FixedSize1 }, Ctz_i64) \
INSTRUCTION (0x7B, { FixedSize1 }, Popcnt_i64) \
INSTRUCTION (0x7C, { FixedSize1 }, Add_i64) \
INSTRUCTION (0x7D, { FixedSize1 }, Sub_i64) \
INSTRUCTION (0x7E, { FixedSize1 }, Mul_i64) \
INSTRUCTION (0x7F, { FixedSize1 }, Div_s_i64) \
INSTRUCTION (0x80, { FixedSize1 }, Div_u_i64) \
INSTRUCTION (0x81, { FixedSize1 }, Rem_s_i64) \
INSTRUCTION (0x82, { FixedSize1 }, Rem_u_i64) \
INSTRUCTION (0x83, { FixedSize1 }, And_i64) \
INSTRUCTION (0x84, { FixedSize1 }, Or_i64) \
INSTRUCTION (0x85, { FixedSize1 }, Xor_i64) \
INSTRUCTION (0x86, { FixedSize1 }, Shl_i64) \
INSTRUCTION (0x87, { FixedSize1 }, Shr_s_i64) \
INSTRUCTION (0x88, { FixedSize1 }, Shr_u_i64) \
INSTRUCTION (0x89, { FixedSize1 }, Rotl_i64) \
INSTRUCTION (0x8A, { FixedSize1 }, Rotr_i64) \
\
INSTRUCTION (0x8B, { FixedSize1 }, Abs_f32) \
INSTRUCTION (0x8C, { FixedSize1 }, Neg_f32) \
INSTRUCTION (0x8D, { FixedSize1 }, Ceil_f32) \
INSTRUCTION (0x8E, { FixedSize1 }, Floor_f32) \
INSTRUCTION (0x8F, { FixedSize1 }, Trunc_f32) \
INSTRUCTION (0x90, { FixedSize1 }, Nearest_f32) \
INSTRUCTION (0x91, { FixedSize1 }, Sqrt_f32) \
INSTRUCTION (0x92, { FixedSize1 }, Add_f32) \
INSTRUCTION (0x93, { FixedSize1 }, Sub_f32) \
INSTRUCTION (0x94, { FixedSize1 }, Mul_f32) \
INSTRUCTION (0x95, { FixedSize1 }, Div_f32) \
INSTRUCTION (0x96, { FixedSize1 }, Min_f32) \
INSTRUCTION (0x97, { FixedSize1 }, Max_f32) \
INSTRUCTION (0x98, { FixedSize1 }, Copysign_f32) \
\
INSTRUCTION (0x99, { FixedSize1 }, Abs_f64) \
INSTRUCTION (0x9A, { FixedSize1 }, Neg_f64) \
INSTRUCTION (0x9B, { FixedSize1 }, Ceil_f64) \
INSTRUCTION (0x9C, { FixedSize1 }, Floor_f64) \
INSTRUCTION (0x9D, { FixedSize1 }, Trunc_f64) \
INSTRUCTION (0x9E, { FixedSize1 }, Nearest_f64) \
INSTRUCTION (0x9F, { FixedSize1 }, Sqrt_f64) \
INSTRUCTION (0xA0, { FixedSize1 }, Add_f64) \
INSTRUCTION (0xA1, { FixedSize1 }, Sub_f64) \
INSTRUCTION (0xA2, { FixedSize1 }, Mul_f64) \
INSTRUCTION (0xA3, { FixedSize1 }, Div_f64) \
INSTRUCTION (0xA4, { FixedSize1 }, Min_f64) \
INSTRUCTION (0xA5, { FixedSize1 }, Max_f64) \
INSTRUCTION (0xA6, { FixedSize1 }, Copysign_f64) \
\
INSTRUCTION (0xA7, { FixedSize1 }, i32_Wrap_i64) \
INSTRUCTION (0xA8, { FixedSize1 }, ) \
INSTRUCTION (0xA9, { FixedSize1 }, ) \
INSTRUCTION (0xAA, { FixedSize1 }, ) \
INSTRUCTION (0xAB, { FixedSize1 }, ) \
INSTRUCTION (0xAC, { FixedSize1 }, ) \
INSTRUCTION (0xAD, { FixedSize1 }, ) \
INSTRUCTION (0xAE, { FixedSize1 }, ) \
INSTRUCTION (0xAF, { FixedSize1 }, ) \
INSTRUCTION (0xB0, { FixedSize1 }, ) \
INSTRUCTION (0xB1, { FixedSize1 }, ) \
INSTRUCTION (0xB2, { FixedSize1 }, ) \
INSTRUCTION (0xB3, { FixedSize1 }, ) \
INSTRUCTION (0xB4, { FixedSize1 }, ) \
INSTRUCTION (0xB5, { FixedSize1 }, ) \
INSTRUCTION (0xB6, { FixedSize1 }, ) \
INSTRUCTION (0xB6, { FixedSize1 }, ) \
INSTRUCTION (0xB7, { FixedSize1 }, ) \
INSTRUCTION (0xB8, { FixedSize1 }, ) \
INSTRUCTION (0xB9, { FixedSize1 }, ) \
INSTRUCTION (0xBA, { FixedSize1 }, ) \
INSTRUCTION (0xBB, { FixedSize1 }, ) \
INSTRUCTION (0xBC, { FixedSize1 }, ) \
INSTRUCTION (0xBD, { FixedSize1 }, ) \
INSTRUCTION (0xBE, { FixedSize1 }, ) \
INSTRUCTION (0xBF, { FixedSize1 }, ) \

struct SectionBasepthread_mutexattr_setprotoc
{
    virtual ~SectionBase()
    {
    }

    uint id;
    std::string name;
    uint payload_len;
    uint8* payload;

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString (StringFormat ("%s not yet implemented", __func__));
    }
};

struct SectionTraits
{
    const char* name;
    SectionBase* (*make) ();
};

template <uint N>
struct Section  : SectionBase
{
};

typedef enum ImportTag { // aka desc
    ImportTag_Function = 0, // aka type
    ImportTag_Table = 1,
    ImportTag_Memory = 2,
    ImportTag_Global = 3,
} ImportTag;

struct MemoryType
{
    Limits limits;
};

struct ImportFunction
{
};

struct ImportTable
{
};

struct ImportMemory
{
};

struct GlobalType
{
    ValueType value_type;
    bool is_mutable;
};

struct Import
{
    std::string module;
    std::string name;
    ImportTag tag;
    // TODO virtual functions to model union
    union
    {
        uint function;
        MemoryType memory;
        GlobalType global;
        TableType table;
    };
};

struct Function
{
    // Functions are split between two sections: types in 3, locals/body in ?
    uint function_type;
    uint code_len;
    uint8* code;
    std::vector<uint8> locals; // TODO
};

struct Global
{
    GlobalType global_type;
    uint8* init;
};

struct Module
{
    MemoryMappedFile mmf;
    uint8* base = 0;
    uint64 file_size = 0;
    uint8* end = 0;
    std::vector<std::shared_ptr<SectionBase>> sections;
    std::vector<std::shared_ptr<SectionBase>> custom_sections; // FIXME

    std::vector<Function> functions; // sections 2 and 10
    std::vector<TableType> tables; // section 3
    std::vector<Global> globals; // section 6

    std::string read_string (uint8*& cursor);
    uint read_byte (uint8*& cursor);
    uint read_varuint7 (uint8*& cursor);
    uint read_varuint32 (uint8*& cursor);
    Limits read_limits (uint8*& cursor);
    MemoryType read_memorytype (uint8*& cursor);
    GlobalType read_globaltype (uint8*& cursor);
    TableType read_tabletype (uint8*& cursor);
    ValueType read_valuetype (uint8*& cursor);
    TableElementType read_elementtype (uint8*& cursor);
    bool read_mutable (uint8*& cursor);
    void read_section (uint8*& cursor);
    void read_module (const char* file_name);
};

// Initial representation of X and XSection are the same.
// This might evolve, i.e. into separate TypesSection and Types,
// or just Types that is not Section.

struct FunctionType
{
    // CONSIDER pointer into mmf
    std::vector<ValueType> parameters;
    std::vector<ValueType> results;

    void read_vector_ValueType (std::vector<ValueType>& result, Module* module, uint8*& cursor)
    {
        uint count = module->read_varuint32 (cursor);
        result.resize (count);
        for (uint i = 0; i < count; ++i)
            result [i] = module->read_valuetype (cursor);
    }

    void read_function_type (Module* module, uint8*& cursor)
    {
        read_vector_ValueType (parameters, module, cursor);
        read_vector_ValueType (results, module, cursor);
    }
};

struct Types : Section<1>
{
    std::vector<FunctionType> functionTypes;

    static SectionBase* make()
    {
        return new Types ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        printf ("reading section 1\n");
        uint count = module->read_varuint32 (cursor);
        functionTypes.resize (count);
        for (uint i = 0; i < count; ++i)
        {
            uint marker = module->read_byte (cursor);
            if (marker != 0x60)
                ThrowString ("malformed2 in Types::read");
            functionTypes [i].read_function_type (module, cursor);
        }
        printf ("read section 1\n");
    }
};

struct Imports : Section<2>
{
    std::vector<Import> data;

    static SectionBase* make()
    {
        return new Imports ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        read_imports (module, cursor);
    }

    void read_imports (Module* module, uint8*& cursor)
    {
        printf ("reading section 2\n");
        size_t count = module->read_varuint32 (cursor);
        data.resize (count);
        for (auto& r : data)
        {
            r.module = module->read_string (cursor);
            r.name = module->read_string (cursor);
            ImportTag tag = r.tag = (ImportTag)module->read_byte (cursor);
            printf ("import %s.%s %X\n", r.module.c_str (), r.name.c_str (), (uint)tag);
            switch (tag)
            {
                // TODO more specific import type and vtable?
            case ImportTag_Function:
                r.function = module->read_varuint32 (cursor);
                break;
            case ImportTag_Table:
                r.table = module->read_tabletype (cursor);
                break;
            case ImportTag_Memory:
                r.memory = module->read_memorytype (cursor);
                break;
            case ImportTag_Global:
                r.global = module->read_globaltype (cursor);
                break;
            default:
                ThrowString ("invalid ImportTag");
            }
        }
        printf ("read section 2\n");
    }
};

struct Functions : Section<3>
{
    static SectionBase* make()
    {
        return new Functions ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        read_functions (module, cursor);
    }

    void read_functions (Module* module, uint8*& cursor)
    {
        printf ("reading section 3\n");
        uint count = module->read_varuint32 (cursor);
        module->functions.resize (count);
        for (auto& a : module->functions)
            a.function_type = module->read_varuint32 (cursor);
        printf ("read section 3\n");
    }
};

struct Tables : Section<4>
{
    static SectionBase* make()
    {
        return new Tables ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        uint count = module->read_varuint32 (cursor);
        printf ("reading tables count:%X\n", count);
        ThrowString ("Tables::read not yet implemented");
        // Hello world does not have this section.
    }
};

struct Memory : Section<5>
{
    static SectionBase* make()
    {
        return new Memory ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Memory::read not yet implemented");
        // Hello world does not have this section.
    }
};

struct Globals : Section<6>
{
    static SectionBase* make()
    {
        return new Globals ();
    }

    void read_globals (Module* module, uint8*& cursor)
    {
        uint count = module->read_varuint32 (cursor);
        printf ("reading globals6 count:%X\n", count);
        module->globals.resize (count);
        for (auto& a: module->globals)
        {
            a.global_type = module->read_globaltype (cursor);
            a.init = cursor;
            printf ("read_globals value_type:%X  mutable:%X init:%p\n", a.global_type.value_type, a.global_type.is_mutable, a.init);

            // Init points to code -- Instructions until end of block 0x0B Instruction.
        }
        ThrowString ("Globals::read not yet implemented");
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        read_globals (module, cursor);
    }
};

struct Exports : Section<7>
{
    static SectionBase* make()
    {
        return new Exports ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Exports::read not yet implemented");
    }
};

struct Start : Section<8>
{
    static SectionBase* make()
    {
        return new Start ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Start::read not yet implemented");
    }
};

struct Elements : Section<9>
{
    static SectionBase* make()
    {
        return new Elements ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Elements::read not yet implemented");
    }
};

struct Code : Section<10>
{
    static SectionBase* make()
    {
        return new Code ();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Code::read not yet implemented");
    }
};

struct Data : Section<11>
{
    static SectionBase* make()
    {
        return new Data();
    }

    virtual void read (Module* module, uint8*& cursor)
    {
        ThrowString ("Data::read not yet implemented");
    }
};

const
SectionTraits section_traits [ ] =
{
    { 0 },
#define SECTIONS \
    SECTION (Types)     \
    SECTION (Imports)     \
    SECTION (Functions)     \
    SECTION (Tables)     \
    SECTION (Memory)     \
    SECTION (Globals)     \
    SECTION (Exports)     \
    SECTION (Start)     \
    SECTION (Elements)     \
    SECTION (Code)     \
    SECTION (Data)     \

#undef SECTION
#define SECTION(x) {#x, &x::make },
SECTIONS

};

uint Module::read_varuint7 (uint8*& cursor)
{
    // TODO move implementation here
    return w3::read_varuint7 (cursor, end);
}

uint Module::read_byte (uint8*& cursor)
{
    // TODO move implementation here
    return w3::read_byte (cursor, end);
}

// TODO efficiency
std::string Module::read_string (uint8*& cursor)
{
    uint length = read_varuint32 (cursor);
    if (length + cursor > end)
        ThrowString ("malformed in read_string");
    // TODO UTF8 handling
    std::string a = std::string ((char*)cursor, length);
    cursor += length;
    return a;
}

uint Module::read_varuint32 (uint8*& cursor)
{
    // TODO move implementation here
    return w3::read_varuint32 (cursor, end);
}

Limits Module::read_limits (uint8*& cursor)
{
    Limits limits { };
    uint tag = read_byte (cursor);
    switch (tag)
    {
    case 0:
    case 1:
        break;
    default:
        ThrowString ("invalid limit tag");
        break;
    }
    limits.hasMax = (tag == 1);
    limits.min = read_varuint32 (cursor);
    if (limits.hasMax)
        limits.max = read_varuint32 (cursor);
    return limits;
}

MemoryType Module::read_memorytype (uint8*& cursor)
{
    return MemoryType { read_limits (cursor) };
}

bool Module::read_mutable (uint8*& cursor)
{
    uint m = read_byte (cursor);
    switch (m)
    {
    case 0:
    case 1: break;
    default:
        ThrowString ("invalid mutable");
    }
    return m == 1;
}

ValueType Module::read_valuetype (uint8*& cursor)
{
    uint value_type = read_byte (cursor);
    switch (value_type)
    {
    default:
        ThrowString (StringFormat ("invalid ValueType:%X", value_type));
        break;
    case ValueType_I32:
    case ValueType_I64:
    case ValueType_F32:
    case ValueType_F64:
        break;
    }
    return (ValueType)value_type;
}

GlobalType Module::read_globaltype (uint8*& cursor)
{
    GlobalType globalType {};
    globalType.value_type = read_valuetype (cursor);
    globalType.is_mutable = read_mutable (cursor);
    return globalType;
}

TableElementType Module::read_elementtype (uint8*& cursor)
{
    TableElementType element_type = (TableElementType)read_byte (cursor);
    if (element_type != TableElementType_FuncRef)
        ThrowString ("invalid elementType");
    return element_type;
}

TableType Module::read_tabletype (uint8*& cursor)
{
    TableType tableType {};
    tableType.element_type = read_elementtype (cursor);
    tableType.limits = read_limits (cursor);
    return tableType;
}

void Module::read_section (uint8*& cursor)
{
    uint payload_len = 0;
    uint8* payload = 0;
    uint name_len = 0;

    uint id = read_varuint7 (cursor);

    if (id > 11)
    {
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_len:%X base:%p end:%p", __LINE__, id, payload, payload_len, base, end)); // UNDONE context (move to module or section)
    }

    payload_len = read_varuint32 (cursor);
    payload = cursor;
    name_len = 0;
    if (id == 0)
    {
        name_len = read_varuint32 (cursor);
        if (cursor + name_len > end)
            ThrowString (StringFormat ("malformed %d", __LINE__)); // UNDONE context (move to module or section)
    }
    if (payload + payload_len > end)
        ThrowString (StringFormat ("malformed line:%d id:%X payload:%p payload_len:%X base:%p end:%p", __LINE__, id, payload, payload_len, base, end)); // UNDONE context (move to module or section)

    cursor = payload + payload_len;

    if (id == 0)
    {
        // UNDONE custom sections
        return;
    }

    auto section = sections [id] = std::shared_ptr<SectionBase>(section_traits [id].make ());
    section->id = id;
    section->name = std::string ((char*)payload, name_len);
    section->payload_len = payload_len;
    section->payload = payload;
    section->read (this, payload);
}

void Module::read_module (const char* file_name)
{
    sections.resize (12); // FIXME
    mmf.read (file_name);
    base = (uint8*)mmf.base;
    file_size = mmf.file.get_file_size ();
    end = file_size + (uint8*)base;

    if (file_size < 8)
        ThrowString (StringFormat ("too small %s", file_name));

    uintLE& magic = (uintLE&)*base;
    uintLE& version = (uintLE&)*(base + 4);
    printf ("magic: %X\n", (uint)magic);
    printf ("version: %X\n", (uint)version);

    if (memcmp (&magic,"\0asm", 4))
        ThrowString (StringFormat ("incorrect magic: %X", (uint)magic));

    if (version != 1)
        ThrowString (StringFormat ("incorrect version: %X", (uint)version));

    // Valid module with no section
    if (file_size == 8)
        return;

    auto cursor = base + 8;
    while (cursor < end)
    {
        read_section (cursor);
    }

    assert (cursor == end);
}


}

using namespace w3;

int
main (int argc, char** argv)
{
#if 0 // test code
    char buf [99] = { 0 };
    uint len;
#define Xd(x) printf ("%s %I64d\n", #x, x);
#define Xx(x) printf ("%s %I64x\n", #x, x);
#define Xs(x) len = x; buf [len] = 0; printf ("%s %s\n", #x, buf);
    Xd (UIntGetPrecision(0));
    Xd (UIntGetPrecision(1));
    Xd (UIntGetPrecision(0x2));
    Xd (UIntGetPrecision(0x2));
    Xd (UIntGetPrecision(0x7));
    Xd (UIntGetPrecision(0x8));
    Xd (IntGetPrecision(0));
    Xd (IntGetPrecision(1));
    Xd (IntGetPrecision(0x2));
    Xd (IntGetPrecision(0x2));
    Xd (IntGetPrecision(0x7));
    Xd (IntGetPrecision(0x8));
    Xd (IntGetPrecision(0));
    Xd (IntGetPrecision(-1));
    Xd (IntGetPrecision(-0x2));
    Xd (IntGetPrecision(-0x2));
    Xd (IntGetPrecision(-0x7));
    Xd (IntGetPrecision(-0x8));
    Xd (IntToDec_GetLength(0))
    Xd (IntToDec_GetLength(1))
    Xd (IntToDec_GetLength(2))
    Xd (IntToDec_GetLength(300))
    Xd (IntToDec_GetLength(-1))
    Xx (SignExtend (0xf, 0));
    Xx (SignExtend (0xf, 1));
    Xx (SignExtend (0xf, 2));
    Xx (SignExtend (0xf, 3));
    Xx (SignExtend (0xf, 4));
    Xx (SignExtend (0xf, 5));
    Xd (IntToHex_GetLength (0xffffffffa65304e4));
    Xd (IntToHex_GetLength (0xfffffffa65304e4));
    Xd (IntToHex_GetLength (-1));
    Xd (IntToHex_GetLength (-1ui64>>4));
    Xd (IntToHex_GetLength (0xf));
    Xd (IntToHex_GetLength (32767));
    Xd (IntToHex_GetLength (-32767));
    Xs (IntToHex (32767, buf));
    Xs (IntToHex (-32767, buf));
    Xs (IntToHex8 (0x123, buf));
    Xs (IntToHex8 (0xffffffffa65304e4, buf));
    Xs (IntToHex8 (-1, buf));
    Xs (IntToHex (0x1, buf));
    Xs (IntToHex (0x12, buf));
    Xs (IntToHex (0x123, buf));
    Xs (IntToHex (0x12345678, buf));
    Xs (IntToHex (-1, buf));
    Xd (IntToHex_GetLength (0x1));
    Xd (IntToHex_GetLength (0x12));
    Xd (IntToHex_GetLength (0x12345678));
    Xd (IntToHex_GetLength (0x01234567));
    Xd (IntToHex_GetLength (-1));
    Xd (IntToHex_GetLength (~0u >> 1));
    Xd (IntToHex_GetLength (~0u >> 2));
    Xd (IntToHex_GetLength (~0u >> 4));
    exit(0);
#endif
#if 1
    try
#endif
    {
        Module m;
        m.read_module (argv [1]);
    }
#if 1
    catch (int er)
    {
        fprintf (stderr, "error 0x%08X\n", er);
    }
    catch (const string& er)
    {
        fprintf (stderr, "%s", er.c_str());
    }
#endif
    return 0;
}