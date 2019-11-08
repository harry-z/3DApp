#pragma once
#include "PreHeader.h"
#include "Pool.h"

template < class Key, class Value, class KeyTraits = TypeTraits<Key>, class ValueTraits = TypeTraits<Value> >
class CRBNode
{
public:
	using _MyKeyType = typename KeyTraits::_ValueType;
    using _MyKeyRefType = typename KeyTraits::_RefType;
    using _MyKeyConstRefType = typename KeyTraits::_ConstRefType;
	using _MyKeyPointerType = typename KeyTraits::_PointerType;
	using _MyKeyConstPointerType = typename KeyTraits::_ConstPointerType;

	using _MyValueType = typename ValueTraits::_ValueType;
    using _MyValueRefType = typename ValueTraits::_RefType;
    using _MyValueConstRefType = typename ValueTraits::_ConstRefType;
	using _MyValuePointerType = typename ValueTraits::_PointerType;
	using _MyValueConstPointerType = typename ValueTraits::_ConstPointerType;

	CRBNode(typename KeyTraits::_ConstRefType key, typename ValueTraits::_ConstRefType value, CRBNode* p = nullptr, CRBNode* l = nullptr , CRBNode* r = nullptr)
	: m_Key(key), m_Value(value), m_pParentNode(p), m_pLeftChild(l), m_pRightChild(r), m_bRed(false)
	{}

	CRBNode()
	: m_Key(_MyKeyType()), m_Value(_MyValueType()), m_pParentNode(nullptr), m_pLeftChild(nullptr), m_pRightChild(nullptr), m_bRed(false)
	{}

	_MyKeyRefType GetKey()
	{
		return m_Key;
	}

    _MyKeyConstRefType GetKey() const
    {
        return m_Key;
    }

    _MyValueRefType GetValue()
    {
        return m_Value;
    }

    _MyValueConstRefType GetValue() const
    {
        return m_Value;
    }

	Key m_Key;
    Value m_Value;
	CRBNode* m_pParentNode;
	CRBNode* m_pLeftChild;
	CRBNode* m_pRightChild;
	bool m_bRed;
};

template <class TreeType, bool bConst>
class CRBTreeBaseIteratorConstType
{};

template <class TreeType>
class CRBTreeBaseIteratorConstType<TreeType, false>
{
public:
	using _MyIterKeyRefType = typename TreeType::_MyKeyRefType;
	using _MyIterValueRefType = typename TreeType::_MyValueRefType;
};

template <class TreeType>
class CRBTreeBaseIteratorConstType<TreeType, true>
{
public:
	using _MyIterKeyRefType = typename TreeType::_MyKeyConstRefType;
	using _MyIterValueRefType = typename TreeType::_MyValueConstRefType;
};

template <class TreeType, bool bConst>
class CRBTreeBaseIterator
{
public:
	using _MyTreeType = TreeType;
	using _MyNodeptr = typename TreeType::_MyNodeptr;
	using _MyIterKeyRefType = typename CRBTreeBaseIteratorConstType<TreeType, bConst>::_MyIterKeyRefType;
	using _MyIterValueRefType = typename CRBTreeBaseIteratorConstType<TreeType, bConst>::_MyIterValueRefType;

	CRBTreeBaseIterator() = default;
	CRBTreeBaseIterator(const CRBTreeBaseIterator&) = default;
	CRBTreeBaseIterator(_MyTreeType* t, _MyNodeptr n)
	: m_pTree(t), m_pNode(n)
	{}

	CRBTreeBaseIterator& operator++()
	{
		m_pNode = m_pTree->Successor(m_pNode);
		return *this;
	}

	CRBTreeBaseIterator operator++(int)
	{
		CRBTreeIterator ret(m_pTree, m_pNode);
		m_pNode = m_pTree->Successor(m_pNode);
		return ret;
	}

	CRBTreeBaseIterator& operator--()
	{
		m_pNode = m_pTree->Predecessor(m_pNode);
		return *this;
	}

	CRBTreeBaseIterator operator--(int)
	{
		CRBTreeIterator ret(m_pTree , m_pNode);
		m_pNode = m_pTree->Predecessor(m_pNode);
		return ret;
	}

	explicit operator bool() const
	{
		return m_pNode != nullptr;
	}

	bool operator !() const
	{
		return !(bool)*this;
	}

protected:
	_MyTreeType* m_pTree = nullptr;
	_MyNodeptr m_pNode = nullptr;
};

