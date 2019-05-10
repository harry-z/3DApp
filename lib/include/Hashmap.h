#pragma once



#include "LinkList.h"
#include "Str.h"

dword GetPrime(dword nSlotCount);
DLL_EXPORT void MurmurHash3_x86_32(const void * key, const int len, dword seed, void * out);

template <class T, class Traits = TypeTraits<T>>
struct Hash {
    dword operator() (typename Traits::_ConstRefType refValue) {
        return static_cast<dword>(refValue);
    }
};

template<> struct Hash < String > {
    dword operator() (const String &refValue) {
        dword hash;
        MurmurHash3_x86_32(refValue.c_str(), refValue.length(), 0x3A8EFA67, &hash);
        return hash;
    }
};

struct DLL_EXPORT IdString {
	IdString() : hash(0) {}
	IdString(const String &s) {
        hash = Hash<String>()(s);
#if defined (_DEBUG) || defined(DEBUG)
        str = s;
#endif
    }

	bool operator== (const IdString &other) const { return hash == other.hash; }
	bool operator!= (const IdString &other) const { return hash != other.hash; }

	inline dword HashValue() const { return hash; }
#if defined (_DEBUG) || defined(DEBUG)
	inline const String& HashString() const { return str; }
#endif

private:
	dword hash;
#if defined (_DEBUG) || defined(DEBUG)
	String str;
#endif
};

template<> struct Hash < IdString > {
	dword operator() (const IdString &refValue) {
		return refValue.HashValue();
	}
};

template <class Key, class Value, 
	class KeyTraits = TypeTraits<Key>, class ValueTraits = TypeTraits<Value>>
class CHashmap {
public:
	typedef CHashmap<Key, Value, KeyTraits, ValueTraits> _MyType;

	typedef typename KeyTraits::_ValueType _KeyType;
	typedef typename KeyTraits::_RefType _KeyRefType;
	typedef typename KeyTraits::_ConstRefType _KeyConstRefType;

	typedef typename ValueTraits::_ValueType _ValueType;
	typedef typename ValueTraits::_RefType _ValueRefType;
	typedef typename ValueTraits::_ConstRefType _ValueConstRefType;

	
	CHashmap() : m_pSlots(nullptr), m_nSlotCount(0) {}
	CHashmap(dword nSlotCount);
	~CHashmap();

	void SetSlotCount(dword nSlotCount);
	bool Insert(_KeyConstRefType key, _ValueConstRefType value);
	bool Remove(_KeyConstRefType key);
	bool Has(_KeyConstRefType key) const;
	_ValueRefType Find(_KeyConstRefType key);
	_ValueConstRefType Find(_KeyConstRefType key) const;
	void Clear();
	void Reset();

	struct Pair {
		_KeyType first;
		_ValueType second;
		LinklistNode<Pair> node;
	};
	
	struct Slot {
		Linklist<Pair> list;
		~Slot() {
			LinklistNode<Pair> *pTemp = list.m_pRoot;
			while (pTemp) {
				Pair *pPair = pTemp->m_pOwner;
				pTemp = pTemp->m_pNext;
				// delete pPair;
				DELETE_TYPE(pPair, Pair);
			}
		}
	};

	template <class HashmapType>
	class MyIterator {
	private:
		HashmapType *m_pHashmap;
		LinklistNode<Pair> *m_pCurrNode;
		dword m_nCurrSlotIndex;
	public:
		typedef typename HashmapType::_KeyRefType _KeyRefType;
		typedef typename HashmapType::_ValueRefType _ValueRefType;

