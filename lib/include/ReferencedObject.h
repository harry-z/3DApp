#pragma once

#include "Prereq.h"

class CReferencedObject : public CNoneCopyable {
public:
	CReferencedObject() { m_nRefCount = 0; }

	inline void AddRef() {
		++m_nRefCount;
	}
	inline void Release() {
		--m_nRefCount;
		if (0 == m_nRefCount)
			Destroy();
	}
	// Description:
	// 	Return true indicates reference count is greater than 0, otherwise
	//	equals to 0
	inline bool CheckRefCount() {
		return 0 != m_nRefCount;
	}

protected:
	virtual void Destroy() = 0;

protected:
	std::atomic_uint m_nRefCount;
};

template <class T>
class CReferencedPointer {
public:
	CReferencedPointer() : m_pPointer(nullptr) {}
	CReferencedPointer(T *pointer) : m_pPointer(pointer) {
		m_pPointer->AddRef();
	}
	// Description:
	// 	Class type F can be converted to class type T
	template <class F> CReferencedPointer(const CReferencedPointer<F> &other) {
		m_pPointer = static_cast<T*>(other.m_pPointer);
		m_pPointer->AddRef();
	}

	~CReferencedPointer() {
		if (m_pPointer != nullptr) {
			m_pPointer->Release();
			m_pPointer = nullptr;
		}
	}

	/*explicit CReferencedPointer& operator= (const CReferencedPointer &other) {
	if (m_pPointer == other.m_pPointer)
	return *this;
	T *pTmp = m_pPointer;
	m_pPointer = static_cast<T*>(other.m_pPointer);
	m_pPointer->AddRef();
	if (pTmp)
	pTmp->Release();
	return *this;
	}*/

	CReferencedPointer& operator= (T *pointer) {
		if (m_pPointer == pointer)
			return *this;
		T *pTmp = m_pPointer;
		m_pPointer = pointer;
		m_pPointer->AddRef();
		if (pTmp)
			pTmp->Release();
		return *this;
	}

	// Description:
	// 	Implicit output conversion
	operator T*() const { return m_pPointer; }

	bool operator== (const CReferencedPointer &other) const {
		return m_pPointer == other.m_pPointer;
	}
	bool operator== (const T *pointer) const {
		return m_pPointer == pointer;
	}
	friend bool operator== (const T *pointer, const CReferencedPointer &ref) {
		return pointer == ref.m_pPointer;
	}

	bool operator!= (const CReferencedPointer &other) const {
		return m_pPointer != other.m_pPointer;
	}
	bool operator!= (const T *pointer) const {
		return m_pPointer != pointer;
	}
	friend bool operator!= (const T *pointer, const CReferencedPointer &ref) {
		return pointer != ref.m_pPointer;
	}

	T& operator* () { return *m_pPointer; }
	const T& operator* () const { return *m_pPointer; }
	T* operator-> () { return m_pPointer; }
	const T* operator-> () const { return m_pPointer; }

	T* Get() { return m_pPointer; }
	const T* Get() const { return m_pPointer; }

	bool IsValid() const { return m_pPointer != nullptr; }

	void Detach() {
		if (m_pPointer) {
			m_pPointer->Release();
			m_pPointer = 0;
		}
	}

private:
	T *m_pPointer;
};