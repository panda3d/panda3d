#define MEMBUF_THRASH_SIZE  25

/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::ClearBuffer
// Description	    : Releases all resources(Memory USed) is locally allocated
// Return type		: inline void 
// Argument         : void
//////////////////////////////////////////////////////////
inline void MemBuffer::ClearBuffer(void)
{
	if(_BufferLocal == true)
	{
		if(_Buffer != NULL)
			delete [] _Buffer;
		
		_Buffer = NULL;
	}		
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::AllocBuffer
// Description	    : Locally allocate a new buffer	
// Return type		: inline void 
// Argument         : int len
//////////////////////////////////////////////////////////
inline void MemBuffer::AllocBuffer(size_t len)
{		
	_Buffer = new char[len];
	_BufferLocal = true;
	_BufferLen = len;
}

/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::MemBuffer
// Description	    : default constructor 
// Return type		: 
// Argument         : void
//////////////////////////////////////////////////////////
inline MemBuffer::MemBuffer(void)
{
	_Buffer = NULL;
	_BufferLocal = false;
	_BufferLen = 0;
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::MemBuffer
// Description	    : Constructure to locall allocate a buffer
// Return type		: 
// Argument         : int len
//////////////////////////////////////////////////////////
inline MemBuffer::MemBuffer(size_t len)
{
	AllocBuffer(len);
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::MemBuffer
// Description	    : Constructure to use an external buffer
// Return type		: 
// Argument         : char * data
// Argument         : int len
//////////////////////////////////////////////////////////
inline MemBuffer::MemBuffer(char * data, size_t len)
{
	_BufferLocal = false;
	_BufferLen = len;
	_Buffer = data;
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::~MemBuffer
// Description	    : CLean UP a mess on Deletetion
// Return type		: 
//////////////////////////////////////////////////////////
inline MemBuffer::~MemBuffer()
{
	ClearBuffer();
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::SetBuffer
// Description	    : Assigne a buffer
// Return type		: inline void 
// Argument         : char * data
// Argument         : int len
//////////////////////////////////////////////////////////
inline void MemBuffer::SetBuffer(char * data, size_t len)
{
	if(_BufferLocal == true)
		ClearBuffer();
	
	_BufferLocal = false;
	_BufferLen = len;
	_Buffer = data;		
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::GrowBuffer
// Description	    :  Grow a buffer is needed to get to a sertion size
//                       No care is made here to preserve convtent unlike a vector of chars
//
// Return type		: inline void 
// Argument         : int len
//////////////////////////////////////////////////////////
inline void MemBuffer::GrowBuffer(size_t new_len)
{
	if(new_len >= _BufferLen)
	{
		size_t len = new_len + MEMBUF_THRASH_SIZE;
		len = len +len;

		char * tmp =  new char[len];

		if(_Buffer != NULL)
			memcpy(tmp,_Buffer,_BufferLen);

		ClearBuffer();

		_Buffer = tmp;
		_BufferLocal = true;
		_BufferLen = len;
	}
}
/////////////////////////////////////////////////////////////
// Function name	: MemBuffer::GetBufferSize
// Description	    : Access to the BUffer Size Information
// Return type		: inline int 
// Argument         : void
//////////////////////////////////////////////////////////
inline size_t MemBuffer::GetBufferSize(void )  const
{
	return  _BufferLen; 
};
/////////////////////////////////////////////////////////////
// Function name	: * MemBuffer::GetBuffer
// Description	    :  Access to the actual BUffer
// Return type		: inline char 
// Argument         : void
//////////////////////////////////////////////////////////
inline char * MemBuffer::GetBuffer(void) 
{
	return  _Buffer; 
};
inline const char * MemBuffer::GetBuffer(void) const
{
	return  _Buffer; 
};

////////////////////////////////////////////////////////////////////
// Function name	: MemBuffer::InBufferRange
// Description	    : 
//  
// Return type		: inline bool 
// Argument         : char * inpos
////////////////////////////////////////////////////////////////////
inline bool MemBuffer::InBufferRange(char * inpos)
{
	return (inpos >= _Buffer && inpos <= (_Buffer + _BufferLen));
}

