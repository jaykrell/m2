// This does not really work.
// Visual C++ 2.0 does not work.
// Visual C++ 5.0 is dodgy.
// Stick to 2008 or newer. Subject to change. Probably will embrace C++11 or newer.


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
	void resize(size_type _N, const T& x = T()) // 103

There are two problems.
Workaround either by not using namespaces or having local vector without that default construction.
Adding a constructor from int, and then a default constructor helps, but does not fix the entire problem.


Visual C++ 2.0 shipped without STL.

Visual C++ 4.0?
*/
#if !defined(_MSC_VER) || _MSC_VER >= 1100 // TODO check more versions; 4.0?
// TODO msvc40 does have STL but I missed it in install.
#include <string>
#include <vector>
//#include <algorithm> // TODO? remove STL dependency?
using namespace std;

#else

inline void
OutOfMemory()
{
    abort();
}

inline void* operator new (size_t n, void * a)
{
    return a;
}

template <class T>
struct vector
{
    T* a;
    T* b;
    T* c;

    T* begin() { return a; }
    T* end() { return b; }

    vector() : a(0), b(0), c(0) { }

    size_t size() const { return b - a; }
    size_t capacity() const { return c - a; }

    void reserve(size_t n);

    static void swap (T*&a, T*&b)
    {
        T*t = a;
        a = b;
        b = t;
    }

    vector& operator=(const vector& d)
    {
        // TODO
        size_t n = d.size();
        vector temp(n);
        for (size_t i = 0; i < n; ++i)
            new (&temp.a[i]) T(d.a[i]);
        swap(a, temp.a);
        swap(b, temp.b);
        swap(c, temp.c);
        return *this;
    }

    vector(const vector& d) :a(0), b(0), c(0)
    {
        operator=(d);
    }

    // TODO do not repeat self
    T& operator[](size_t n) { return a[n]; }
    const T& operator[](size_t n) const { return a[n]; }

    void resize(size_t n)
    {
        // TODO doubling
        size_t s = size();
        if (s == n)
            return;
        if (s > n)
        {
            // Destroy tail and reset end.
            for (size_t i = n; i < s; ++i)
                (&a[i])->~T();
            b = a + n;
            return;
        }
        T* newa = (T*)calloc(n, sizeof(T));
        size_t i;
        for (i = 0; i < s; ++i)
            new (&newa[i]) T(a[i]); // TODO move
        for (i = 0; i < s; ++i)
            (&a[i])->~T(); // TODO move
        for (i = s; i < n; ++i)
            new (&newa[i]) T();
        free(a);
        a = newa;
        b = a + n;
        c = a + n;
    }

    ~vector()
    {
        // TODO
    }

    vector(size_t n) : a(0), b(0), c(0)
    {
        a = (T*)calloc(n, sizeof(T));
        if (!a)
            OutOfMemory();
        b = a + n;
        c = a + n;
        for (size_t i = 0; i < n; ++i)
            new (&a[i]) T();
        // TODO
    }
};

template <class T>
struct basic_string
{
    vector<T> a;

    basic_string() { }

    static size_t len(const T* c)
    {
        size_t a = 0;
        while (*c)
        {
            ++c;
            ++a;
        }
        return a;
    }

    basic_string(const T* c)
    {
        operator=(c);
    }

    basic_string& operator+(const T* c)
    {
        // TODO
        return *this;
    }

    void operator=(const T* c)
    {
        size_t n = len(c);
        a.resize(n + 1);
        memcpy(&a[0], c, n * sizeof(T));
        a[n] = (T)0;
    }

    const T* c_str() const
    {
        return &a[0];
    };

    basic_string operator+(const basic_string<T>& e) const
    {
        // TODO
        return basic_string();
    }


};

typedef basic_string<char> string;


template <class T>
basic_string<T> operator+(const T* c, const basic_string<T>& d)
{
    // TODO
    return d;
}

#endif