template <class TreeType>
class CRBTreeIterator : public CRBTreeBaseIterator<TreeType, false>
{
public:  
	using _MyParentType = CRBTreeBaseIterator<TreeType, false>;
	using _MyTreeType = typename _MyParentType::_MyTreeType;
	using _MyNodeptr = typename _MyParentType::_MyNodeptr;
	using _MyIterKeyRefType = typename _MyParentType::_MyIterKeyRefType;
	using _MyIterValueRefType = typename _MyParentType::_MyIterValueRefType;
	
	CRBTreeIterator() {}
	CRBTreeIterator(_MyTreeType* t, _MyNodeptr n)
	: CRBTreeBaseIterator(t, n)
	{}
	CRBTreeIterator(const CRBTreeIterator& other)
	{
		m_pTree = other.m_pTree;
		m_pNode = other.m_pNode;
	}

	_MyNodeptr operator->() { return m_pNode; }
	_MyIterKeyRefType Key() { return m_pNode->GetKey(); }
	_MyIterValueRefType Value() { return m_pNode->GetValue(); }
};

template <class TreeType>
class CRBTreeConstIterator : public CRBTreeBaseIterator<TreeType, true>
{
public:  
	using _MyParentType = CRBTreeBaseIterator<TreeType, true>;
	using _MyTreeType = typename _MyParentType::_MyTreeType;
	using _MyNodeptr = typename _MyParentType::_MyNodeptr;
	using _MyIterKeyRefType = typename _MyParentType::_MyIterKeyRefType;
	using _MyIterValueRefType = typename _MyParentType::_MyIterValueRefType;
	
	CRBTreeConstIterator() {}
	CRBTreeConstIterator(const _MyTreeType* t, _MyNodeptr n)
	: CRBTreeBaseIterator(const_cast<_MyTreeType*>(t), n)
	{}
	CRBTreeConstIterator(const CRBTreeConstIterator& other)
	{
		m_pTree = other.m_pTree;
		m_pNode = other.m_pNode;
	}

	const _MyNodeptr operator->() const { return m_pNode; }
	_MyIterKeyRefType Key() const { return m_pNode->GetKey(); }
	_MyIterValueRefType Value() const { return m_pNode->GetValue(); }
};

template < class Type, class Traits = TypeTraits<Type> >
struct CDefaultLessComparison
{
    static bool Compare(typename Traits::_ConstRefType left, typename Traits::_ConstRefType right)
    {
        return left < right;
    }
};

template< class NodeType, class Comparison >
class CRBTree
{
public:
	using _MyKeyType = typename NodeType::_MyKeyType;
	using _MyKeyRefType = typename NodeType::_MyKeyRefType;
    using _MyKeyConstRefType = typename NodeType::_MyKeyConstRefType;
	using _MyKeyPointerType = typename NodeType::_MyKeyPointerType;
	using _MyKeyConstPointerType = typename NodeType::_MyKeyConstPointerType;

	using _MyValueType = typename NodeType::_MyValueType;
    using _MyValueRefType = typename NodeType::_MyValueRefType;
    using _MyValueConstRefType = typename NodeType::_MyValueConstRefType;
	using _MyValuePointerType = typename NodeType::_MyValuePointerType;
	using _MyValueConstPointerType = typename NodeType::_MyValueConstPointerType;

    using _MyNodeType = NodeType;
	using _MyNodeptr = _MyNodeType*;

	CRBTree()
	{
		m_pNil = &m_NilNode;
		m_pRoot = m_pNil;
        m_NodePool.Initialize(sizeof(_MyNodeType));
	}

	~CRBTree()
	{
		ClearTree(m_pRoot);
	}

	_MyNodeptr GetRoot() const
	{
		return m_pRoot;
	}

	_MyNodeptr GetNil() const
	{
		return m_pNil;
	}

	bool IsEmpty() const
	{
		return m_pRoot == m_pNil;
	}

	dword BackHeight() const
	{
		_MyNodeptr np = m_pRoot;
		dword back_count = 0;
		for (; np != m_pNil;)
		{
			if (!np->m_bRed)
				++back_count;
			np = np->m_pLeftChild;
		}
		return back_count;
	}

	void InsertMulti(_MyKeyConstRefType key, _MyValueConstRefType value)
	{
		_MyNodeptr np = BuyNode(key, value);
		Insert(np);
	}

	bool InsertUnique(_MyKeyConstRefType key, _MyValueConstRefType value)
	{
		if (Find(key) == nullptr)
		{
			_MyNodeptr np = BuyNode(key, value);
			Insert(np);
			return true;
		}
		
		return false;
	}

