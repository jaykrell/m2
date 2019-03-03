// 2-clause BSD license unless that does not suffice
// else MIT like mono. Need to research the difference.

// https://www.ntcore.com/files/dotnetformat.htm
// https://www.ecma-international.org/publications/files/ECMA-ST/ECMA-335.pdf.

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

#ifdef METADATA_TABLE

// Metadata or other multi-evaluation macro tables must be first in the file.
// The alternatives:
//   1. Multiple source files. A build system. etc.
//   2. Lots of backward slashes, ok, but then bad compile error messages
//      without useful line number.


// The ordering of the tables here is important -- it assigns their enums.
// The ordering of the fields within the tables is also important.
// The second parameter to METADATA_FIELD can go away.
// Rows have a strongly typed in-memory form, formed on demand,
// an a very dynamic in-file form.
// Rows other rows via indices, coded indices, or pointers; or lists/vectors.
//#define METADATA_TABLES

/* table0x00*/ METADATA_TABLE (Module, NOTHING, 
    METADATA_FIELD2 (Module, Generation, uint16) /* ignore */
    METADATA_FIELD (Module, Name)
    METADATA_FIELD2 (Module, Mvid, guid)
    METADATA_FIELD2 (Module, EncId, guid) /* ignore */
    METADATA_FIELD2 (Module, EncBaseId, guid)) /* ignore */

/*table0x01*/ METADATA_TABLE (TypeRef, NOTHING,
    METADATA_FIELD (TypeRef, ResolutionScope)
    METADATA_FIELD (TypeRef, TypeName)
    METADATA_FIELD (TypeRef, TypeNameSpace))

/*table0x02*/ METADATA_TABLE (TypeDef, NOTHING,
    METADATA_FIELD3 (TypeDef, Flags, uint, TypeFlags)
    METADATA_FIELD (TypeDef, TypeName)
    METADATA_FIELD (TypeDef, TypeNameSpace)
    METADATA_FIELD3 (TypeDef, Extends, TypeDefOrRef, MetadataRow*)
    METADATA_FIELD (TypeDef, FieldList)
    METADATA_FIELD (TypeDef, MethodList))

/*table0x03*/ METADATA_TABLE_UNUSED(Unused3) /* FieldPtr nonstandard */

/*table0x04*/ METADATA_TABLE (Field, NOTHING /* : Member */,
    METADATA_FIELD3 (Field, Flags, uint16, FieldFlags)
    METADATA_FIELD (Field, Name)
    METADATA_FIELD (Field, signature))

/*table0x05*/ METADATA_TABLE_UNUSED(Unused5) /*MethodPtr nonstandard*/

/*table0x06*/METADATA_TABLE (MethodDef, NOTHING,
    METADATA_FIELD2 (MethodDef, RVA, uint)
    METADATA_FIELD3 (MethodDef, ImplFlags, uint16, MethodDefImplFlags) /* TODO higher level support */
    METADATA_FIELD3 (MethodDef, Flags, uint16, MethodDefFlags) /* TODO higher level support */
    METADATA_FIELD (MethodDef, Name)
    METADATA_FIELD (MethodDef, signature)      /* Blob heap, 7 bit encode/decode */
    METADATA_FIELD (MethodDef, ParamList)) /* Param table, start, until table end, or start of next MethodDef; index into Param table, 2 or 4 bytes */

/*table0x07*/ METADATA_TABLE_UNUSED(Unused7) /*ParamPtr nonstandard*/

/*table0x08*/METADATA_TABLE (Param, NOTHING,
    METADATA_FIELD2 (Param, Flags, uint16)
    METADATA_FIELD2 (Param, Sequence, uint16)
    METADATA_FIELD (Param, Name))

/*table0x09*/METADATA_TABLE (InterfaceImpl, NOTHING,
    METADATA_FIELD2 (InterfaceImpl, Class, TypeDef)
    METADATA_FIELD3 (InterfaceImpl, Interface, TypeDefOrRef, MetadataRow*))

/*table0x0A*/METADATA_TABLE (MemberRef, NOTHING, /* FieldRef */
    METADATA_FIELD3 (MemberRef, Class, MemberRefParent, MetadataRow*)
    METADATA_FIELD (MemberRef, Name)       /*string*/
    METADATA_FIELD (MemberRef, signature)) /*blob*/

/*table0x0B*/METADATA_TABLE (Constant, NOTHING,
    METADATA_FIELD2 (Constant, Type, uint8)
    METADATA_FIELD2 (Constant, Pad, uint8)
    METADATA_FIELD3 (Constant, Parent, HasConstant, MetadataRow*)
    METADATA_FIELD2 (Constant, Value, blob)
    METADATA_FIELD3 (Constant, IsNull, NotStored, bool))

/*table0x0C*/METADATA_TABLE (CustomAttribute, NOTHING,
    METADATA_FIELD3 (CustomAttribute, Parent, HasCustomAttribute, MetadataRow*)
    METADATA_FIELD3 (CustomAttribute, Type, CustomAttributeType, MetadataRow*)
    METADATA_FIELD2 (CustomAttribute, Value, blob))

/*table0x0D*/METADATA_TABLE (FieldMarshal, NOTHING,
    METADATA_FIELD3 (FieldMarshal, Parent, HasFieldMarshal, MetadataRow*)
    METADATA_FIELD2 (FieldMarshal, NativeType, blob))

/*table0x0E*/METADATA_TABLE (DeclSecurity, NOTHING,
    METADATA_FIELD3 (DeclSecurity, Action, uint16, DeclSecurityAction)
    METADATA_FIELD3 (DeclSecurity, Parent_or_Type_TODO, HasDeclSecurity, MetadataRow*)
    METADATA_FIELD2 (DeclSecurity, PermissionSet_or_Value_TODO, blob))

/*table0x0F*/ METADATA_TABLE (ClassLayout, NOTHING,
    METADATA_FIELD2 (ClassLayout, PackingSize, uint16)
    METADATA_FIELD2 (ClassLayout, ClassSize, uint)
    METADATA_FIELD2 (ClassLayout, Parent, TypeDef))

/*table0x10*/ METADATA_TABLE (FieldLayout, NOTHING,
    METADATA_FIELD2 (FieldLayout, Offset, uint)
    METADATA_FIELD3 (FieldLayout, Field, Field, Field_t*))

/*table0x11*/ METADATA_TABLE (StandAloneSig, NOTHING,
    METADATA_FIELD2 (StandAloneSig, signature, blob))

/*table0x12*/ METADATA_TABLE (EventMap, NOTHING,
    METADATA_FIELD2 (EventMap, Parent, TypeDef)
    METADATA_FIELD (EventMap, EventList))

/*table0x13*/ METADATA_TABLE_UNUSED(Unused13) /* EventPtr nonstandard */

/*table0x14*/ METADATA_TABLE (Event, NOTHING,
    METADATA_FIELD3 (Event, Flags, uint16, EventFlags)
    METADATA_FIELD2 (Event, Name, string)
    METADATA_FIELD3 (Event, EventType, TypeDefOrRef, MetadataRow*))

/*table0x15*/ METADATA_TABLE (PropertyMap, NOTHING,
    METADATA_FIELD2 (PropertyMap, Parent, TypeDef)
    METADATA_FIELD (PropertyMap, PropertyList))

/*table0x16*/ METADATA_TABLE_UNUSED(Unused16) /* PropertyPtr */

/*table0x17*/ METADATA_TABLE (Property, NOTHING,
    METADATA_FIELD2 (Property, Flags, uint16)
    METADATA_FIELD2 (Property, Name, string)
    METADATA_FIELD2 (Property, Type, blob))

/* .property and .event
   Links Events and Properties to specific methods.
   For example one Event can be associated to more methods.
   A property uses this table to associate get/set methods. */
/*table0x18*/ METADATA_TABLE (MethodSemantics, NOTHING,
    METADATA_FIELD3 (MethodSemantics, Semantics, uint16, MethodSemanticsFlags)
    METADATA_FIELD3 (MethodSemantics, Method, MethodDef, MethodDef_t*) /* index into MethodDef table, 2 or 4 bytes */
    METADATA_FIELD3 (MethodSemantics, Association, HasSemantics, MetadataRow*)) /* Event or Property, CodedIndex */

/*table0x19*/ METADATA_TABLE (MethodImpl, NOTHING,
    METADATA_FIELD2 (MethodImpl, Class, TypeDef)
    METADATA_FIELD3 (MethodImpl, MethodBody, MethodDefOrRef, MetadataRow*)
    METADATA_FIELD3 (MethodImpl, MethodDeclaration, MethodDefOrRef, MetadataRow*))

/*table0x1A*/ METADATA_TABLE (ModuleRef, NOTHING,
    METADATA_FIELD2 (ModuleRef, Name, string))

/*table0x1B*/ METADATA_TABLE (TypeSpec, NOTHING,
    METADATA_FIELD2 (TypeSpec, signature, blob))

/*table0x1C*/ METADATA_TABLE (ImplMap, NOTHING,
    METADATA_FIELD3 (ImplMap, MappingFlags, uint16, PInvokeAttributes)
    METADATA_FIELD3 (ImplMap, MemberForwarded, MemberForwarded, MethodDef_t*)
    METADATA_FIELD2 (ImplMap, ImportName, string)
    METADATA_FIELD3 (ImplMap, ImportScope, ModuleRef, ModuleRef_t*))

/*table0x1D*/ METADATA_TABLE (FieldRVA, NOTHING,
    METADATA_FIELD2 (FieldRVA, RVA, uint)
    METADATA_FIELD3 (FieldRVA, Field, Field, Field_t*))

/*table0x1E*/ METADATA_TABLE_UNUSED(Unused1E) /* ENCLog */

/*table0x1F*/ METADATA_TABLE_UNUSED(Unused1F) /* ENDMap */

/*table0x20*/ METADATA_TABLE (Assembly, NOTHING,
    METADATA_FIELD2 (Assembly, HashAlgId, uint)
    METADATA_FIELD2 (Assembly, MajorVersion, uint16)
    METADATA_FIELD2 (Assembly, MinorVersion, uint16)
    METADATA_FIELD2 (Assembly, BuildNumber, uint16)
    METADATA_FIELD2 (Assembly, RevisionNumber, uint16)
    METADATA_FIELD3 (Assembly, Flags, uint, AssemblyFlags)
    METADATA_FIELD2 (Assembly, PublicKey, blob)
    METADATA_FIELD2 (Assembly, Name, string)
    METADATA_FIELD2 (Assembly, Culture, string))

/*table0x21*/ METADATA_TABLE (AssemblyProcessor, NOTHING,
    METADATA_FIELD2 (AssemblyProcessor, Processor, uint))