		MyIterator() : m_pHashmap(nullptr), m_pCurrNode(nullptr), m_nCurrSlotIndex(0xFFFFFFFF) {}
		MyIterator(HashmapType *pHashmap, dword nCurrSlotIndex, LinklistNode<Pair> *pCurrNode)
			: m_pHashmap(pHashmap), m_nCurrSlotIndex(nCurrSlotIndex), m_pCurrNode(pCurrNode) {}
		MyIterator(const MyIterator &other) {
			m_pHashmap = other.m_pHashmap;
			m_pCurrNode = other.m_pCurrNode;
			m_nCurrSlotIndex = other.m_nCurrSlotIndex;
		}
		MyIterator& operator= (const MyIterator &other) {
			m_pHashmap = other.m_pHashmap;
			m_pCurrNode = other.m_pCurrNode;
			m_nCurrSlotIndex = other.m_nCurrSlotIndex;
			return *this;
		}
		bool operator== (const MyIterator &other) const {
			return m_pHashmap == other.m_pHashmap && m_pCurrNode == other.m_pCurrNode && m_nCurrSlotIndex == other.m_nCurrSlotIndex;
		}
		bool operator!= (const MyIterator & other) const {
			return m_pHashmap != other.m_pHashmap || m_pCurrNode != other.m_pCurrNode || m_nCurrSlotIndex != other.m_nCurrSlotIndex;
		}
		void operator++ () {
			assert(m_pCurrNode != nullptr);
			assert(m_pHashmap != nullptr);
			m_pCurrNode = m_pCurrNode->m_pNext;
			if (m_pCurrNode == nullptr) {
				bool bReachEnd = true;
				while (++m_nCurrSlotIndex < m_pHashmap->m_nSlotCount) {
					m_pCurrNode = m_pHashmap->m_pSlots[m_nCurrSlotIndex].list.m_pRoot;
					if (m_pCurrNode != nullptr) {
						bReachEnd = true;
						break;
					}
				}
				if (bReachEnd)
					m_nCurrSlotIndex = 0xFFFFFFFF;
			}
		}
		void operator-- () {
			assert(m_pCurrNode != nullptr);
			assert(m_pHashmap != nullptr);
			m_pCurrNode = m_pCurrNode->m_pPrev;
			if (m_pCurrNode == nullptr) {
				while (--m_nCurrSlotIndex >= 0) {
					m_pCurrNode = m_pHashmap->m_pSlots[m_nCurrSlotIndex].list.m_pRoot;
					if (m_pCurrNode != nullptr)
						break;
				}
			}
		}
		_KeyRefType Key() const {
			assert(m_pCurrNode);
			return m_pCurrNode->m_pOwner->first;
		}
		_ValueRefType Value() const {
			assert(m_pCurrNode);
			return m_pCurrNode->m_pOwner->second;
		}
	};
	typedef MyIterator<_MyType> Iterator;
	friend class Iterator;

	Iterator Begin() {
		dword idx = 0;
		while (idx < m_nSlotCount && m_pSlots[idx].list.m_pRoot == nullptr)
			++idx;
		if (idx < m_nSlotCount)
			return Iterator(this, idx, m_pSlots[idx].list.m_pRoot);
		else
			return End();
	}
	Iterator End() { return Iterator(this, 0xFFFFFFFF, nullptr); }

	template <class HashmapType>
	class MyConstIterator {
	private:
		HashmapType *m_pHashmap;
		LinklistNode<Pair> *m_pCurrNode;
		dword m_nCurrSlotIndex;
	public:
		typedef typename HashmapType::_KeyConstRefType _KeyConstRefType;
		typedef typename HashmapType::_ValueConstRefType _ValueConstRefType;

