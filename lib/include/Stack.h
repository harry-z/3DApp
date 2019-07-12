#pragma once

#include "PreHeader.h"

class CStackAllocator {
public:
	CStack();
	~CStack();

	bool Initialize(dword nSize);
	void* Allocate(dword nBytes);
	inline void Clear() { m_nOffset = 0; }

private:
	inline bool NotEnoughMemory(dword nSize) const {
		return m_nOffset + nSize > m_nSize;
	}

private:
	dword m_nSize;
	dword m_nOffset;
	byte *m_pBuffer;
};

template <class T, class Traits = TypeTraits<T>>
class CStack {
public:
	typedef typename Traits::_ValueType _ValueType;
	typedef typename Traits::_PointerType _PointerType;
	typedef typename Traits::_ConstPointerType _ConstPointerType;
	typedef typename Traits::_RefType _RefType;
	typedef typename Traits::_ConstRefType _ConstRefType;

	void Push(_RefType obj) { m_Storage.Add(obj); }
	template <class ... ArgsType>
	void Emplace(ArgsType &&... Args) {
		m_Storage.Emplace(std::forward<ArgsType>(Args)...);
	}
	_ConstPointerType Last() const { return m_Storage.Num() > 0 ? &m_Storage[m_Storage.Num() - 1] : nullptr; }
	_ValueType Pop() const { 
		_ValueType obj = m_Storage[m_Storage.Num() - 1];
		m_Storage.Remove(m_Storage.Num() - 1);
		return std::move(obj);
	}
	dword Num() const { return m_Storage.Num(); }

private:
	CArray<T> m_Storage;
};