/*table0x22*/ METADATA_TABLE (AssemblyOS, NOTHING,
    METADATA_FIELD2 (AssemblyOS, OSPlatformID, uint)
    METADATA_FIELD2 (AssemblyOS, OSMajorVersion, uint)
    METADATA_FIELD2 (AssemblyOS, OSMinorVersion, uint))

/*table0x23*/ METADATA_TABLE (AssemblyRef, NOTHING,
    METADATA_FIELD2 (AssemblyRef, MajorVersion, uint16)
    METADATA_FIELD2 (AssemblyRef, MinorVersion, uint16)
    METADATA_FIELD2 (AssemblyRef, BuildNumber, uint16)
    METADATA_FIELD2 (AssemblyRef, RevisionNumber, uint16)
    METADATA_FIELD3 (AssemblyRef, Flags, uint, AssemblyFlags)
    METADATA_FIELD2 (AssemblyRef, PublicKey, blob)
    METADATA_FIELD2 (AssemblyRef, Name, string)
    METADATA_FIELD2 (AssemblyRef, Culture, string)
    METADATA_FIELD2 (AssemblyRef, HashValue, blob))

/*table0x24*/ METADATA_TABLE (AssemblyRefProcessor, NOTHING,
    METADATA_FIELD2 (AssemblyRefProcessor, Processor, uint)
    METADATA_FIELD3 (AssemblyRefProcessor, AssemblyRef, AssemblyRef, uint /* index into AssemblyRef table but ignored */))

/*table0x25*/ METADATA_TABLE (AssemblyRefOS, NOTHING,
    METADATA_FIELD2 (AssemblyRefOS, OSPlatformID, uint)
    METADATA_FIELD2 (AssemblyRefOS, OSMajorVersion, uint)
    METADATA_FIELD2 (AssemblyRefOS, OSMinorVersion, uint)
    METADATA_FIELD3 (AssemblyRefOS, AssemblyRef, AssemblyRef, uint))

/*table0x26*/ METADATA_TABLE (File, NOTHING,
    METADATA_FIELD3 (File, Flags, uint, FileFlags)
    METADATA_FIELD2 (File, Name, string)
    METADATA_FIELD2 (File, HashValue, blob))

/*table0x27*/ METADATA_TABLE (ExportedType, NOTHING,
    METADATA_FIELD3 (ExportedType, Flags, uint, TypeFlags)
    METADATA_FIELD3 (ExportedType, TypeDefId, uint, uint)
    METADATA_FIELD  (ExportedType, TypeName)
    METADATA_FIELD  (ExportedType, TypeNameSpace)
    METADATA_FIELD3 (ExportedType, Implementation, Implementation, MetadataToken))

/*table0x28*/ METADATA_TABLE (ManifestResource, NOTHING,
    METADATA_FIELD2 (ManifestResource, Offset, uint)
    METADATA_FIELD3 (ManifestResource, Flags, uint, ManifestResourceFlags)
    METADATA_FIELD2 (ManifestResource, Name, string)
    METADATA_FIELD3 (ManifestResource, Implementation, Implementation, MetadataRow*))

/*table0x29*/ METADATA_TABLE (NestedClass, NOTHING,
    METADATA_FIELD2 (NestedClass, NestedClass, TypeDef)
    METADATA_FIELD2 (NestedClass, EnclosingClass, TypeDef))

/*table0x2A*/ METADATA_TABLE (GenericParam, NOTHING,
    METADATA_FIELD2 (GenericParam, Number, uint16)
    METADATA_FIELD3 (GenericParam, Flags, uint16, GenericParamFlags)
    METADATA_FIELD3 (GenericParam, Owner, TypeOrMethodDef, MetadataRow*)
    METADATA_FIELD2 (GenericParam, Name, string))

/*table0x2B*/ METADATA_TABLE (MethodSpec, NOTHING,
    METADATA_FIELD3 (MethodSpec, Method, MethodDefOrRef, MetadataRow*)
    METADATA_FIELD2 (MethodSpec, Instantiation, blob))

/*table0x2C*/ METADATA_TABLE (GenericParamConstraint, NOTHING,
    METADATA_FIELD3 (GenericParamConstraint, Owner, GenericParam, GenericParam_t*)
    METADATA_FIELD3 (GenericParamConstraint, Constraint, TypeDefOrRef, MetadataRow*))

#else

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

namespace m2
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
    //fputs (("AssertFailedFormat:" + string (condition) + ":" + m2::StringFormatVa (format, args) + "\n").c_str (), stderr);
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

struct DosHeader
{
    char mz [2];
    uint8 pad [64 - 6];
    uintLE pe;

    bool CheckSignature () { return mz [0] == 'M' && mz [1] == 'Z'; }
    uint GetPE () { return Unpack (pe); }
};

struct FileHeader
{
    uintLE16 Machine;
    uintLE16 NumberOfSections;
    uintLE TimeDateStamp;
    uintLE PointerToSymbolTable;
    uintLE NumberOfSymbols;
    uintLE16 SizeOfOptionalHeader;
    uintLE16 Characteristics;
};

struct DataDirectory_t
{
    uintLE VirtualAddress;
    uintLE Size;
};

struct OptionalHeader32
{
    uintLE16 Magic;

    uint8  unused_MajorLinkerVersion;
    uint8  unused_MinorLinkerVersion;
    uintLE unused_SizeOfCode;
    uintLE unused_SizeOfInitializedData;
    uintLE unused_SizeOfUninitializedData;
    uintLE unused_AddressOfEntryPoint;
    uintLE unused_BaseOfCode;
    uintLE unused_BaseOfData;
    uintLE unused_ImageBase;
    uintLE unused_SectionAlignment;
    uintLE unused_FileAlignment;
    uintLE16 unused_MajorOperatingSystemVersion;
    uintLE16 unused_MinorOperatingSystemVersion;
    uintLE16 unused_MajorImageVersion;
    uintLE16 unused_MinorImageVersion;
    uintLE16 unused_MajorSubsystemVersion;
    uintLE16 unused_MinorSubsystemVersion;
    uintLE unused_Win32VersionValue;
    uintLE unused_SizeOfImage;
    uintLE unused_SizeOfHeaders;
    uintLE unused_CheckSum;
    uintLE16 unused_Subsystem;
    uintLE16 unused_DllCharacteristics;
    uintLE unused_SizeOfStackReserve;
    uintLE unused_SizeOfStackCommit;
    uintLE unused_SizeOfHeapReserve;
    uintLE unused_SizeOfHeapCommit;
    uintLE unused_LoaderFlags;

    uintLE NumberOfRvaAndSizes;
    //DataDirectory_t DataDirectory_t [NumberOfRvaAndSizes];
};

struct OptionalHeader64
{
    uintLE16 Magic;

    uint8  unused_MajorLinkerVersion;
    uint8  unused_MinorLinkerVersion;
    uintLE unused_SizeOfCode;
    uintLE unused_SizeOfInitializedData;
    uintLE unused_SizeOfUninitializedData;
    uintLE unused_AddressOfEntryPoint;
    uintLE unused_BaseOfCode;
    uintLE64 unused_ImageBase;
    uintLE unused_SectionAlignment;
    uintLE unused_FileAlignment;
    uintLE16 unused_MajorOperatingSystemVersion;
    uintLE16 unused_MinorOperatingSystemVersion;
    uintLE16 unused_MajorImageVersion;
    uintLE16 unused_MinorImageVersion;
    uintLE16 unused_MajorSubsystemVersion;
    uintLE16 unused_MinorSubsystemVersion;
    uintLE unused_Win32VersionValue;
    uintLE unused_SizeOfImage;
    uintLE unused_SizeOfHeaders;
    uintLE unused_CheckSum;
    uintLE16 unused_Subsystem;
    uintLE16 unused_DllCharacteristics;
    uintLE64 unused_SizeOfStackReserve;
    uintLE64 unused_SizeOfStackCommit;
    uintLE64 unused_SizeOfHeapReserve;
    uintLE64 unused_SizeOfHeapCommit;
    uintLE unused_LoaderFlags;

    uintLE NumberOfRvaAndSizes;
    //DataDirectory_t DataDirectory_t [NumberOfRvaAndSizes];
};

const uint kDataDirectory_Export = 0;
const uint kDataDirectory_Import = 1;
const uint kDataDirectory_Resource = 2;
const uint kDataDirectory_Exception = 3;
const uint kDataDirectory_Security = 4;
const uint kDataDirectory_Reloc = 5;
const uint kDataDirectory_Debug = 6;
const uint kDataDirectory_Arch = 7;
const uint kDataDirectory_GlobalPtr = 8;
const uint kDataDirectory_Tls = 9;
const uint kDataDirectory_LoadConfig = 10;
const uint kDataDirectory_BoundImport = 11;
const uint kDataDirectory_Iat = 12;
const uint kDataDirectory_DelayImport = 13;
const uint kDataDirectory_Com = 14;

static
const char* DataDirectoryName (uint a)
{
    switch (a) {
#define X(x) case kDataDirectory_ ## x: return "kDataDirectory_" #x;
X(Export)
X(Import)
X(Resource)
X(Exception)
X(Security)
X(Reloc)
X(Debug)
X(Arch)
X(GlobalPtr)
X(Tls)
X(LoadConfig)
X(BoundImport)
X(Iat)
X(DelayImport)
X(Com)
#undef X
    }
    return "unknown";
}

struct SectionHeader;

struct NtHeaders
{
    uintLE signature;
    FileHeader file_header;
    uintLE16 OptionalHeader; // magic value

    SectionHeader*
    GetFirstSectionHeader ()
    {
        return (SectionHeader*)((char*)&OptionalHeader + Unpack(file_header.SizeOfOptionalHeader));
    }
};

struct SectionHeader
{
    // In very old images, VirtualSize is zero, in which case, use SizeOfRawData.
    char Name [8];
    uintLE VirtualSize;
    uintLE VirtualAddress;
    uintLE SizeOfRawData;
    uintLE PointerToRawData;
    uintLE PointerToRelocations;
    uintLE PointerToLinenumbers;
    uintLE16 NumberOfRelocations;
    uintLE16 NumberOfLinenumbers;
    uintLE Characteristics;
};

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

struct String_t
{
    char* chars;
    size_t length;
};

struct UString_t
{
    char16_t* chars;
    uint length;
    bool ascii;
};

struct Blob_t
{
    void* data;
    uint size;
};

struct IMetadataTable
{
    virtual void* iat (size_t n) { return 0; }
    virtual void* iresize (size_t) { return 0; }
    virtual ~IMetadataTable () { }
};

struct Member
{
    uint offset;
};

typedef struct _Unused_t { } *Unused_t;