	void Insert(_MyNodeptr np)
	{
		_MyNodeptr parent = m_pNil;
		for (_MyNodeptr temp = m_pRoot; temp != m_pNil;)
		{
			parent = temp;
			if (Comparison::Compare(np->GetKey(), temp->GetKey()))
				temp = temp->m_pLeftChild;
			else
				temp = temp->m_pRightChild;
		}
		np->m_pParentNode = parent;
		if (parent == m_pNil)
			m_pRoot = np;
		else if (Comparison::Compare(np->GetKey(), parent->GetKey()))
			parent->m_pLeftChild = np;
		else
			parent->m_pRightChild = np;
		np->m_pRightChild = m_pNil;
		np->m_pLeftChild = m_pNil;
		np->m_bRed = true;
		InsertFixup(np);
	}

	_MyNodeptr Find(_MyKeyConstRefType key) const
	{
		if (m_pRoot == m_pNil)
			return nullptr;
			
		_MyNodeptr now = m_pRoot;
		for (; now;)
		{
			if (key == now->GetKey())
				break;
			else if (Comparison::Compare(key, now->GetKey()))
				now = now->m_pLeftChild;
			else
				now = now->m_pRightChild;
		}
		if (now != m_pNil)
			return now;
		else
			return nullptr;
	}

	bool Delete(_MyKeyConstRefType key)
	{
		_MyNodeptr np = Find(key);
		if (np)
		{
			Erase(np);
			return true;
		}
		else
			return false;
	}

	void Erase(_MyNodeptr z)
	{
		_MyNodeptr x;
		_MyNodeptr y = z;
		bool y_original_color = y->m_bRed;
		if (z->m_pLeftChild == m_pNil)
		{
			x = z->m_pRightChild;
			Transplant(z, z->m_pRightChild);
		}
		else if (z->m_pRightChild == m_pNil)
		{
			x = z->m_pLeftChild;
			Transplant(z, z->m_pLeftChild);
		}
		else
		{
			y = Minimum(z->m_pRightChild);
			y_original_color = y->m_bRed;
			x = y->m_pRightChild;
			if (y->m_pParentNode == z)
				x->m_pParentNode = y;
			else
			{
				Transplant(y , y->m_pRightChild);
				y->m_pRightChild = z->m_pRightChild;
				y->m_pRightChild->m_pParentNode = y;
			}
			Transplant(z , y);
			y->m_pLeftChild = z->m_pLeftChild;
			y->m_pLeftChild->m_pParentNode = y;
			y->m_bRed = z->m_bRed;
		}
		if (y_original_color == false)
			DeleteFixup(x);

		FreeNode(z);
	}

	void Clear()
	{
		ClearTree(m_pRoot);
		m_pRoot = m_pNil;
	}

	_MyNodeptr Successor(_MyNodeptr np) const
	{
		if (np->m_pRightChild != m_pNil)
			return Minimum(np->m_pRightChild);

		_MyNodeptr parent = np->m_pParentNode;
		for (; parent!=m_pNil && np==parent->m_pRightChild;)
		{
			np = parent;
			parent = parent->m_pParentNode;
		}

		if (parent != m_pNil)
			return parent;
		else
			return nullptr;
	}

	_MyNodeptr Predecessor(_MyNodeptr np) const
	{
		if (np->m_pLeftChild != m_pNil)
			return Maximum(np->m_pLeftChild);

		_MyNodeptr parent = np->m_pParentNode;
		for (; parent != m_pNil && np == parent->m_pLeftChild)
		{
			np = parent;
			parent = parent->m_pParentNode;
		}

		if (parent != m_pNil)
			return parent;
		else
			return nullptr;
	}

	_MyNodeptr First() const
	{
		if (m_pRoot == m_pNil)
			return nullptr;
		else
			return Minimum(m_pRoot);
	}

	_MyNodeptr Last() const
	{
		if (m_pRoot == m_pNil)
			return nullptr;
		else
			return Maximum(m_pRoot);
	}

	_MyNodeptr Minimum(_MyNodeptr np) const
	{
		for (;;)
		{
			if (np->m_pLeftChild != m_pNil)
				np = np->m_pLeftChild;
			else
				return np;
		}
	}

	_MyNodeptr Maximum(_MyNodeptr np) const
	{
		for (;;)
		{
			if (np->m_pRightChild)
				np = np->m_pRightChild;
			else
				return np;
		}
	}

private:

	_MyNodeptr BuyNode(_MyKeyConstRefType key, _MyValueConstRefType value)
	{
		void *pmem = m_NodePool.Allocate_mt();
        _MyNodeptr np = new (pmem) _MyNodeType(key, value);
		return np;
	}

