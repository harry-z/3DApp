#pragma once

#include "Memory.h"



template <class T, class Traits = TypeTraits<T>>
class CArray {
public:
	typedef typename Traits::_ValueType _ValueType;
	typedef typename Traits::_PointerType _PointerType;
	typedef typename Traits::_ConstPointerType _ConstPointerType;
	typedef typename Traits::_RefType _RefType;
	typedef typename Traits::_ConstRefType _ConstRefType;

	static const dword npos;

	CArray()
		: m_pElements(nullptr),
		m_nCount(0),
		m_nAllocCount(0) {}
	CArray(dword nCount)
		: m_pElements(nullptr),
		m_nCount(0),
		m_nAllocCount(nCount) {
		Realloc(); Clear();
	}
	CArray(const CArray &other) {
		m_pElements = nullptr;
		m_nCount = other.m_nCount;
		m_nAllocCount = other.m_nAllocCount;
		Realloc();
		Copy(m_pElements, other.m_pElements, other.m_nCount);
	}
	~CArray() { Free(); }

	CArray& operator= (const CArray &other) {
		Free();
		new(this) CArray(other);
		return *this;
	}

	void Free() {
		DestructItems(m_pElements, m_nCount);
		m_nCount = 0;
		if (m_nAllocCount)
			MEMFREE(m_pElements);
		m_nAllocCount = 0;
		m_pElements = nullptr;
	}
	void Reserve(dword nCount) {
		m_nCount = 0;
		m_nAllocCount = nCount;
		Realloc(); Clear();
	}
	void Realloc() {
		if (m_nAllocCount == 0)
			m_pElements = nullptr;
		else {
			if (m_pElements != nullptr) {
				_PointerType pTempElements = (T*)MEMALLOC(sizeof(_ValueType) * m_nAllocCount);
				assert(pTempElements != nullptr);
				if (m_nCount)
					Copy(pTempElements, m_pElements, m_nCount);
				DestructItems(m_pElements, m_nCount);
				MEMFREE(m_pElements);
				m_pElements = pTempElements;
			}
			else
			{
				m_pElements = (T*)MEMALLOC(sizeof(_ValueType) * m_nAllocCount);
				assert(m_pElements != nullptr);
			}
		}
	}
	void Remove(dword nIndex, dword nCount = 1) {
		if (nCount && nIndex + nCount <= m_nCount) {
			dword nIter = nCount;
			while (nIter > 0)
			{
				(m_pElements + (nIndex + nCount - nIter))->~T();
				--nIter;
			}
			memmove(m_pElements + nIndex, m_pElements + nIndex + nCount, sizeof(_ValueType) * (m_nCount - nIndex - nCount));
			m_nCount -= nCount;
		}
	}
	inline dword Num() const { return m_nCount; }
	inline dword Capacity() const { return m_nAllocCount; }
	inline bool Empty() const { return m_nCount == 0; }
	inline bool Full() const { return m_nCount >= m_nAllocCount; }
	inline void SetNum(dword nCount) { m_nCount = nCount; }
	void Clear() {
		DestructItems(m_pElements, m_nCount);
		memset(m_pElements, 0, sizeof(_ValueType) * m_nCount);
	}
	_PointerType Data() { return m_pElements; }
	_ConstPointerType Data() const { return m_pElements; }
	bool IsValidIndex(dword nIndex) const { return nIndex < m_nCount; }
	_RefType operator[] (dword nIndex) { assert(nIndex < m_nCount); return m_pElements[nIndex]; }
	_ConstRefType operator[] (dword nIndex) const { assert(nIndex < m_nCount); return m_pElements[nIndex]; }
	_RefType Get(dword nIndex) {
		return operator[] (nIndex);
	}
	_ConstRefType Get(dword nIndex) const {
		return operator[] (nIndex);
	}
	dword Find(_ConstRefType obj) const {
		for (dword i = 0; i < m_nCount; ++i)
			if (obj == m_pElements[i])
				return i;
		return npos;
	}
	_PointerType AddIndex(dword inc) {
		dword nIndex = m_nCount;
		dword nNewCount = m_nCount + inc;
		if (nNewCount > m_nAllocCount) {
			dword nOldAlloc = m_nAllocCount;
			m_nAllocCount = nNewCount * 2; //nNewCount + (nNewCount >> 1) + 10;
			Realloc();
			// memset(&m_pElements[nOldAlloc], 0, sizeof(_ValueType) * (m_nAllocCount - nOldAlloc));
		}
		m_nCount = nNewCount;
		return &m_pElements[nIndex];
	}
	void Add(_RefType obj) {
		_PointerType p = AddIndex(1);
		new (p) T(obj);
		// *p = obj;
	}
	template <class ... ArgsType>
	void Emplace(ArgsType &&... Args) {
		_PointerType p = AddIndex(1);
		new (p) T(std::forward<ArgsType>(Args)...);
	}
	void Set(dword nIndex, _RefType obj) {
		assert(nIndex < m_nCount);
		m_pElements[nIndex] = obj;
	}
	template <class ... ArgsType>
	void SetRValue(dword nIndex, ArgsType &&... Args) {
		assert(nIndex < m_nCount);
		m_pElements[nIndex] = T(std::forward<ArgsType>(Args)...);
	}

	friend _PointerType begin(CArray &array) { return array.Data(); }
	friend _ConstPointerType begin(const CArray &array) { return array.Data(); }
	friend _PointerType end(CArray &array) { return array.Data() + array.Num(); }
	friend _ConstPointerType end(const CArray &array) { return array.Data() + array.Num(); }

private:
	void Copy(_PointerType pRawDestData, _ConstPointerType pSrcData, dword nCount) {
		for (dword i = 0; i < nCount; ++i)
		{
			new (&pRawDestData[i]) T;
			pRawDestData[i] = pSrcData[i];
		}
	}

private:
	_PointerType m_pElements;
	dword m_nCount;
	dword m_nAllocCount;
};

template <class T, class Traits> const dword CArray<T, Traits>::npos = 0xFFFFFFFF;