#if HAS_TYPED_ENUM
#define BEGIN_ENUM(name, type) enum name : type
#define END_ENUM(name, type) ;
#else
#define BEGIN_ENUM(name, type) enum _ ## name
#define END_ENUM(name, type) ; typedef type name;
#endif

BEGIN_ENUM(MethodDefFlags, uint16) // table0x06
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
END_ENUM(MethodDefFlags, uint16) // table0x06

BEGIN_ENUM(MethodDefImplFlags, uint16) // table0x06
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
END_ENUM(MethodDefImplFlags, uint16) // table0x06

BEGIN_ENUM(TypeFlags, uint)
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
END_ENUM(TypeFlags, uint)

BEGIN_ENUM(EventFlags, uint16)
{
    EventFlags_SpecialName           =   0x0200,     // event is special. Name describes how.
    // Reserved flags for Runtime use only.
    EventFlags_RTSpecialName         =   0x0400      // Runtime(metadata internal APIs) should check name encoding.
}
END_ENUM(EventFlags, uint16)

struct MetadataRow;

// Avoiding virtual functions.

typedef void (*Metadata_SetCName_t)(MetadataRow*);

struct MetadataFunctions
{
    Metadata_SetCName_t SetCName;
};

static void MetadataRow_SetCName (MetadataRow*)
{
    printf("MetadataRow_SetCName\n");
    __debugbreak();
}

struct TypeDef_t;
struct TypeRef_t;
struct TypeSpec_t;

static void TypeDef_SetCName (TypeDef_t* self);
static void TypeRef_SetCName (TypeRef_t* self);
static void TypeSpec_SetCName (TypeSpec_t* self);

const static MetadataFunctions ModuleFunctions = { };

const static MetadataFunctions TypeRefFunctions =
{
    (Metadata_SetCName_t)TypeRef_SetCName,
};

const static MetadataFunctions TypeDefFunctions =
{
    (Metadata_SetCName_t)TypeDef_SetCName,
};

const static MetadataFunctions FieldFunctions = { };
const static MetadataFunctions FieldMarshalFunctions = { };
const static MetadataFunctions MethodDefFunctions = { };
const static MetadataFunctions ParamFunctions = { };
const static MetadataFunctions InterfaceImplFunctions = { };
const static MetadataFunctions MemberRefFunctions = { };
const static MetadataFunctions ConstantFunctions = { };
const static MetadataFunctions CustomAttributeFunctions = { };
const static MetadataFunctions DeclSecurityFunctions = { };
const static MetadataFunctions ClassLayoutFunctions = { };
const static MetadataFunctions FieldLayoutFunctions = { };
const static MetadataFunctions StandAloneSigFunctions = { };
const static MetadataFunctions EventMapFunctions = { };
const static MetadataFunctions EventFunctions = { };
const static MetadataFunctions PropertyMapFunctions = { };
const static MetadataFunctions PropertyFunctions = { };
const static MetadataFunctions MethodSemanticsFunctions = { };
const static MetadataFunctions MethodImplFunctions = { };
const static MetadataFunctions ModuleRefFunctions = { };
const static MetadataFunctions NestedClassFunctions = { };

const static MetadataFunctions TypeSpecFunctions =
{
    (Metadata_SetCName_t)TypeSpec_SetCName,
};

const static MetadataFunctions ImplMapFunctions = { };
const static MetadataFunctions FieldRVAFunctions = { };
const static MetadataFunctions AssemblyFunctions = { };
const static MetadataFunctions AssemblyProcessorFunctions = { };
const static MetadataFunctions AssemblyOSFunctions = { };
const static MetadataFunctions AssemblyRefFunctions = { };
const static MetadataFunctions AssemblyRefProcessorFunctions = { };
const static MetadataFunctions AssemblyRefOSFunctions = { };
const static MetadataFunctions FileFunctions = { };
const static MetadataFunctions ExportedTypeFunctions = { };
const static MetadataFunctions ManifestResourceFunctions = { };
const static MetadataFunctions NextedClassFunctions = { };
const static MetadataFunctions GenericParamFunctions = { };
const static MetadataFunctions MethodSpecFunctions = { };
const static MetadataFunctions GenericParamConstraintFunctions = { };

// TODO This is the base of every row type.
// It is especially for coded indices.
// TODO This should actually be templatized with a bitmask of allowed table indices,
// and provide some "dispatch assistance" where we guaranteeably switch on all allowed
// tables.
struct MetadataRow
{
    MetadataFunctions const * Functions;
    std::string cname;
    //uint8 table_index;
    //MetadataRow () { }
    //MetadataRow (const MetadataRow&) = default;
    //virtual ~MetadataRow () { }
    //virtual void Print() { }
};

BEGIN_ENUM(FieldFlags, uint16)
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
END_ENUM(FieldFlags, uint16)

BEGIN_ENUM(DeclSecurityAction, uint16)
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
END_ENUM(DeclSecurityAction, uint16) // TODO get the values


struct Interface_t
{
    vector<MetadataRow*> methods;
};

struct Signature
{
    Blob_t blob; // TODO
};

struct AssemblyRef;
struct File;

BEGIN_ENUM(MethodSemanticsFlags, uint16)
{
    MethodSemanticsFlags_Setter = 1, // msSetter
    MethodSemanticsFlags_Getter = 2,
    MethodSemanticsFlags_Other = 4,
    MethodSemanticsFlags_AddOn = 8,
    MethodSemanticsFlags_RemoveOn = 0x10,
    MethodSemanticsFlags_Fire = 0x20
}
END_ENUM(MethodSemanticsFlags, uint16)

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
const uint HeapOffsetSize_String = 1;
const uint HeapOffsetSize_Guid = 2;
const uint HeapOffsetSize_Blob = 4;

struct Guid
{
    uint8 bytes [16];
};

// TODO enum/bitfield
const uint CorILMethod_TinyFormat = 2;
const uint CorILMethod_FatFormat = 3;
const uint CorILMethod_MoreSects = 8;
const uint CorILMethod_InitLocals = 0x10;

struct MethodHeaderTiny
// no locals
// no exceptions
// no extra data sections (what does this mean?)
// maxstack=8
{
    uint8 Value;
    uint GetFlags () { return (uint)(Value & 3); }
    uint GetSize () { return (uint)(Value >> 2); }
};

struct MethodHeaderFat
{
    uintLE16 FlagsAndHeaderSize;
    uintLE16 MaxStack;
    uintLE CodeSize;
    uintLE LocalVarSigTok;

    bool MoreSects () { return !!(GetFlags () & CorILMethod_MoreSects); }
    bool InitLocals () { return !!(GetFlags () & CorILMethod_InitLocals); }
    uint GetFlags () { return FlagsAndHeaderSize & 0xFFF; }
    uint GetHeaderSize () { return FlagsAndHeaderSize >> 12; } // should be 3
};

#define NOTHING /* nothing */

// #define METADATA_TABLES ..


// Every table has two maybe three maybe four sets of types/data/forms.
// 1. A very typed form. Convenient to work with. Does the most work to form.
// 2. A very tokeny form. Coded index become tokens. Does minimal work to form. Needed?
// 3. An indirect constant form that describes it enough to size and extract.
// 4. An indirect dynamic per-module form with field sizes and offsets.
//    This is just, again, field sizes, and offsets. Used in conjunction
//    with form 3. While a row of all fixed size/offset type is possible,
//    this is simply correctly sized arrays of size/offset and maybe link to form 3.
#define metadata_schema_TYPED_blob              Blob_t
#define metadata_schema_TYPED_guid              Guid*
#define metadata_schema_TYPED_string            String_t
#define metadata_schema_TYPED_uint16            uint16
#define metadata_schema_TYPED_uint              uint
#define metadata_schema_TYPED_uint8             uint8
#define metadata_schema_TYPED_Class             Class_t*
#define metadata_schema_TYPED_Extends           MetadataRow*
#define metadata_schema_TYPED_FieldList         vector<Field_t*>
#define metadata_schema_TYPED_Interface         Interface_t*
#define metadata_schema_TYPED_MethodList        vector<MetadataRow*>
#define metadata_schema_TYPED_Name              String_t
#define metadata_schema_TYPED_ParamList         vector<Param_t*>
#define metadata_schema_TYPED_PropertyList      vector<MetadataRow*>
#define metadata_schema_TYPED_RVA               uint
#define metadata_schema_TYPED_ResolutionScope   MetadataRow*
#define metadata_schema_TYPED_Sequence          uint16
#define metadata_schema_TYPED_signature         Signature
#define metadata_schema_TYPED_TypeDef           TypeDef_t*
#define metadata_schema_TYPED_TypeName          String_t
#define metadata_schema_TYPED_TypeNameSpace     String_t
#define metadata_schema_TYPED_Unused            Unused_t
#define metadata_schema_TYPED_NotStored         Unused_t
#define metadata_schema_TYPED_EventList         vector<MetadataRow*>

#define METADATA_FIELD(table, name) METADATA_FIELD2 (table, name, name)

// Generate enums for tables.
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_TABLE(name, base, fields)                         k## name, // TODO longer name?
#define METADATA_FIELD2(table, name, type)                                /* nothing */
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type)    /* nothing */
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) k ## Unused_ ## name,
enum : uint8 {
#include __FILE__ // METADATA_TABLES
    kMethodRef = kMemberRef,
    kFieldRef = kMemberRef,
};

