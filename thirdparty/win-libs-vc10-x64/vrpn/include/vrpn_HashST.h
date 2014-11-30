/*
 * vrpn_HashST.H
 *
 */


#ifndef _VRPN_HASHST_H
#define _VRPN_HASHST_H

inline unsigned int vrpn_LinearUnsignedIntHashFunction(const unsigned int &i)
{
	return i;
}

template <class T>
inline unsigned int vrpn_LinearHashFunction(const T &value)
{
	return (unsigned int)value;
}

//! Hash class (not thread-safe)
/**
 * This class implements a NON thread-safe template Hash. Both the key as the value are templates.
 * It is possible to iterate over this Hash, but no guarantee is given about the order in which the items are returned.
 * All keys must be unique.
 * \attention In order to use the \e find function, the value template must support a cast from 0.
 * \author Joan De Boeck
 * \author Chris Raymaekers
 */
template <class TKey,class TValue>
class vrpn_Hash
{
public:
	//! constructor
	vrpn_Hash(int init=16);
	//! constructor
	vrpn_Hash(unsigned int (*func)(const TKey &key), int init=16);
	//! destructor
	virtual ~vrpn_Hash();
	//! clears the Hash
	void Clear();

	//! returns the number of items in the Hash
	unsigned int GetNrItems() const	{ return m_NrItems; }
	//! returns the value that belongs to this key
	TValue& Find(const TKey &key);
	//! returns the value that belongs to this key
	const TValue& Find(const TKey &key) const;
	//! checks if the Hash contains a value and returns its key
	bool IsPresent(const TValue &value, TKey &key) const;

	bool MoveFirst() const;			//!< moves an iterator to the first element and returns false if no element is present
	bool MoveNext() const;			//!< moves the iterator to the next element and returns false if no more element is present
	TValue GetCurrentValue() const;	//!< returns the value of the current item
	TKey   GetCurrentKey() const;	//!< returns the key of the current item
	void SetCurrentValue(TValue theValue);	//!< sets the Value of the current key
	bool GetCurrentKeyAndValue(TKey &theKey, TValue &theValue) const;	//!< returns the key and the value of the current item

	bool Add(TKey key, TValue value);	//!< adds a new (key, value) pair, returns true if succeeded
	bool Remove(TKey key);			//!< removes the value that belongs to this key, returns true if succeeded

private:
	//! Hash item struct
	/**
	* This struct implements a Hash item for use in Hash.
	*/
	struct HashItem
	{
		TKey key;		//!< the item's key
		TValue value;	//!< the item's value
		struct HashItem *next;	//!< the next Hash item, used when iterating the Hash
	};

	void ClearItems();	//!< helper function that clears all items in the Hash, for use within a thread safe function
	void ReHash();		//!< reHashes the keys when the Hash size changes
	void MakeNull(HashItem **table,int size);	//!< makes all (key, value) pairs null

	unsigned int m_NrItems;		//!< number of items in the Hash 
	unsigned int m_SizeHash;	//!< current Hash size
	unsigned int m_InitialSize;	//!< initial Hash size

	HashItem **m_Items;	//!< the actual Hash
	HashItem *m_First;	//!< the first Hash item, used for the iterator

	unsigned int (*HashFunction)(const TKey &key);	//!< the function that is used to calculate the Hash values from the keys

	mutable HashItem *m_CurrentItem;		//!< the current Hash item

	unsigned int m_Owner;		//!< the thread ID of the owner
};

/**
 * Constructs a new Hash
 * \param init Hash's initial size and grow size
 */
template <class TKey,class TValue>
vrpn_Hash<TKey, TValue>::vrpn_Hash(int init)
{
	HashFunction = vrpn_LinearHashFunction<TKey>;
	m_NrItems = 0;
	m_InitialSize = m_SizeHash = init;
	m_Items = new HashItem*[m_SizeHash];
	MakeNull( m_Items, m_SizeHash );
	m_CurrentItem = 0;

	m_First=0L;
}

/**
 * Constructs a new Hash
 * \param func the function that used to calculate Hash values from the keys
 * \param init Hash's initial size and grow size
 */
template <class TKey,class TValue>
vrpn_Hash<TKey, TValue>::vrpn_Hash(unsigned int (*func)(const TKey &key), int init)
{
	HashFunction = func;
	m_NrItems = 0;
	m_InitialSize = m_SizeHash = init;
	m_Items = new HashItem*[m_SizeHash];
	MakeNull( m_Items, m_SizeHash );
	m_CurrentItem = 0;

	m_First=0L;
}

template <class TKey,class TValue>
vrpn_Hash<TKey, TValue>::~vrpn_Hash()
{
	
	ClearItems();
	delete[] m_Items;
}

template <class TKey,class TValue>
void vrpn_Hash<TKey, TValue>::Clear()
{
	ClearItems();

	m_NrItems = 0;
	delete m_Items;

	m_SizeHash = m_InitialSize;
	m_Items = new HashItem*[m_SizeHash];
	MakeNull( m_Items, m_SizeHash );
	m_First = 0;
	m_CurrentItem = 0;
}

template <class TKey,class TValue>
TValue& vrpn_Hash<TKey, TValue>::Find(const TKey &key)
{
	TValue zero = 0;
	TValue &result = zero ;
	
	unsigned int HashValue = HashFunction( key ) % m_SizeHash;
	if ( m_Items[ HashValue ] != 0 )
		if ( m_Items[ HashValue ]->key == key )	
		{
			result = m_Items[ HashValue ]->value;	
			m_CurrentItem = m_Items[ HashValue ];
		}

	return result;
}

