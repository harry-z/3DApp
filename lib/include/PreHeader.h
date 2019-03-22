#pragma once

#define ARCHITECTURE_X86 1
#define ARCHITECTURE_X64 2

#if defined(ARCH_64) 
	#define TARGET_ARCHITECTURE	ARCHITECTURE_X64
#else
	#define TARGET_ARCHITECTURE	ARCHITECTURE_X86
#endif

#define PLATFORM_WINDOWS			1

#if defined(PLATFORM_WIN)
	#define TARGET_PLATFORM		PLATFORM_WINDOWS
#else
	#pragma error("Error: Unknown target platform!")
#endif




#define COMPILER_MSVC					0
#define COMPILER_GNUCXX					1

#if defined(_MSC_VER)
	#define COMPILER					COMPILER_MSVC
	#define COMPILER_VERSION			_MSC_VER
#elif defined(_GNUCXX)
	#define COMPILER					COMPILER_GNUCXX
	#define COMPILER_VERSION			COMPILER_GNUCXX
#endif



#pragma warning(disable:4251)
#pragma warning(disable:4996) // CRT_SECURE warning 
#pragma warning(disable:4275)

#if COMPILER == COMPILER_MSVC
	#if defined(EXPORTS)
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif

	#define FORCE_INLINE __forceinline
#else
	// #define DLL_EXPORT
	// #define FORCE_INLINE __attribute__((always_inline))
#endif



#define IN
#define OUT
#define INOUT

#define BIT_ADD(flag, bit) (flag) |= (bit)
#define BIT_REMOVE(flag, bit) (flag) &= (~(bit))
#define BIT_CHECK(flag, bit) ((flag) & (bit)) != 0

#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = 0; }

#if TARGET_ARCHITECTURE == ARCHITECTURE_X86
	#define ALIGN 4
#elif TARGET_ARCHITECTURE == ARCHITECTURE_X64
	#define ALIGN 8
#endif

#if COMPILER == COMPILER_MSVC
	#define ALIGNMENT(n) __declspec(align(n))
#elif COMPILER == COMPILER_GNUCXX
	#define ALIGNMENT(n) __attribute__((aligned(n)))
#endif

#define ALIGN_SIZE(bytes) ((bytes+ALIGN-1) & ~(ALIGN-1))

#define EPSILON 1e-6

#define PI 3.14159265f

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <exception>
#include <new>





//#if REDIRECT_NEW_DELETE == 1
//
//#if DEBUG_MEMORY == 1
//
//extern "C" DLL_EXPORT void* ConstructObj(size_t s, const char *source, unsigned line);
//extern "C" DLL_EXPORT void DeconstructObj(void *p);
//template <class T>
//inline T* ConstructNObj(size_t s,  unsigned n, const char *source, unsigned line) {
//	T *p = (T*)ConstructObj(s * n, source, line);
//	for (unsigned i = 0; i < n; ++i)
//		new (p + i) T();
//	return p;
//}
//
//#define NEW(T) new(ConstructObj(sizeof(T), __FILE__, __LINE__))
//#define DEL(ptr, T) ptr->~T(); DeconstructObj(ptr);
//#define NEW_ARRAY(T, n) ConstructNObj<T>(sizeof(T), n, __FILE__, __LINE__)
//#define DEL_ARRAY(ptr, T, n) \
//	for (unsigned i=0; i<n; ++i) \
//		ptr[i].~T(); \
//	DeconstructObj(ptr);
//
//#else
//
//void* operator new (size_t s);
//void operator delete (void *p);
//void* operator new[] (size_t s);
//void operator delete[] (void *p);
//
//#define NEW(T) new
//#define DEL(ptr, T) delete ptr
//#define NEW_ARRAY(T, n) new T[n]
//#define DEL_ARRAY(ptr, T, n) delete[] ptr
//
//#endif
//
//#else
//
//#define NEW(T) new
//#define DEL(ptr, T) delete
//#define NEW_ARRAY(T, n) new T[n]
//#define DEL_ARRAY(ptr, T, n) delete[] ptr
//
//#endif

#if TARGET_PLATFORM == PLATFORM_WINDOWS
	#include <Windows.h>
// #elif TARGET_PLATFORM == PLATFORM_ANDROID
// 	#include <android/log.h>
#endif

typedef unsigned long long ldword;
typedef unsigned int dword;
typedef unsigned short word;
typedef unsigned char byte;

template <class T>
struct GetValueType {
	typedef T ValType;
};
template <class T> struct GetValueType < T* > { typedef T* ValType; };

template <class T>
struct GetPtrType {
	typedef T* PtrType;
};
template <class T> struct GetPtrType < T* > { typedef T** PtrType; };

template <class T>
struct GetCPtrType {
	typedef const T* CPtrType;
};
template <class T> struct GetCPtrType < T* > { typedef const T** CPtrType; };

template <class T>
struct GetReferenceType {
	typedef T& RType;
};
template <class T> struct GetReferenceType < T* > { typedef T* RType; };


template <class T>
struct GetConstRefType {
	typedef const T& RType;
};
template <class T> struct GetConstRefType < T* > { typedef const T* RType; };

template <class T>
struct TypeTraits {
	typedef typename GetValueType<T>::ValType _ValueType;
	typedef typename GetPtrType<T>::PtrType _PointerType;
	typedef typename GetCPtrType<T>::CPtrType _ConstPointerType;
	typedef typename GetReferenceType<T>::RType _RefType;
	typedef typename GetConstRefType<T>::RType _ConstRefType;
};

class CNoneCopyable {
public:
	CNoneCopyable() {}
	virtual ~CNoneCopyable() {}
	CNoneCopyable(const CNoneCopyable&) = delete;
	CNoneCopyable& operator= (const CNoneCopyable&) = delete;
};

template <class T>
inline T Clamp(T value, T lower, T upper) {
	T f = value < upper ? value : upper;
	return lower > f ? lower : f;
}

inline bool fZero(float f) {
	union {
		float *pf;
		int *pi;
	} u;
	u.pf = &f;
	return *u.pi == 0;
}

#ifndef min
    #define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef max
	#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

// #if COMPILER == COMPILER_GNUCXX
// #define stricmp strcasecmp
// #endif