// CLR Metadata often has indices into one of a few tables.
// The indices are tagged, in a scheme specific to the possibilities of that index.
// As well, such indices have variable size depending on the maximum index.
// i.e. find maximum of table sizes among possibly referenced, subtract
// off bits from 16 to indicate which table, see if enough. If so, such coded
// indices are 16 bits, for this module. Else 32.
struct CodedIndex_t
{
    //const char name [24];
    uint tag_size;
    uint count;
    uint map;
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
#define CODED_INDICES                                                                                       \
CODED_INDEX (TypeDefOrRef,      3, {kTypeDef COMMA kTypeRef COMMA kTypeSpec})                               \
CODED_INDEX (HasConstant,       3, {kField COMMA kParam COMMA kProperty})                                   \
CODED_INDEX (HasFieldMarshal,   2, {kField COMMA kParam})                                                   \
CODED_INDEX (HasDeclSecurity,   3, {kTypeDef COMMA kMethodDef COMMA kAssembly})                             \
CODED_INDEX (MemberRefParent,   5, {kTypeDef COMMA kTypeRef COMMA kModuleRef COMMA kMethodDef COMMA kTypeSpec}) \
CODED_INDEX (HasSemantics,      2, {kEvent COMMA kProperty})                                                \
CODED_INDEX (MethodDefOrRef,    2, {kMethodDef COMMA kMethodRef})                                           \
CODED_INDEX (MemberForwarded,   2, {kField COMMA kMethodDef})                                               \
CODED_INDEX (Implementation,    3, {kFile COMMA kAssemblyRef COMMA kExportedType})                          \
/* CodeIndex(CustomAttributeType) has a range of 5 values but only 2 are valid.                             \
 * -1 is "null" and this is a wart, requiring signed numbers. */                                            \
CODED_INDEX (CustomAttributeType, 5, {-1 COMMA -1 COMMA kMethodDef COMMA kMemberRef COMMA -1})              \
CODED_INDEX (ResolutionScope, 4, {kModule COMMA kModuleRef COMMA kAssemblyRef COMMA kTypeRef})              \
CODED_INDEX (TypeOrMethodDef, 2, {kTypeDef COMMA kMethodDef})                                               \
CODED_INDEX (HasCustomAttribute, 22,                                                                        \
      {kMethodDef              COMMA kField         COMMA kTypeRef      COMMA kTypeDef          COMMA kParam COMMA         /* HasCustomAttribute */ \
       kInterfaceImpl          COMMA kMemberRef     COMMA kModule       COMMA kDeclSecurity     COMMA kProperty COMMA      /* HasCustomAttribute */ \
       kEvent                  COMMA kStandAloneSig COMMA kModuleRef    COMMA kTypeSpec         COMMA kAssembly COMMA      /* HasCustomAttribute */ \
       kAssemblyRef            COMMA kFile          COMMA kExportedType COMMA kManifestResource COMMA kGenericParam COMMA  /* HasCustomAttribute */ \
       kGenericParamConstraint COMMA kMethodSpec                                          })                               /* HasCustomAttribute */

#define CODED_INDEX(name, count, values) CodedIndex_ ## name,
BEGIN_ENUM(CodedIndex, uint8)
{
CODED_INDICES
#undef CODED_INDEX
    CodedIndex_Count
}
END_ENUM(CodedIndex, uint8)


struct CodedIndexMap_t // TODO array and named
{
#define CODED_INDEX(name, count, values) int8 name [count];
CODED_INDICES
#undef CODED_INDEX
};

const CodedIndexMap_t CodedIndexMap = {
#define CODED_INDEX(name, count, values) values,
CODED_INDICES
#undef CODED_INDEX
};

#define LOG_BASE2_X(a, x) (a) > (1u << x) ? (x + 1u) :
#define LOG_BASE2(a)                                                                                \
  (LOG_BASE2_X(a, 31) LOG_BASE2_X(a, 30)                                                          \
   LOG_BASE2_X(a, 29) LOG_BASE2_X(a, 28) LOG_BASE2_X(a, 27) LOG_BASE2_X(a, 26) LOG_BASE2_X(a, 25) \
   LOG_BASE2_X(a, 24) LOG_BASE2_X(a, 23) LOG_BASE2_X(a, 22) LOG_BASE2_X(a, 21) LOG_BASE2_X(a, 20) \
   LOG_BASE2_X(a, 19) LOG_BASE2_X(a, 18) LOG_BASE2_X(a, 17) LOG_BASE2_X(a, 16) LOG_BASE2_X(a, 15) \
   LOG_BASE2_X(a, 14) LOG_BASE2_X(a, 13) LOG_BASE2_X(a, 12) LOG_BASE2_X(a, 11) LOG_BASE2_X(a, 10) \
   LOG_BASE2_X(a,  9) LOG_BASE2_X(a,  8) LOG_BASE2_X(a,  7) LOG_BASE2_X(a,  6) LOG_BASE2_X(a,  5) \
   LOG_BASE2_X(a,  4) LOG_BASE2_X(a,  3) LOG_BASE2_X(a,  2) LOG_BASE2_X(a,  1) LOG_BASE2_X(a,  0) 0)

#if __cplusplus >= 201703L || _MSC_VER >= 1900
#define CountOf(x) (std::size(x)) // C++17
#else
#define CountOf(x) (sizeof (x) / sizeof ((x) [0])) // TODO
#endif
#define CountOfField(x, y) (CountOf(x().y))
#define CODED_INDEX(name, count, values) CodedIndex_t name;

union CodedIndices_t
{
    CodedIndex_t array [CodedIndex_Count];
    struct {
CODED_INDICES
#undef CODED_INDEX
    } name;
};

#undef CODED_INDEX
#define CODED_INDEX(name, count, values) #name,
const char *CodeIndexName [ ] = {
CODED_INDICES
#undef CODED_INDEX
};

const CodedIndices_t CodedIndices = {{
#define CODED_INDEX(name, count, values) {LOG_BASE2 ((uint)CountOfField (CodedIndexMap_t, name)), (uint)CountOfField(CodedIndexMap_t, name), (uint)offsetof(CodedIndexMap_t, name) },
CODED_INDICES
#undef CODED_INDEX
}};

#undef CodedIndex
#define CodedIndex(name) CodedIndex_ ## name

struct MetadataToken
{
    uint table : 8;
    uint index : 24;
};

struct MetadataTablesHeader // tilde stream
{
    uintLE reserved;        // 0
    uint8 MajorVersion;
    uint8 MinorVersion;
    uint8 HeapSizes;
    uint8 reserved2;        // 1
    uintLE64 Valid;         // metadata_typedef etc.
    uintLE64 Sorted;        // metadata_typedef etc.
    // uintLE NumberOfRows [];
};

struct MetadataRoot
{
    enum { SIGNATURE = 0x424A5342 };
    /* 0 */ uintLE signature;
    /* 4 */ uintLE16 MajorVersion; // 1, ignore
    /* 6 */ uintLE16 MinorVersion; // 1, ignore
    /* 8 */ uintLE Reserved;     // 0
    /* 12 */ uintLE VersionLength; // VersionLength null, round up to 4
    /* 16 */ char Version [1];
    // uintLE16 Flags; // 0
    // uintLE16 NumberOfStreams;
    // MetadataStreamHeader stream_headers [NumberOfStreams];
};

struct MetadataStreamHeader // see mono verify_metadata_header
{
    uintLE offset;
    uintLE Size; // multiple of 4
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

BEGIN_ENUM(AssemblyFlags, uint)
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
END_ENUM(AssemblyFlags, uint)

BEGIN_ENUM(FileFlags, uint)
{
    FileFlags_ContainsMetaData      =   0x0000,     // This is not a resource file
    FileFlags_ContainsNoMetaData    =   0x0001,     // This is a resource file or other non-metadata-containing file
}
END_ENUM(FileFlags, uint)

BEGIN_ENUM(ManifestResourceFlags, uint)
{
    ManifestResourceFlags_VisibilityMask        =   0x0007,
    ManifestResourceFlags_Public                =   0x0001,     // The Resource is exported from the Assembly.
    ManifestResourceFlags_Private               =   0x0002,     // The Resource is private to the Assembly.
}
END_ENUM(ManifestResourceFlags, uint)

BEGIN_ENUM(GenericParamFlags, uint16)
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
END_ENUM(GenericParamFlags, uint16)

#if 0 // TODO This is copy/pasted from the web and should be gradually
      // worked into types/defines/enums, or deleted because it already was. And cross check with ECMA .pdf.

12 - CustomAttribute Table

//I think the best description is given by the SDK: "The CustomAttribute table stores data that can be used to instantiate a
// Custom Attribute (more precisely, an object of the specified Custom Attribute class) at runtime. The field called Type
//is slightly misleading - it actually indexes a constructor method - the owner of that constructor method is the Type of the Custom Attribute."

// Fields:

// - Parent (index into any metadata table, except the CustomAttribute table itself; more precisely, a HasCustomAttribute coded index)
// - Type (index into the MethodDef or MethodRef table; more precisely, a CustomAttributeType coded index)
// - Value (index into Blob heap)

20 - Event Table

Each row represents an event.

Fields:

- EventFlags (a 2-byte bitmask of type EventAttribute)
- Name (index into String_t heap)
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

Fields:

- Flags (a 2-byte bitmask of type PropertyAttributes)
- Name (index into String_t heap)
- Type (index into Blob heap) [the name of this field is misleading. It does not index a TypeDef or TypeRef table – instead it indexes the signature in the Blob heap of the Property)

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

26 - ModuleRef Table

Each row represents a reference to an external module.

Fields:

- Name (index into String_t heap)

27 - TypeSpec Table

Each row represents a specification for a TypeDef or TypeRef. The only field indexes a token in the #Blob stream.

Fields:

- Signature (index into the Blob heap)

28 - ImplMap Table

// I quote: "The ImplMap table holds information about unmanaged methods that can be reached from managed code,
// using PInvoke dispatch. Each row of the ImplMap table associates a row in the MethodDef table (MemberForwarded)
//  with the name of a routine (ImportName) in some unmanaged DLL (ImportScope).". This means all the unmanaged functions used by the assembly are listed here.

// Fields:

// - MappingFlags (a 2-byte bitmask of type PInvokeAttributes)
// - MemberForwarded (index into the Field or MethodDef table; more precisely, a MemberForwarded coded index.
//  However, it only ever indexes the MethodDef table, since Field export is not supported)
// - ImportName (index into the String_t heap)
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

Fields:

- RVA (a 4-byte constant)
- Field (index into Field table)

32 - Assembly Table

//It's a one-row table. It stores information about the current assembly.

Fields:

- HashAlgId (a 4-byte constant of type AssemblyHashAlgorithm)
- MajorVersion, MinorVersion, BuildNumber, RevisionNumber (2-byte constants)
- Flags (a 4-byte bitmask of type AssemblyFlags)
- PublicKey (index into Blob heap)
- Name (index into String_t heap)
- Culture (index into String_t heap)

Available flags are:

The PublicKey is != 0, only if the StrongName Signature is present and the afPublicKey flag is set.

35 - AssemblyRef Table

Each row references an external assembly.

Fields:

- MajorVersion, MinorVersion, BuildNumber, RevisionNumber (2-byte constants)
- Flags (a 4-byte bitmask of type AssemblyFlags)
- PublicKeyOrToken (index into Blob heap – the public key or token that identifies the author of this Assembly)
- Name (index into String_t heap)
- Culture (index into String_t heap)
- HashValue (index into Blob heap)

The flags are the same ones of the Assembly table.

36 - AssemblyRefProcessor Table

//This table is ignored by the CLI and should not be present in an assembly.

Fields:

- Processor (4-byte constant)
- AssemblyRef (index into the AssemblyRef table)

37 - AssemblyRefOS Table

//This table is ignored by the CLI and should not be present in an assembly.

Fields:

- OSPlatformId (4-byte constant)
- OSMajorVersion (4-byte constant)
- OSMinorVersion (4-byte constant)
- AssemblyRef (index into the AssemblyRef table)

38 - File Table

Each row references an external file.

Fields:

- Flags (a 4-byte bitmask of type FileAttributes)
- Name (index into String_t heap)
- HashValue (index into Blob heap)

Available flags are:


39 - ExportedType Table

//I quote: "The ExportedType table holds a row for each type, defined within other modules of this Assembly,
//that is exported out of this Assembly. In essence, it stores TypeDef row numbers of all types that are marked
//public in other modules that this Assembly comprises.". Be careful, this does not mean that when an assembly
// uses a class contained in my assembly I export that type. In fact, I have not seen yet this table in an assembly.

Fields:

- Flags (a 4-byte bitmask of type TypeAttributes)
- TypeDefId (4-byte index into a TypeDef table of another module in this Assembly). This field is used as a
// hint only. If the entry in the target TypeDef table matches the TypeName and TypeNamespace entries in
//this table, resolution has succeeded. But if there is a mismatch, the CLI shall fall back to a search of the target TypeDef table
- TypeName (index into the String_t heap)
- TypeNamespace (index into the String_t heap)
- Implementation. This can be an index (more precisely, an Implementation coded index) into one of 2 tables, as follows:
        o File table, where that entry says which module in the current assembly holds the TypeDef
        o ExportedType table, where that entry is the enclosing Type of the current nested Type

The flags are the same ones of the TypeDef.

40 - ManifestResource Table

Each row references an internal or external resource.

Fields:

- Offset (a 4-byte constant)
- Flags (a 4-byte bitmask of type ManifestResourceAttributes)
- Name (index into the String_t heap)
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
//number of readers for this resources file, do not worry, it's something we do not have to talk about...
//Framework stuff. Third dword is the size of reader types This number is only good for us to skip
// the string (or strings) that follows, which is something like: "System.Resources.ResourceReader,
//mscorlibsSystem.Resources.RuntimeResourceSet, mscorlib, Version=1.0.5000.0, Culture=neutral, PublicKeyToken=b77a5c561934e089".
//It tells the framework the reader to use for this resources file.

// Ok, now we got to the interesting part. The next dword tells us the version of the resources file
// (existing versions are 1 and 2). After the version, another dword gives the number of actual resources in the
// file. Another dword follows and gives the number of resource types.

//To gather the additional information we need, we have to skip the resource types. For each type
//there's a 7bit encoded integer who gives the size of the string that follows. To decode these kind of
// integers you have to read every byte until you find one which has not the highest bit set and make some
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

Fields:

- Number (the 2-byte index of the generic parameter, numbered left-to-right, from zero)
- Flags (a 2-byte bitmask of type GenericParamAttributes)
- Owner (an index into the TypeDef or MethodDef table, specifying the Type or Method to which this generic parameter applies; more precisely, a TypeOrMethodDef coded index)
- Name (a non-null index into the String_t heap, giving the name for the generic parameter. This is purely descriptive and is used only by source language compilers and by Reflection)

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

The fields needed are, of course, only two

Fields:

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

// No other types than the exception handling sections are declared (this does not mean
// you should not check the CorILMethod_Sect_EHTable flag). So if the section is small it will be:

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
struct ImageClrHeader // data_directory [15]
{
    uintLE cb; // count of bytes
    uintLE16 MajorRuntimeVersion;
    uintLE16 MinorRuntimeVersion;
    DataDirectory_t MetaData;
    uintLE Flags;
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is not set, EntryPointToken represents a managed entrypoint.
    // If COMIMAGE_FLAGS_NATIVE_ENTRYPOINT is set, EntryPointRVA represents an RVA to a native entrypoint.
    union {
        uintLE EntryPointToken;
        uintLE EntryPointRVA;
    };
    DataDirectory_t Resources;
    DataDirectory_t StrongNameSignature;
    DataDirectory_t CodeManagerTable;
    DataDirectory_t VTableFixups;
    DataDirectory_t ExportAddressTableJumps;
    DataDirectory_t ManagedNativeHeader;
};

struct MetadataType;
struct Image;

typedef void (*MetadataType_Read)(const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);
typedef uint (*MetadataType_GetSize)(const MetadataType*, Image*);
typedef void (*MetadataType_Print)(const MetadataType*, Image*);

struct MetadataTypeFunctions // Virtual functions, but allowing for static construction.
{
    void (*Read)(const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);
    uint (*GetSize)(const MetadataType*, Image*);
    void (*Print)(const MetadataType*, Image*);
};

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

struct MetadataType
{
    const char *name;
    MetadataTypeFunctions const * functions;
    union
    {
        uint fixed_size;
        uint table_index;
        CodedIndex coded_index;
   };
};

static
uint
GetCodedIndexSize (Image* image, CodedIndex coded_index);

static
void
MetatadataReadFixed (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    Assert (size);
//    if (size)
        memcpy (mem, file, size); // TODO endian, enum size mismatch
}

static
void
MetatadataReadBlob (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetatadataReadString (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetatadataReadUString (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetadataReadCodedIndex (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetatadataReadIndex (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetatadataReadIndexList (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
void
MetatadataReadGuid (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem);

static
uint
MetadataGetFixedSize (const MetadataType* type, Image* image)
{
    return type->fixed_size; // TODO? change from int8 to uint8? or uint to int?
}

static
uint
MetadataGetBlobSize (const MetadataType* type, Image* image);

static
uint
MetadataGetStringSize (const MetadataType* type, Image* image);

static
uint
MetadataGetUStringSize (const MetadataType* type, Image* image)
{
    return 4u;
}

static
uint
MetadataGetCodedIndexSize (const MetadataType* type, Image* image)
{
    return GetCodedIndexSize (image, type->coded_index);
}

static
uint
ImageGetIndexSize (Image* image, uint /* todo enum */ table_index);

static
uint
MetadataGetIndexSize (const MetadataType* type, Image* image)
{
    return ImageGetIndexSize (image, type->table_index);
}

static
uint
MetadataGetIndexListSize (const MetadataType* type, Image* image)
{
    return MetadataGetIndexSize (type, image);
}

static
uint
MetadataGetGuidSize (const MetadataType* type, Image* image);

const MetadataTypeFunctions MetadataType_Fixed =
{
    MetatadataReadFixed,
    MetadataGetFixedSize,
};

const MetadataTypeFunctions MetadataType_blob_functions =
{
    MetatadataReadBlob,
    MetadataGetBlobSize,
};

const MetadataTypeFunctions MetadataType_string_functions =
{
    MetatadataReadString,
    MetadataGetStringSize,
};

const MetadataTypeFunctions MetadataType_guid_functions =
{
    MetatadataReadGuid,
    MetadataGetGuidSize,
};

const MetadataTypeFunctions MetadataType_ustring_functions =
{
    MetatadataReadUString,
    MetadataGetUStringSize,
};

const MetadataTypeFunctions MetadataType_Index =
{
    MetatadataReadIndex,
    MetadataGetIndexSize,
};

const MetadataTypeFunctions MetadataType_IndexList =
{
    MetatadataReadIndexList,
    MetadataGetIndexSize,
};

const MetadataTypeFunctions MetadataType_CodedIndex =
{
    MetadataReadCodedIndex,
    MetadataGetCodedIndexSize,
};

const MetadataType MetadataType_uint8 = { "uint8", &MetadataType_Fixed, {1} };
const MetadataType MetadataType_uint16 = { "uint16", &MetadataType_Fixed, {2} };
const MetadataType MetadataType_uint = { "uint", &MetadataType_Fixed, {4} };

// heap indices or offsets
const MetadataType MetadataType_blob                  = { "blob",  &MetadataType_blob_functions };
const MetadataType MetadataType_guid                  = { "guid", &MetadataType_guid_functions };
const MetadataType MetadataType_string                = { "string", &MetadataType_string_functions };

// table indices
const MetadataType MetadataType_AssemblyRef           = { "AssemblyRef", &MetadataType_Index, {kAssemblyRef} };
const MetadataType MetadataType_Field                 = { "Field", &MetadataType_Index, {kField} };
const MetadataType MetadataType_GenericParam          = { "GenericParam", &MetadataType_Index, {kGenericParam} };
const MetadataType MetadataType_MethodDef             = { "MethodDef", &MetadataType_Index, {kMethodDef} };
const MetadataType MetadataType_ModuleRef             = { "ModuleRef", &MetadataType_Index, {kModuleRef} };
const MetadataType MetadataType_Property              = { "Property", &MetadataType_Index, {kProperty} };
const MetadataType MetadataType_TypeDef               = { "TypeDef", &MetadataType_Index, {kTypeDef} };

// coded indices
const MetadataType MetadataType_CustomAttributeType   = { "CustomAttributeType", &MetadataType_CodedIndex, { CodedIndex(CustomAttributeType)} };
const MetadataType MetadataType_HasConstant           = { "HasConstant", &MetadataType_CodedIndex, { CodedIndex(HasConstant)} };
const MetadataType MetadataType_HasCustomAttribute    = { "HasCustomAttribute", &MetadataType_CodedIndex, { CodedIndex(HasCustomAttribute)} };
const MetadataType MetadataType_HasDeclSecurity       = { "HasDeclSecurity", &MetadataType_CodedIndex, { CodedIndex(HasDeclSecurity)} };
const MetadataType MetadataType_HasFieldMarshal       = { "HasFieldMarshal", &MetadataType_CodedIndex, { CodedIndex(HasFieldMarshal)} };
const MetadataType MetadataType_HasSemantics          = {" HasSemantics", &MetadataType_CodedIndex, { CodedIndex(HasSemantics)} };
const MetadataType MetadataType_Implementation        = { "Implementation", &MetadataType_CodedIndex, { CodedIndex(Implementation)} };
const MetadataType MetadataType_MemberForwarded       = { "MemberForwarded", &MetadataType_CodedIndex, { CodedIndex(MemberForwarded)} };
const MetadataType MetadataType_MemberRefParent       = { "MemberRefParent", &MetadataType_CodedIndex, { CodedIndex(MemberRefParent)} };
const MetadataType MetadataType_MethodDefOrRef        = { "MethodDefOrRef", &MetadataType_CodedIndex, { CodedIndex(MethodDefOrRef)} };
const MetadataType MetadataType_ResolutionScope       = { "ResolutionScope", &MetadataType_CodedIndex, { CodedIndex(ResolutionScope)} };
const MetadataType MetadataType_TypeDefOrRef          = { "TypeDefOrRef", &MetadataType_CodedIndex, { CodedIndex(TypeDefOrRef)} };
const MetadataType MetadataType_TypeOrMethodDef       = { "TypeOrMethodDef", &MetadataType_CodedIndex, { CodedIndex(TypeOrMethodDef)} };

// Lists go to end of table, or start of next list, referenced from next element of same table
const MetadataType MetadataType_EventList             = { "EventList", &MetadataType_IndexList, {kEvent} };
const MetadataType MetadataType_FieldList             = { "FieldList", &MetadataType_IndexList, {kField} };
const MetadataType MetadataType_MethodList            = { "MethodList", &MetadataType_IndexList, {kMethodDef} };
const MetadataType MetadataType_ParamList             = { "ParamList", &MetadataType_IndexList, {kParam} };
const MetadataType MetadataType_PropertyList          = { "PropertyList", &MetadataType_IndexList, {kProperty} };
const MetadataType MetadataType_Unused                = { "Unused" /* TODO runtime error */ };
const MetadataType MetadataType_NotStored             = { "NotStored", &MetadataType_Fixed };

// enums/flags
#define MetadataType_Interface          MetadataType_TypeDefOrRef /* or spec, TODO? creater? */
#define MetadataType_signature          MetadataType_blob /* TODO? decode and maybe creater */
#define MetadataType_Name               MetadataType_string
#define MetadataType_Extends            MetadataType_TypeDefOrRef
#define MetadataType_TypeName           MetadataType_string
#define MetadataType_TypeNameSpace      MetadataType_string
#define MetadataType_Unused             MetadataType_Unused

struct MetaTableStaticField
{
    const char* name;
    const MetadataType* type; // old compilers do not allow reference here, with initialization
    uint mem_offset;
};

struct MetadataTableStatic_t
{
    const char *name;
    uint mem_row_size;
    uint field_count;
    const MetaTableStaticField* fields;
};

#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) struct name ## _t; struct name ## Table;
#include __FILE__ // METADATA_TABLES

#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_TABLE(name, xbase, fields)                             \
    struct name ## _t                                                   \
    {                                                                   \
            MetadataRow base { &name ## Functions };                    \
            /*virtual ~name ## _t ( ) { }*/                             \
            /*name ##  _t ( ) { }*/                                     \
            /*name ##  _t (const name ##  _t&) = default;*/             \
            fields                                                      \
    };                                                                  \
    struct name ## Table : vector<name ## _t>, IMetadataTable           \
    {                                                                   \
        virtual void* iat(size_t n)                                     \
        {                                                               \
            return &at(n);                                              \
        }                                                               \
        virtual void* iresize(size_t n)                                 \
        {                                                               \
            resize(n, { });                                             \
            return n ? &(*this) [0] : 0;                                \
        }                                                               \
    };
#define METADATA_FIELD2(table, name, type) metadata_schema_TYPED_ ## type name = { };
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type) pointerful_type name = { };
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
#include __FILE__ // METADATA_TABLES

#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_TABLE(name, base, fields) const MetaTableStaticField metadata_field_ ## name [ ] = { fields };
#define METADATA_FIELD2(table, name, type)                             { # name, &MetadataType_  ## type, offsetof (table ## _t, name) },
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type) { # name, &MetadataType_  ## persistant_type, offsetof (table ## _t, name) },
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
#include __FILE__ // METADATA_TABLES

struct MetadataFieldDynamic
{
    MetadataFieldDynamic()
    {
        memset (this, 0, sizeof (*this));
    }

    const char* debug_name;
    uint size;
    uint offset;
};

struct MetadataTableDynamic
{
    //MetadataTableDynamic() { memset (this, 0, sizeof (*this)); }

    const char* debug_name;
    void* file_base;
    void* mem_base; // from iresize()
    MetadataFieldDynamic* fields;
    uint row_count;
    uint file_row_size; // vs. mem_row_size which is elsewhere
    uint index_size; // 2 or 4
    bool present;
    int8_t name_field;
    bool name_field_valid;
};

struct MetadataDynamicZero // dynamic, ZeroMemory compatible part
{
    struct
    {
        union {
            MetadataTableDynamic array [
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) 1+
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) 1+
#include __FILE__ // METADATA_TABLES
                0];

            struct
            {
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) MetadataTableDynamic unused_ ## name;
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) MetadataTableDynamic name;
#include __FILE__ // METADATA_TABLES
            } name;
        };
    } file;

    MetadataFieldDynamic fields [
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_FIELD2(table, name, type)                                1+
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type)    1+
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) /* nothing */
#define METADATA_TABLE(name, base, fields) fields
#include __FILE__ // METADATA_TABLES
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
    0];

#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name)
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) name ## Table name;
#include __FILE__ // METADATA_TABLES

