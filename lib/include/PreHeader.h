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
#else
	// #define DLL_EXPORT
#endif



#define IN
#define OUT
#define INOUT

#define BIT_ADD(flag, bit) (flag) |= (bit)
#define BIT_REMOVE(flag, bit) (flag) &= (~(bit))
#define BIT_CHECK(flag, bit) ((flag) & (bit)) != 0

#define SAFE_DELETE(p) if (p) { delete (p); (p) = nullptr; }
#define SAFE_RELEASE(p) if (p) { (p)->Release(); (p) = nullptr; }

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

#define ALIGN_SIZE(bytes) (((bytes)+ALIGN-1) & ~(ALIGN-1))

#define EPSILON 1e-6

#define PI 3.14159265f

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <exception>
#include <new>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <thread>

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

#define CSTR_MAX 256


// #if COMPILER == COMPILER_GNUCXX
// #define stricmp strcasecmp
// #endif