	void FreeNode(_MyNodeptr np)
	{
        np->~_MyNodeType();
		m_NodePool.Free_mt(np);
	}

	void InsertFixup(_MyNodeptr np)
	{
		for (; np->m_pParentNode->m_bRed;)
		{
			_MyNodeptr parbro;
			if (np->m_pParentNode == np->m_pParentNode->m_pParentNode->m_pLeftChild)
			{
				parbro = np->m_pParentNode->m_pParentNode->m_pRightChild;

				if (parbro->m_bRed)
				{
					np->m_pParentNode->m_bRed = false;
					parbro->m_bRed = false;
					np->m_pParentNode->m_pParentNode->m_bRed = true;
					np = np->m_pParentNode->m_pParentNode;
				}
				else
				{
					if (np == np->m_pParentNode->m_pRightChild)
					{
						np = np->m_pParentNode;
						LeftRotate(np);
					}
					np->m_pParentNode->m_bRed = false;
					np->m_pParentNode->m_pParentNode->m_bRed = true;
					RightRotate(np->m_pParentNode->m_pParentNode);
				}
			}
			else
			{
				parbro = np->m_pParentNode->m_pParentNode->m_pLeftChild;
				if (parbro->m_bRed)
				{
					np->m_pParentNode->m_bRed = false;
					parbro->m_bRed = false;
					np->m_pParentNode->m_pParentNode->m_bRed = true;
					np = np->m_pParentNode->m_pParentNode;
				}
				else
				{
					if (np == np->m_pParentNode->m_pLeftChild)
					{
						np = np->m_pParentNode;
						RightRotate(np);
					}
					np->m_pParentNode->m_bRed = false;
					np->m_pParentNode->m_pParentNode->m_bRed = true;
					LeftRotate(np->m_pParentNode->m_pParentNode);
				}
			}
		}
		m_pRoot->m_bRed = false;
	}

	void LeftRotate(_MyNodeptr np)
	{
		_MyNodeptr oth = np->m_pRightChild;
		np->m_pRightChild = oth->m_pLeftChild;

		if (oth->m_pLeftChild != m_pNil)
			oth->m_pLeftChild->m_pParentNode = np;

		oth->m_pParentNode = np->m_pParentNode;
		if (np->m_pParentNode == m_pNil)
			m_pRoot = oth;
		else if (np == np->m_pParentNode->m_pLeftChild)
			np->m_pParentNode->m_pLeftChild = oth;
		else
			np->m_pParentNode->m_pRightChild = oth;

		oth->m_pLeftChild = np;
		np->m_pParentNode = oth;
	}

	void RightRotate(_MyNodeptr np)
	{
		_MyNodeptr oth = np->m_pLeftChild;
		np->m_pLeftChild = oth->m_pRightChild;

		if (oth->m_pRightChild != m_pNil)
			oth->m_pRightChild->m_pParentNode = np;

		oth->m_pParentNode = np->m_pParentNode;
		if (np->m_pParentNode == m_pNil)
			m_pRoot = oth;
		else if (np == np->m_pParentNode->m_pLeftChild)
			np->m_pParentNode->m_pLeftChild = oth;
		else
			np->m_pParentNode->m_pRightChild = oth;

		oth->m_pRightChild = np;
		np->m_pParentNode = oth;
	}

	void Transplant(_MyNodeptr x, _MyNodeptr y)
	{
		if (x->m_pParentNode == m_pNil)
			m_pRoot = y;
		else if (x == x->m_pParentNode->m_pLeftChild)
			x->m_pParentNode->m_pLeftChild = y;
		else
			x->m_pParentNode->m_pRightChild = y;

		y->m_pParentNode = x->m_pParentNode;
	}