    IMetadataTable* itables [
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) 1+
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) 1+
#include __FILE__ // METADATA_TABLES
        0];

    MetadataDynamicZero()
    {
        memset (this, 0, sizeof (*this));
    }
};

struct MetadataDynamic : MetadataDynamicZero
{
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name)
#undef METADATA_TABLE
#define METADATA_TABLE(name, base, fields) name ## Table name; // has vtables, do not memset(0)
#include __FILE__ // METADATA_TABLES

    std::vector<MetadataRow*> all_rows;

    struct UnusedTable : IMetadataTable
    {
    } unused_table;

    MetadataDynamic()
    {
        MetadataFieldDynamic* field = fields;
        MetadataTableDynamic* table = file.array;
        uint i = 0;
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_FIELD2(table, name, type)                                1+
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type)    1+
#define METADATA_TABLE(name, base, fieldz) table->debug_name = #name; itables [i++] = &name; table->fields = field; field += fieldz 0; ++table;
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) ++table; itables [i++] = &unused_table;
#include __FILE__ // METADATA_TABLES
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3

        field = fields;
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_FIELD2(table, name, type)                                   (field++)->debug_name = #name;
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type)       (field++)->debug_name = #name;
#define METADATA_TABLE(name, base, fieldz)                                   fieldz
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name)
#include __FILE__ // METADATA_TABLES
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
    }
};
#undef METADATA_TABLE

