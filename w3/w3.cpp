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
    throw a;
    //abort ();
}

void
ThrowInt (int i, const char* a = "")
{
    ThrowString (StringFormat ("error 0x%08X %s\n", i, a));
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
    ThrowString ("AssertFailed:" + string (condition) + ":" + extra + "\n");
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

typedef struct _Unused_t { } *Unused_t;

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
        //im.read (argv [1]);
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