	void DeleteFixup(_MyNodeptr x)
	{
		for (; x != m_pRoot && (x->m_bRed == false);)
		{
			_MyNodeptr bro;
			if (x == x->m_pParentNode->m_pLeftChild)
			{
				bro = x->m_pParentNode->m_pRightChild;
				if (bro->m_bRed)
				{
					bro->m_bRed = false;
					x->m_pParentNode->m_bRed = true;
					LeftRotate(x->m_pParentNode);
					bro = x->m_pParentNode->m_pRightChild;
				}
				if ((bro->m_pLeftChild->m_bRed == false) && (bro->m_pRightChild->m_bRed == false))
				{
					bro->m_bRed = true;
					x = x->m_pParentNode;
				}
				else
				{
					if (bro->m_pRightChild->m_bRed == false)
					{
						bro->m_pLeftChild->m_bRed = false;
						bro->m_bRed = true;
						RightRotate(bro);
						bro = x->m_pParentNode->m_pRightChild;
					}
					bro->m_bRed = x->m_pParentNode->m_bRed;
					x->m_pParentNode->m_bRed = false;
					bro->m_pRightChild->m_bRed = false;
					LeftRotate(x->m_pParentNode);
					x = m_pRoot;
				}
			}
			else
			{
				bro = x->m_pParentNode->m_pLeftChild;
				if (bro->m_bRed)
				{
					bro->m_bRed = false;
					x->m_pParentNode->m_bRed = true;
					RightRotate(x->m_pParentNode);
					bro = x->m_pParentNode->m_pLeftChild;
				}
				if ((bro->m_pRightChild->m_bRed == false) && (bro->m_pLeftChild->m_bRed == false))
				{
					bro->m_bRed = true;
					x = x->m_pParentNode;
				}
				else
				{
					if (bro->m_pLeftChild->m_bRed == false)
					{
						bro->m_pRightChild->m_bRed = false;
						bro->m_bRed = true;
						LeftRotate(bro);
						bro = x->m_pParentNode->m_pLeftChild;
					}
					bro->m_bRed = x->m_pParentNode->m_bRed;
					x->m_pParentNode->m_bRed = false;
					bro->m_pLeftChild->m_bRed = false;
					RightRotate(x->m_pParentNode);
					x = m_pRoot;
				}
			}
		}
		x->m_bRed = false;
	}

	void ClearTree(_MyNodeptr np)
	{
		if (np == m_pNil)
			return;

		ClearTree(np->m_pLeftChild);
		ClearTree(np->m_pRightChild);
		FreeNode(np);
	}

	_MyNodeptr m_pRoot;
	_MyNodeptr m_pNil;
	NodeType m_NilNode;
    CPool m_NodePool;
};

template < class Key, class Value, class Comparison = CDefaultLessComparison<Key> >
class CMap : CRBTree<CRBNode<Key, Value>, Comparison>
{
public:
    using _MyParentType = CRBTree<CRBNode<Key, Value>, Comparison>;
    using _MyKeyRefType = typename _MyParentType::_MyKeyRefType;
    using _MyKeyConstRefType = typename _MyParentType::_MyKeyConstRefType;
	using _MyKeyPointerType = typename _MyParentType::_MyKeyPointerType;
	using _MyKeyConstPointerType = typename _MyParentType::_MyKeyConstPointerType;

	using _MyValueType = typename _MyParentType::_MyValueType;
    using _MyValueRefType = typename _MyParentType::_MyValueRefType;
    using _MyValueConstRefType = typename _MyParentType::_MyValueConstRefType;
	using _MyValuePointerType = typename _MyParentType::_MyValuePointerType;
	using _MyValueConstPointerType = typename _MyParentType::_MyValueConstPointerType;

	using _MyIterType = CRBTreeIterator< _MyParentType >;
	using _MyConstIterType = CRBTreeConstIterator< _MyParentType >;

    bool Insert(_MyKeyConstRefType key, _MyValueConstRefType value)
    {
        return CRBTree::InsertUnique(key, value);
    }

	void Remove(_MyKeyConstRefType key)
	{
		CRBTree::Delete(key);
	}

	void Remove(const _MyIterType &iter)
	{
		CRBTree::Delete(iter.GetKey());
	}

    _MyIterType Find(_MyKeyConstRefType key)
	{
		typename _MyParentType::_MyNodeptr pNode = CRBTree::Find(key);
		if (pNode != nullptr)
			return _MyIterType(this, pNode);
		else
			return _MyIterType();
	}

	_MyConstIterType Find(_MyKeyConstRefType key) const
	{
		typename _MyParentType::_MyNodeptr pNode = CRBTree::Find(key);
		if (pNode != nullptr)
			return _MyConstIterType(this, pNode);
		else
			return _MyConstIterType();
	}

	void Clear()
	{
		CRBTree::Clear();
	}

	_MyIterType CreateIterator()
	{
		return CRBTree::IsEmpty() ? _MyIterType() : _MyIterType(this, CRBTree::Minimum(CRBTree::GetRoot()));
	}

	_MyConstIterType CreateConstIterator()
	{
		return CRBTree::IsEmpty() ? _MyConstIterType() :_MyConstIterType(this, CRBTree::Minimum(CRBTree::GetRoot()));
	}
};