const char * const MetadataTableName [ ] =
{
#undef METADATA_TABLE
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_TABLE(name, base, fields) #name,
#undef METADATA_TABLE_UNUSED
#define METADATA_TABLE_UNUSED(name) "unused_" #name,
#include __FILE__ // METADATA_TABLES
#undef METADATA_TABLE
};
#define MetadataTableName(x) MetadataTableName [x]

#undef METADATA_TABLE
#undef METADATA_TABLE_UNUSED
#undef METADATA_FIELD2
#undef METADATA_FIELD3
#define METADATA_FIELD2(table, name, type)                                /* nothing */
#define METADATA_FIELD3(table, name, persistant_type, pointerful_type)    /* nothing */
#define METADATA_TABLE(name, base, fields) { #name, sizeof (name ## _t), (uint)CountOf (metadata_field_ ## name), metadata_field_ ## name },
#define METADATA_TABLE_UNUSED(name)        { 0 },

const MetadataTableStatic_t MetadataStatic [ ] =
{
#include __FILE__ // METADATA_TABLES
};

void Type_SetCName_Common (std::string& cname, String_t TypeNameSpace, String_t TypeName)
{
    const char* dot = (TypeNameSpace.length && TypeName.length) ? "_" : "";
    cname.reserve (TypeNameSpace.length + TypeName.length + !!*dot);
    cname = std::string (TypeNameSpace.chars, TypeNameSpace.length);
    cname += dot;
    cname += std::string (TypeName.chars, TypeName.length);
    for (auto& c: cname)
    {
        if (c == '.' || c == '`')
            c = '_';
    }
}

void TypeDef_SetCName (TypeDef_t* self)
{
    Type_SetCName_Common (self->base.cname, self->TypeNameSpace, self->TypeName);
}

void TypeRef_SetCName (TypeRef_t* self)
{
    Type_SetCName_Common (self->base.cname, self->TypeNameSpace, self->TypeName);
    printf("TypeRef\n");
    __debugbreak();
}

void TypeSpec_SetCName (TypeSpec_t* self)
{
    printf("TypeSpec\n");
    // TODO __debugbreak();
}

struct ImageZero // zero-inited part of Image
{
    ImageZero() { memset (this, 0, sizeof (*this)); }

    uint coded_index_size [CodedIndex_Count]; // 2 or 4
    uint64 file_size; // TODO Use this more.
    struct
    {
        MetadataStreamHeader* guid;
        MetadataStreamHeader* string; // utf8
        MetadataStreamHeader* ustring; // unicode/user strings
        MetadataStreamHeader* blob;
    } streams;
    MetadataRoot* metadata_root;
    void* base;
    NtHeaders* nt;
    uint number_of_sections;
    uint blob_size; // 2 or 4
    uint string_size; // 2 or 4
    uint guid_size; // 2 or 4
};

struct Image : ImageZero
{
    MemoryMappedFile mmf;
    MetadataDynamic metadata;

    char* GetString (uint a)
    {
        Assert (streams.string);
        Assert (a <= streams.string->Size);
        return streams.string->offset + a + (char*)metadata_root;
    }

    char* GetUString (uint a)
    {
        Assert (streams.string);
        Assert (a <= streams.string->Size);
        return streams.ustring->offset + a + (char*)metadata_root;
    }

    Guid* GetGuid (uint a)
    {
        Assert (streams.guid);
        Assert (a * 16 <= streams.guid->Size);
        return a + (Guid*)((uint)streams.guid->offset + (char*)metadata_root);
    }

    void* GetBlob (uint a)
    {
        Assert (streams.blob);
        Assert (a <= streams.blob->Size);
        return a + (char*)((uint)streams.blob->offset + (char*)metadata_root);
    }

    void ComputeFileRowSize (uint table_index)
    {
        MetadataTableStatic_t const * const schema = &MetadataStatic [table_index];
        uint const field_count = schema->field_count;
        if (!field_count)
            return;
        MetadataTableDynamic * const table = &metadata.file.array [table_index];
        uint size = 0;
        for (uint i = 0; i < field_count; ++i)
        {
            MetaTableStaticField const * const field = &schema->fields [i];
            uint field_size = field->type->functions->GetSize (field->type, this);
            table->fields [i].offset = size;
            table->fields [i].size = field_size;
            size += field_size;
            if (!table->name_field_valid)
            {
                if (strcmp(field->name, "TypeName") == 0 || strcmp(field->name, "Name") == 0)
                {
                    table->name_field = (int8)i; // TODO
                    table->name_field_valid = true;
                }
                else if (strcmp(field->name, "name") == 0)
                {
                    abort();
                }
            }
        }
        printf ("ComputeFileRowSize(%s):%X\n", MetadataTableName (table_index), size);
        table->file_row_size = size;
    }

    uint GetFileRowSize (uint table_index)
    {
        return metadata.file.array [table_index].file_row_size;
    }

    static
    uint GetMemRowSize (uint table_index)
    {
        return MetadataStatic [table_index].mem_row_size;
    }