		MyConstIterator() : m_pHashmap(nullptr), m_pCurrNode(nullptr), m_nCurrSlotIndex(0xFFFFFFFF) {}
		MyConstIterator(const HashmapType *pHashmap, dword nCurrSlotIndex, LinklistNode<Pair> *pCurrNode)
			: m_pHashmap(pHashmap), m_nCurrSlotIndex(nCurrSlotIndex), m_pCurrNode(pCurrNode) {}
		MyConstIterator(const MyConstIterator &other) {
			m_pHashmap = other.m_pHashmap;
			m_pCurrNode = other.m_pCurrNode;
			m_nCurrSlotIndex = other.m_nCurrSlotIndex;
		}
		MyConstIterator& operator= (const MyConstIterator &other) {
			m_pHashmap = other.m_pHashmap;
			m_pCurrNode = other.m_pCurrNode;
			m_nCurrSlotIndex = other.m_nCurrSlotIndex;
			return *this;
		}
		bool operator== (const MyConstIterator &other) const {
			return m_pHashmap == other.m_pHashmap && m_pCurrNode == other.m_pCurrNode && m_nCurrSlotIndex == other.m_nCurrSlotIndex;
		}
		bool operator!= (const MyConstIterator & other) const {
			return m_pHashmap != other.m_pHashmap || m_pCurrNode != other.m_pCurrNode || m_nCurrSlotIndex != other.m_nCurrSlotIndex;
		}
		void operator++ () {
			assert(m_pCurrNode != nullptr);
			assert(m_pHashmap != nullptr);
			m_pCurrNode = m_pCurrNode->m_pNext;
			if (m_pCurrNode == nullptr) {
				bool bReachEnd = true;
				while (++m_nCurrSlotIndex < m_pHashmap->m_nSlotCount) {
					m_pCurrNode = m_pHashmap->m_pSlots[m_nCurrSlotIndex].list.m_pRoot;
					if (m_pCurrNode != nullptr) {
						bReachEnd = true;
						break;
					}
				}
				if (bReachEnd)
					m_nCurrSlotIndex = 0xFFFFFFFF;
			}
		}
		void operator-- () {
			assert(m_pCurrNode != nullptr);
			assert(m_pHashmap != nullptr);
			m_pCurrNode = m_pCurrNode->m_pPrev;
			if (m_pCurrNode == nullptr) {
				while (--m_nCurrSlotIndex >= 0) {
					m_pCurrNode = m_pHashmap->m_pSlots[m_nCurrSlotIndex].list.m_pRoot;
					if (m_pCurrNode != nullptr)
						break;
				}
			}
		}
		_KeyConstRefType Key() const {
			assert(m_pCurrNode);
			return m_pCurrNode->m_pOwner->first;
		}
		_ValueConstRefType Value() const {
			assert(m_pCurrNode);
			return m_pCurrNode->m_pOwner->second;
		}
	};
	typedef MyConstIterator<_MyType> ConstIterator;
	friend class ConstIterator;

	ConstIterator Begin() const {
		dword idx = 0;
		while (idx < m_nSlotCount && m_pSlots[idx].list.m_pRoot == nullptr)
			++idx;
		if (idx < m_nSlotCount)
			return ConstIterator(this, idx, m_pSlots[idx].list.m_pRoot);
		else
			return End();
	}
	ConstIterator End() const { return ConstIterator(this, 0xFFFFFFFF, nullptr); }