template <class TKey,class TValue>
const TValue& vrpn_Hash<TKey, TValue>::Find(const TKey &key) const
{
	TValue zero = 0;
	TValue &result = zero ;
	
	unsigned int HashValue = HashFunction( key ) % m_SizeHash;
	if ( m_Items[ HashValue ] != 0 )
		if ( m_Items[ HashValue ]->key == key )	
			result = m_Items[ HashValue ]->value;	

	return result;
}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::IsPresent(const TValue &value, TKey &key) const
{
	bool searching = MoveFirst();

	while( searching )
	{
		if( GetCurrentValue() == value )
		{
			key = GetCurrentKey();
			return true;
		}
		searching = MoveNext();
	}

	return false;

}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::Add(TKey key, TValue value)
{
	bool result;
	TKey theKey;

	unsigned int HashValue = HashFunction( key ) % m_SizeHash;
	HashItem* elementPointer = m_Items[ HashValue ];
	if (elementPointer != 0)
		theKey = m_Items[ HashValue ]->key;

	//---- if object already exists
	if ( elementPointer != 0 )
	{
		if ( theKey == key )
			result = false;
		else
		{
			ReHash();	//--- private function does not have mutex in this class...					

			result = Add( key, value );	//---- mutex must be released here when calling recursively
		}
	}

	//---- if object does not exits in the table
	else
	{
		m_NrItems++;
		HashItem *item = new HashItem; //( HashItem * ) malloc( sizeof( HashItem ) );
		item->key = key;
		item->value = value;
		item->next = m_First;
		m_First = item;
		m_Items[ HashValue ] = item;
		m_CurrentItem = m_Items[ HashValue ];

		result = true;
	}

	return result;
}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::Remove(TKey key)
{
    bool result = false;

    unsigned int HashValue = HashFunction( key ) % m_SizeHash;
    if ( m_Items[ HashValue ] != 0 ) {
	if ( m_Items[ HashValue ]->key == key )	{
		m_NrItems--;
		result = true;

		//--adjust pointers
		if ( m_Items[ HashValue ] == m_First ) {
			m_First = m_First->next;
		} else {
			HashItem *item;
			for ( item = m_First ; item->next != m_Items[ HashValue ]; item = item->next );
			item->next = item->next->next;
		}

		//--free memory
		delete m_Items[ HashValue ]; //free( m_Items[ HashValue ] );
		m_Items[ HashValue ] = 0;
	}
    }
	
    return result;
}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::MoveFirst() const
{
	m_CurrentItem = m_First;

	return ( m_First==NULL ) ? false : true;
}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::MoveNext() const
{
	if ( m_CurrentItem == NULL ) 
		return false;

	m_CurrentItem = m_CurrentItem->next;
	return ( m_CurrentItem != NULL ) ;
}

template <class TKey,class TValue>
TValue vrpn_Hash<TKey, TValue>::GetCurrentValue() const
{
	if ( m_CurrentItem  ) 
		return m_CurrentItem->value;
	else
		return 0;
}

template <class TKey,class TValue>
void vrpn_Hash<TKey, TValue>::SetCurrentValue(TValue theValue) 
{
	if ( m_CurrentItem )
		m_CurrentItem->value=theValue;
}

template <class TKey,class TValue>
TKey vrpn_Hash<TKey, TValue>::GetCurrentKey() const
{

	if ( m_CurrentItem  ) 
		return m_CurrentItem->key;
	else
		return 0;
}

template <class TKey,class TValue>
bool vrpn_Hash<TKey, TValue>::GetCurrentKeyAndValue(TKey &theKey, TValue &theValue) const
{
	bool result;
		
	 
	if ( m_CurrentItem  )
	{
		theKey = m_CurrentItem->key;
		theValue = m_CurrentItem->value;
		return true;
	}
	else
		return false;
}

template <class TKey,class TValue>			//--- these functions do not implement the mutex (this is done in the calling function)
void vrpn_Hash<TKey, TValue>::MakeNull(HashItem **table, int size)
{
	for ( int i = 0 ; i < size ; i++ )
		table[i]=0;
}

template <class TKey,class TValue>
void vrpn_Hash<TKey, TValue>::ReHash()			//--- these functions do not implement the mutex (this is done in the calling function)
{
	HashItem **temp;
	int OldSizeHash = m_SizeHash;
	m_SizeHash *= 2;
	temp = new HashItem*[m_SizeHash];
	MakeNull( temp, m_SizeHash );
	HashItem *NewFirst = 0;
	for ( HashItem *item = m_First ; item != 0 ; item = item->next )
	{
		unsigned int HashValue = HashFunction( item->key )% OldSizeHash;
		HashItem *NewItem = new HashItem;
		NewItem->key = item->key;
		NewItem->value = item->value;
		NewItem->next = NewFirst;
		NewFirst = NewItem;
		HashValue = HashFunction( item->key ) % m_SizeHash;
		temp[ HashValue ] = NewItem;
	}
	ClearItems();
	m_First = NewFirst;

	delete m_Items; //free( m_Items );
	m_Items = temp;
}


template <class TKey,class TValue>
void vrpn_Hash<TKey, TValue>::ClearItems()		
{
	for ( HashItem *item = m_First ; item != 0 ;	)
	{
		HashItem *it = item;
		item = item->next;
		delete it;
	}
	m_CurrentItem = 0;
}

#endif