    void
    ComputeIndexSize (uint /* todo enum */ table_index)
    {
        uint const row_count = metadata.file.array [table_index].row_count;
        uint result = (row_count <= 0xFFFFu) ? 2u : 4u;
        metadata.file.array [table_index].index_size = result;
    }

    void
    ComputeCodedIndexSize (CodedIndex coded_index)
    {
        const CodedIndex_t* data = &CodedIndices.array [coded_index];
        uint const map = data->map;
        uint const count = data->count;
        int8* Map = (int8*)&CodedIndexMap;
        uint result = 2;
        uint cap = 0xFFFFu >> data->tag_size;
        for (uint i = 0; i < count; ++i)
        {
            int m = Map [map + i];
            if (m < 0)
                continue;
            uint const rows = metadata.file.array [m].row_count;
            if (rows > cap)
            {
                result = 4;
                break;
            }
        }
        coded_index_size [coded_index] = result;
    }

    void read (const char *file_name)
    {
        mmf.read (file_name);
        base = mmf.base;
        file_size = mmf.file.get_file_size ();
        void * end = file_size + (char*)base;
        DosHeader* const dos = (DosHeader*)base;
        printf ("mz: %02x%02x\n", ((uint8*)dos) [0], ((uint8*)dos) [1]);
        if (memcmp (base, "MZ", 2))
            ThrowString (StringFormat ("incorrect MZ signature %s", file_name));
        printf ("mz: %c%c\n", ((char*)dos) [0], ((char*)dos) [1]);
        uint pe_offset = dos->GetPE ();
        printf ("pe_offset: %#x\n", pe_offset);
        uint8* pe = (pe_offset + (uint8*)base);
        printf ("pe: %02x%02x%02x%02x\n", pe [0], pe [1], pe [2], pe [3]);
        if (memcmp (pe, "PE\0\0", 4))
            ThrowString (StringFormat ("incorrect PE00 signature %s", file_name));
        printf ("pe: %c%c\\0x%08X\\0x%08X\n", pe [0], pe [1], pe [2], pe [3]);
        nt = (NtHeaders*)pe;
        printf ("Machine:0x%08X\n", (uint)nt->file_header.Machine);
        printf ("NumberOfSections:0x%08X\n", (uint)nt->file_header.NumberOfSections);
        printf ("TimeDateStamp:0x%08X\n", (uint)nt->file_header.TimeDateStamp);
        printf ("PointerToSymbolTable:0x%08X\n", (uint)nt->file_header.PointerToSymbolTable);
        printf ("NumberOfSymbols:0x%08X\n", (uint)nt->file_header.NumberOfSymbols);
        printf ("SizeOfOptionalHeader:0x%08X\n", (uint)nt->file_header.SizeOfOptionalHeader);
        printf ("Characteristics:0x%08X\n", (uint)nt->file_header.Characteristics);
        OptionalHeader32* opt32 = (OptionalHeader32*)(&nt->OptionalHeader);
        OptionalHeader64* opt64 = (OptionalHeader64*)(&nt->OptionalHeader);
        uint opt_magic = Unpack(opt32->Magic);
        AssertFormat ((opt_magic == 0x10b && !(opt64 = 0)) || (opt_magic == 0x20b && !(opt32 = 0)), ("file:%s opt_magic:%x", file_name, opt_magic));
        printf ("opt.magic:%x opt32:%p opt64:%p\n", opt_magic, (void*)opt32, (void*)opt64);
        uint NumberOfRvaAndSizes = Unpack(opt32 ? &opt32->NumberOfRvaAndSizes : &opt64->NumberOfRvaAndSizes);
        printf ("opt.rvas:0x%08X\n", NumberOfRvaAndSizes);
        number_of_sections = nt->file_header.NumberOfSections;
        printf ("number_of_sections:0x%08X\n", number_of_sections);
        SectionHeader* section_header = nt->GetFirstSectionHeader ();
        uint i = 0;
        for (i = 0; i < number_of_sections; ++i, ++section_header)
            printf ("section [%02X].Name: %.8s\n", i, section_header->Name);
        DataDirectory_t* dataDirectory = (DataDirectory_t*)(opt32 ? (void*)(opt32 + 1) : opt64 + 1);
        for (i = 0; i < NumberOfRvaAndSizes; ++i)
        {
            printf ("dataDirectory [%s (%02X)].Offset: 0x%08X\n", DataDirectoryName(i), i, (uint)dataDirectory [i].VirtualAddress);
            printf ("dataDirectory [%s (%02X)].Size: 0x%08X\n", DataDirectoryName(i), i, (uint)dataDirectory [i].Size);
        }
        AssertFormat (dataDirectory [14].VirtualAddress, ("Not a .NET image? %x", dataDirectory [14].VirtualAddress));
        AssertFormat (dataDirectory [14].Size, ("Not a .NET image? %x", dataDirectory [14].Size));
        ImageClrHeader* clr = RvaToPointer<ImageClrHeader>(dataDirectory [14].VirtualAddress);
        printf ("clr.cb:0x%08X\n", (uint)clr->cb);
        printf ("clr.MajorRuntimeVersion:0x%08X\n", (uint)clr->MajorRuntimeVersion);
        printf ("clr.MinorRuntimeVersion:0x%08X\n", (uint)clr->MinorRuntimeVersion);
        printf ("clr.MetaData.Offset:0x%08X\n", (uint)clr->MetaData.VirtualAddress);
        printf ("clr.MetaData.Size:0x%08X\n", (uint)clr->MetaData.Size);
        AssertFormat (clr->MetaData.Size, ("0x%08X", clr->MetaData.Size));
        AssertFormat (clr->cb >= sizeof (ImageClrHeader), ("0x%08X 0x%08X", clr->cb, (uint)sizeof (ImageClrHeader)));
        metadata_root = RvaToPointer<MetadataRoot>(clr->MetaData.VirtualAddress);
        printf ("metadata_root_ptr:%p metadata_root_fileofffset:%X\n", metadata_root, (uint)((char*)metadata_root - (char*)base));
        printf ("metadata_root.signature:0x%08X\n", (uint)metadata_root->signature);
        printf ("metadata_root.MajorVersion:0x%08X\n", (uint)metadata_root->MajorVersion);
        printf ("metadata_root.MinorVersion:0x%08X\n", (uint)metadata_root->MinorVersion);
        printf ("metadata_root.Reserved:0x%08X\n", (uint)metadata_root->Reserved);
        printf ("metadata_root.VersionLength:0x%08X\n", (uint)metadata_root->VersionLength);
        AssertFormat ((metadata_root->VersionLength % 4) == 0, ("0x%08X", (uint)metadata_root->VersionLength));
        size_t VersionLength = strlen(metadata_root->Version);
        AssertFormat (VersionLength < metadata_root->VersionLength, ("0x%08X 0x%08X", VersionLength, (uint)metadata_root->VersionLength));
        // TODO bounds checks throughout
        uint16* pflags = (uint16*)&metadata_root->Version [(uint)metadata_root->VersionLength];
        uint16* pnumber_of_streams = 1 + pflags;
        const uint number_of_streams = *pnumber_of_streams;
        printf ("metadata_root.Version:%s\n", metadata_root->Version);
        printf ("flags:0x%08X\n", *pflags);
        printf ("number_of_streams:0x%08X\n", number_of_streams);
        MetadataStreamHeader* stream = (MetadataStreamHeader*)(pnumber_of_streams + 1);
        MetadataStreamHeader* tables = 0;
        for (i = 0; i < number_of_streams; ++i)
        {
            printf ("stream[0x%08X].Offset:0x%08X\n", i, (uint)stream->offset);
            printf ("stream[0x%08X].Size:0x%08X\n", i, (uint)stream->Size);
            AssertFormat ((stream->Size % 4) == 0, ("0x%08X", (uint)stream->Size));
            const char* name = stream->Name;
            size_t length = 0;
            // TODO Bounds check against end of file.
            for (length = 0; length <= 32 && &name [length] < end && name [length]; ++length)
            {
                // nothing
            }
            if (name [length])
            {
                printf("                                       012345678\n");
                AssertFormat (!name [length], ("0x%08X:%s:%c", length, name, name [length]));
            }
            printf ("stream[0x%08X].Name:0x%08X:%.*s\n", i, (int)length, (int)length, name);
            if (length >= 2 && name [0] == '#')
            {
                ++name;
                if (length == 2 && name [0] == '~')
                    tables = stream;
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
            else
            {
unknown_stream:
                fprintf (stderr, "unknown stream %s\n", stream->Name);
                abort ();
            }
            length = (length + 4) & -4;
            stream = (MetadataStreamHeader*)(stream->Name + length);
        }
        MetadataTablesHeader* metadata_tables_header = (MetadataTablesHeader*)((uint)tables->offset + (char*)metadata_root);
        printf ("metadata_tables_header.reserved:0x%08X\n", (uint)metadata_tables_header->reserved);
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
        printf ("metadata_tables_header.        Valid:0x%08X`0x%08X\n", (uint)(valid >> 32), (uint)valid);
        // Mono does not use sorted, and there are bits set beyond Valid.
        // I suspect this has to do with writing/appending, and occasionally sorting.
        printf ("metadata_tables_header.       Sorted:0x%08X`0x%08X\n", (uint)(sorted >> 32), (uint)sorted);
        printf ("metadata_tables_header.     Unsorted:0x%08X`0x%08X\n", (uint)(unsorted >> 32), (uint)unsorted);
        printf ("metadata_tables_header.InvalidSorted:0x%08X`0x%08X\n", (uint)(invalidSorted >> 32), (uint)invalidSorted);
        uint64 mask = 1;
        uint all_row_count = 0;
        uint* prow_count = (uint*)(metadata_tables_header + 1);

        // Presence and row counts.
        for (mask = 1, i = 0; i < CountOf (metadata.file.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (!present)
            {
                printf ("%X(%s) not present\n", i, MetadataTableName (i));
                continue;
            }
            metadata.file.array [i].present = true;
            const uint row_count = *prow_count++;
            metadata.file.array [i].row_count = row_count;
            all_row_count += row_count;
        }

        metadata.all_rows.reserve (all_row_count);

        for (mask = 1, i = 0; i < CountOf (metadata.file.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (!present)
                continue;
            printf ("%X(%s).row_count:%X\n", i, MetadataTableName (i), metadata.file.array [i].row_count);
        }

        // After row counts, coded index size and index size.
        for (i = 0; i < CountOf (metadata.file.array); ++i)
            ComputeIndexSize (i);

        for (i = 0; i < CodedIndex_Count; ++i)
            ComputeCodedIndexSize ((CodedIndex)i);

        // After coded index size, row sizes and table bases.
        char* table_file_base = (char*)prow_count;

        for (mask = 1, i = 0; i < CountOf (metadata.file.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (!present)
                continue;
            ComputeFileRowSize (i);
            metadata.file.array [i].file_base = table_file_base;
            printf ("%s at p:%p metadata_offset:%X file_offset:%X\n", MetadataTableName(i), table_file_base, (uint)((char*)table_file_base - (char*)metadata_root), (uint)((char*)table_file_base - (char*)base));
            table_file_base += metadata.file.array [i].file_row_size * metadata.file.array [i].row_count;
        }

        // TODO running should not do this, but dumping/converting probably ok.
        // TODO A convenient efficient form, pick both? Here we are going for convenient but inefficient.
        //
        // Allocate room for every row to point to file metadatata and have nicely constructed metadata.
        // Or maybe to read/reify it all.
        for (i = 0; i < CountOf (metadata.file.array); ++i)
            metadata.file.array [i].mem_base = metadata.itables [i]->iresize(metadata.file.array [i].row_count);

        for (i = 0; i < CountOf (metadata.file.array); ++i)
        {
            printf ("reading %s\n", MetadataTableName (i));
            MetadataTableDynamic * const dynamic_table = &metadata.file.array [i];
            char* mem = (char*)dynamic_table->mem_base;
            if (!mem)
                continue;
            uint row_count = dynamic_table->row_count;
            char* file = (char*)dynamic_table->file_base;
            MetadataTableStatic_t const * const schema = &MetadataStatic [i];
            uint const field_count = schema->field_count;
            //
            // In-memoy rows can have padding and in-file do not, so
            // add file field at a time and memory row at a time.
            //
            for (uint ri = 0; ri < row_count; ++ri)
            {
                MetaTableStaticField const * static_field = schema->fields;
                MetadataFieldDynamic const * dynamic_field = dynamic_table->fields;
                metadata.all_rows.push_back ((MetadataRow*)mem);
                for (uint fi = 0; fi < field_count; ++fi, ++static_field, ++dynamic_field)
                {
                    uint const dynamic_size = dynamic_field->size;
                    if (dynamic_size)
                    {
                        MetadataType const * const type = static_field->type;
                        MetadataType_Read const read = type->functions->Read;
                        if (read)
                            type->functions->Read (type, this, i, ri, fi, dynamic_size, file, mem + static_field->mem_offset);
                        file += dynamic_field->size;
                    }
                }
                mem += schema->mem_row_size;
            }
        }

        for (mask = 1, i = 0; i < CountOf (metadata.file.array); ++i, mask <<= 1)
        {
            const bool present = (valid & mask) != 0;
            if (!present)
                continue;
            fprintf (stderr, "table 0x%08X (%s) has 0x%08X rows (%s)\n", i, MetadataTableName (i), metadata.file.array [i].row_count, (sorted & mask) ? "sorted" : "unsorted");
        }

        for (MetadataRow* a: metadata.all_rows)
            if (a->Functions->SetCName)
                a->Functions->SetCName (a);

        for (TypeDef_t& t: metadata.TypeDef)
        {
            //printf("// type { 0x%X, %s%s%s .. }\n", t.Flags, t.TypeNameSpace.chars, dot, t.TypeName.chars);
            //printf("extends:");
            // TODO flags? enum?
            printf("type %s /*flags:%X*/ ", t.base.cname.c_str(), t.Flags);
            if (t.Extends)
                printf(": %s\n", t.Extends->cname.c_str ()); // output C++ or C? For exceptions, C++. Otherwise?
            printf("{\n};\n");
        }
    }

    template <typename T>
    T* RvaToPointer (uint rva)
    {
        rva = RvaToFileOffset (rva);
        return rva ? reinterpret_cast<T*>(((char*)base) + rva) : 0;
    }

    uint RvaToFileOffset (uint rva)
    {
        // TODO binary search and/or cache
        SectionHeader* section_header = nt->GetFirstSectionHeader ();
        for (uint i = 0; i < number_of_sections; ++i, ++section_header)
        {
            uint va = section_header->VirtualAddress;
            if (rva >= va && rva < (va + section_header->SizeOfRawData))
                return section_header->PointerToRawData + (rva - va);
        }
        return 0;
    }
};

static
Blob_t
MetatadataDecodeBlob (uint8* data)
{
    uint8* p = data;
    uint size = *p++;
    if ((size & 0x80) == 0)
        ; // nothing
    else if ((size & 0xC0) == 0x80) // TODO check against file bounds (reading data and computing size)
        size = ((size & 0x3F) << 8) | *p++;
    else if ((size & 0xE0) == 0xE0) // TODO check against file bounds (reading data and computing size)
    {
        size = (((size & 0x1F) << 8) | *p++);
        size = ((size << 8) | *p++);
        size = ((size << 8) | *p++);
    }
    else
        AssertFailed("invalid metadata (blob)");

    Blob_t blob;
    blob.size = size; // TODO range check
    blob.data = p;
    return blob;
}

void
MetatadataReadBlob (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    uint const offset = Unpack (file, size);
    Blob_t* const blob = (Blob_t*)mem;
    void* const m = image->GetBlob (offset);
    *blob = MetatadataDecodeBlob ((uint8*)m);
}

static
void
MetatadataReadString (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    uint const offset = Unpack (file, size);
    char * const s = image->GetString(offset);
    String_t* const st = (String_t*)mem;
    st->length = strlen (s); // TODO range check (do not call strlen)
    st->chars = s;
}

static
void
MetatadataReadUString (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    uint offset = Unpack (file, size);
    Blob_t blob = MetatadataDecodeBlob ((uint8*)image->GetUString(offset));
    UString_t* us = (UString_t*)mem;
    us->length = (blob.size - 1) >> 1;
    us->ascii = ((char16_t*)blob.data) [blob.size - 1] == 0;
    us->chars = (char16_t*)blob.data;
}

static
void
MetadataReadIndexCommon (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem,
                         uint index, uint table_index)
{
    if (!index)
        return;
    --index;

    MetadataTableDynamic const * const reffed_table = &image->metadata.file.array [table_index];
    Assert (index <= reffed_table->row_count);
    *(MetadataRow**)mem = (MetadataRow*)image->metadata.itables [table_index]->iat(index);
}

static
void
MetadataReadCodedIndex (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    const uint code = Unpack (file, size);
    CodedIndex_t const * const coded_index = &CodedIndices.array [type->coded_index];

    const uint index = (code >> coded_index->tag_size);
    const int table_index = ((int8*)&CodedIndexMap) [coded_index->map + (code & ~(~0u << coded_index->tag_size))]; // TODO precompute
    Assert (table_index >= 0);

    MetadataReadIndexCommon (type, image, table, row, field, size, file, mem, index, (uint)table_index);
}

static
void
MetatadataReadIndex (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    MetadataReadIndexCommon (type, image, table, row, field, size, file, mem, Unpack (file, size), type->table_index);
}

static
void
MetatadataReadIndexList (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    // The size of an index list is either until the end of the referenced table, or the next element in the referring table.
    // The in-memory form of an index list a vector of pointers to rows of a metadata table.
    // There are no coded index lists, just index lists. Like other indices, these are 2 or 4 bytes.
    // The table they refer to is recorded in type.
    uint start = Unpack (file, size);
    if (!start)
        return;
    --start;
    uint count = 0;
    uint const row_count = image->metadata.file.array [table].row_count;
    uint const file_row_size = image->metadata.file.array [table].file_row_size;
    vector<void*>* const mem_typed = (vector<void*>*)mem;
    MetadataTableDynamic const * const reffed_table = &image->metadata.file.array [type->table_index];
    uint const mem_row_size = MetadataStatic [type->table_index].mem_row_size;
    uint const reffed_row_count = reffed_table->row_count;
    Assert (start <= reffed_row_count);

    if (row + 1 == row_count)
    {
        count = reffed_row_count - start + 1;
    }
    else
    {
        uint const next = Unpack (file_row_size + (char*)file, size) - 1;
        Assert (next <= reffed_row_count);
        count = next - start;
    }

    char* ref = (char*)reffed_table->mem_base + mem_row_size * start;
    mem_typed->resize (count, { });
    for (uint i = 0; i < count; ++i)
    {
        (*mem_typed) [i] = ref;
        ref += mem_row_size;
    }
}

static
void
MetatadataReadGuid (const MetadataType* type, Image* image, uint table, uint row, uint field, uint size, const void* file, void* mem)
{
    *(const void**)mem = file;
}

#define GUID_FORMAT "{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}"
#define GUID_BYTES(g) \
    (g)->bytes [ 3],  \
    (g)->bytes [ 2],  \
    (g)->bytes [ 1],  \
    (g)->bytes [ 0],  \
    (g)->bytes [ 5],  \
    (g)->bytes [ 4],  \
    (g)->bytes [ 7],  \
    (g)->bytes [ 6],  \
    (g)->bytes [ 8],  \
    (g)->bytes [ 9],  \
    (g)->bytes [10],  \
    (g)->bytes [11],  \
    (g)->bytes [12],  \
    (g)->bytes [13],  \
    (g)->bytes [14],  \
    (g)->bytes [15]   \

static
uint
MetadataGetBlobSize (const MetadataType* type, Image* image)
{
    return image->blob_size;
}

static
uint
MetadataGetStringSize (const MetadataType* type, Image* image)
{
    return image->string_size;
}

static
uint
MetadataGetGuidSize (const MetadataType* type, Image* image)
{
    return image->guid_size;
}

static
uint
GetCodedIndexSize (Image* image, CodedIndex coded_index)
{
    return image->coded_index_size [coded_index];
}

static
uint
ImageGetIndexSize (Image* image, uint /* todo enum */ table_index)
{
    uint a = image->metadata.file.array [table_index].index_size;
    Assert (a == 2 || a == 4);
    return a;
}

}

using namespace m2;

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
    Image im;
#define X(x) printf ("%s %#x\n", #x, (int)x)
X (sizeof (DosHeader));
X (sizeof (FileHeader));
X (sizeof (NtHeaders));
X (sizeof (SectionHeader));
X (CodedIndices.array [CodedIndex(TypeDefOrRef)].tag_size);
X (CodedIndices.array [CodedIndex(ResolutionScope)].tag_size);
X (CodedIndices.array [CodedIndex(HasConstant)].tag_size);
X (CodedIndices.array [CodedIndex(HasCustomAttribute)].tag_size);
X (CodedIndices.array [CodedIndex(HasFieldMarshal)].tag_size);
X (CodedIndices.array [CodedIndex(HasDeclSecurity)].tag_size);
#undef X
#if 1
    try
#endif
    {
        im.read (argv [1]);
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

#endif // METADATA_TABLE