	Iterator FindIter(_KeyConstRefType key);
	ConstIterator FindIter(_KeyConstRefType key) const;

private:
	Slot *m_pSlots;
	dword m_nSlotCount;
};

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	CHashmap<Key, Value, KeyTraits, ValueTraits>::CHashmap(dword nSlotCount)
{
	m_nSlotCount = GetPrime(nSlotCount);
	m_pSlots = NEW_TYPE_ARRAY(Slot, m_nSlotCount);
	// m_pSlots = new Slot[m_nSlotCount];
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	CHashmap<Key, Value, KeyTraits, ValueTraits>::~CHashmap() 
{
	if (m_pSlots) {
		DELETE_TYPE_ARRAY(m_pSlots, Slot, m_nSlotCount);
		// delete[] m_pSlots;
		m_pSlots = nullptr;
	}
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	void CHashmap<Key, Value, KeyTraits, ValueTraits>::SetSlotCount(dword nSlotCount)
{
	if (m_pSlots) {
		// delete[] m_pSlots;
		DELETE_TYPE_ARRAY(m_pSlots, Slot, m_nSlotCount);
		m_pSlots = nullptr;
	}
	m_nSlotCount = GetPrime(nSlotCount);
	// m_pSlots = new Slot[m_nSlotCount];
	m_pSlots = NEW_TYPE_ARRAY(Slot, m_nSlotCount);
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	bool CHashmap<Key, Value, KeyTraits, ValueTraits>::Insert(_KeyConstRefType key, _ValueConstRefType value)
{
	if (!Has(key)) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		// Pair *pair = new Pair;
		Pair *pair = NEW_TYPE(Pair);
		pair->first = key;
		pair->second = const_cast<_ValueRefType>(value);
		pair->node.m_pOwner = pair;
		m_pSlots[idx].list.PushBack(&pair->node);
		return true;
	}
	return false;
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	bool CHashmap<Key, Value, KeyTraits, ValueTraits>::Remove(_KeyConstRefType key)
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				break;
			p = p->m_pNext;
		}
		if (p) {
			m_pSlots[idx].list.Remove(p);
			// delete p->m_pOwner;
			DELETE_TYPE(p->m_pOwner, Pair);
			return true;
		}
	}
	return false;
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	bool CHashmap<Key, Value, KeyTraits, ValueTraits>::Has(_KeyConstRefType key) const
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				return true;
			p = p->m_pNext;
		}
	}

	return false;
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	typename CHashmap<Key, Value, KeyTraits, ValueTraits>::_ValueRefType 
	CHashmap<Key, Value, KeyTraits, ValueTraits>::Find(_KeyConstRefType key)
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				return pair->second;
			p = p->m_pNext;
		}
	}
	return _ValueType();
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	typename CHashmap<Key, Value, KeyTraits, ValueTraits>::_ValueConstRefType
	CHashmap<Key, Value, KeyTraits, ValueTraits>::Find(_KeyConstRefType key) const
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				return pair->second;
			p = p->m_pNext;
		}
	}

	return _ValueType();
}

template <class Key, class Value,
class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	void CHashmap<Key, Value, KeyTraits, ValueTraits>::Clear()
{
	if (m_pSlots) {
		// delete[] m_pSlots;
		DELETE_TYPE_ARRAY(m_pSlots, Slot, m_nSlotCount);
		m_pSlots = nullptr;
	}
}

template <class Key, class Value,
	class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	void CHashmap<Key, Value, KeyTraits, ValueTraits>::Reset()
{
	if (m_pSlots) {
		// delete[] m_pSlots;
		DELETE_TYPE_ARRAY(m_pSlots, Slot, m_nSlotCount);
		// m_pSlots = new Slotp[m_nSlotCount];
		m_pSlots = NEW_TYPE_ARRAY(Slot, m_nSlotCount);
	}
}

template <class Key, class Value,
	class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	typename CHashmap<Key, Value, KeyTraits, ValueTraits>::Iterator
	CHashmap<Key, Value, KeyTraits, ValueTraits>::FindIter(_KeyConstRefType key)
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				return Iterator(this, idx, p);
			p = p->m_pNext;
		}
	}
	
	return End();
}

template <class Key, class Value,
	class KeyTraits /*= TypeTraits<Key>*/, class ValueTraits /*= TypeTraits<Value>*/>
	typename CHashmap<Key, Value, KeyTraits, ValueTraits>::ConstIterator
	CHashmap<Key, Value, KeyTraits, ValueTraits>::FindIter(_KeyConstRefType key) const
{
	if (m_pSlots) {
		dword hash = Hash<_KeyType>()(key);
		dword idx = hash % m_nSlotCount;
		LinklistNode<Pair> *p = m_pSlots[idx].list.m_pRoot;
		while (p) {
			Pair *pair = p->m_pOwner;
			if (pair->first == key)
				return Iterator(this, idx, p);
			p = p->m_pNext;
		}
	}
	
	return End();
}
