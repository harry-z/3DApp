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
#include <malloc.h>
#include <exception>
#include <new>
#include <mutex>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <thread>
#include <initializer_list>
#include <iostream>
#include <algorithm>

#if TARGET_PLATFORM == PLATFORM_WINDOWS
	#include <Windows.h>
// #elif TARGET_PLATFORM == PLATFORM_ANDROID
// 	#include <android/log.h>
#endif

typedef unsigned long long ldword;
typedef unsigned int dword;
typedef unsigned short word;
typedef unsigned char byte;

// And
template <class... Types>
struct TAnd;

template <bool LHSValue, class... RHS>
struct TAndValue
{
	enum { Value = TAnd<RHS...>::Value };
};

template <class... RHS>
struct TAndValue<false, RHS...>
{
	enum { Value = false };
};

template <class LHS, class... RHS>
struct TAnd<LHS, RHS...> : TAndValue<LHS::Value, RHS...>
{
};

template <>
struct TAnd<>
{
	enum { Value = true };
};

// Or
template <class... Types>
struct TOr;

template <bool LHSValue, class... RHS>
struct TOrValue
{
	enum { Value = TOr<RHS...>::Value };
};

template <class... RHS>
struct TOrValue<true, RHS...>
{
	enum { Value = true };
};

template <class LHS, class... RHS>
struct TOr<LHS, RHS...> : TOrValue<LHS::Value, RHS...>
{
};

template <>
struct TOr<>
{
	enum { Value = false };
};

// Not
template <class T>
struct TNot
{
	enum { Value = !T::Value };
};

// IsEnum
template <class T>
struct TIsEnum
{
	enum { Value = __is_enum(T) };
};

// IsArithmetic
template <class T>
struct TIsArithmetic
{ 
	enum { Value = false };
};

template <> struct TIsArithmetic<float>       { enum { Value = true }; };
template <> struct TIsArithmetic<double>      { enum { Value = true }; };
template <> struct TIsArithmetic<long double> { enum { Value = true }; };
template <> struct TIsArithmetic<byte>       { enum { Value = true }; };
template <> struct TIsArithmetic<word>      { enum { Value = true }; };
template <> struct TIsArithmetic<dword>      { enum { Value = true }; };
template <> struct TIsArithmetic<ldword>      { enum { Value = true }; };
template <> struct TIsArithmetic<char>        { enum { Value = true }; };
template <> struct TIsArithmetic<short>       { enum { Value = true }; };
template <> struct TIsArithmetic<int>       { enum { Value = true }; };
template <> struct TIsArithmetic<long long>       { enum { Value = true }; };
template <> struct TIsArithmetic<bool>        { enum { Value = true }; };
template <> struct TIsArithmetic<wchar_t>    { enum { Value = true }; };

template <class T> struct TIsArithmetic<const          T> { enum { Value = TIsArithmetic<T>::Value }; };
template <class T> struct TIsArithmetic<      volatile T> { enum { Value = TIsArithmetic<T>::Value }; };
template <class T> struct TIsArithmetic<const volatile T> { enum { Value = TIsArithmetic<T>::Value }; };

// IsPointer
template <class T>
struct TIsPointer
{
	enum { Value = false };
};

template <class T> struct TIsPointer<               T*> { enum { Value = true }; };
template <class T> struct TIsPointer<const          T*> { enum { Value = true }; };
template <class T> struct TIsPointer<      volatile T*> { enum { Value = true }; };
template <class T> struct TIsPointer<const volatile T*> { enum { Value = true }; };

template <class T> struct TIsPointer<const          T> { enum { Value = TIsPointer<T>::Value }; };
template <class T> struct TIsPointer<      volatile T> { enum { Value = TIsPointer<T>::Value }; };
template <class T> struct TIsPointer<const volatile T> { enum { Value = TIsPointer<T>::Value }; };

// ZeroContructType
template <class T>
struct TIsZeroContructType
{
	enum { Value = TOr< TIsEnum<T>, TIsArithmetic<T>, TIsPointer<T> >::Value };
};

// TriviallyDestructible
namespace IsTriviallyDestructible_Private
{
	template <class T, bool bIsTriviallyTriviallyDestructible = __is_enum(T)>
	struct TImpl
	{
		enum { Value = true };
	};

	template <class T>
	struct TImpl<T, false>
	{
		enum { Value = __has_trivial_destructor(T) };
	};
}

template <class T>
struct TIsTriviallyDestructible
{
	enum { Value = IsTriviallyDestructible_Private::TImpl<T>::Value };
};

template <bool Predicate, class Result = void>
class TEnableIf;

template <class Result>
class TEnableIf<true, Result>
{
public:
	typedef Result Type;
};

template <class Result>
class TEnableIf<false, Result>
{};

template <class T>
struct GetValueType {
	typedef T ValType;
};

template <class T>
struct GetPtrType {
	typedef T* PtrType;
};

template <class T>
struct GetCPtrType {
	typedef const T* CPtrType;
};

template <class T>
struct GetReferenceType {
	typedef T& RType;
};

template <class T>
struct GetConstRefType {
	typedef const T& RType;
};

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
