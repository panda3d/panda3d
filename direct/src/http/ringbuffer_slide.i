
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::GetMessageHead
// Description      :  This will get a pointer to the fist undelivered data in buffer
// Return type      : char *
// Argument         : void
//////////////////////////////////////////////////////////
inline char *   RingBuffer_Slide::GetMessageHead(void) 
{ 
    return _Buffer+_StartPos;
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::GetBufferOpen
// Description      : This will get the first writabe section of the buffer space
// Return type      : 
// Argument         : void
//////////////////////////////////////////////////////////
inline char *   RingBuffer_Slide::GetBufferOpen(void) 
{
    return _Buffer+_EndPos; 
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::ForceWindowSlide
// Description      :  Will force a compression of data // shift left to start position
// Return type      : inline void 
// Argument         : void
//////////////////////////////////////////////////////////
inline void RingBuffer_Slide::ForceWindowSlide(void)
{
    size_t len = AmountBuffered();
    if(len > 0 && _StartPos != 0)  // basic flush left..
    {
        memmove(_Buffer,GetMessageHead(),len);
        _StartPos = 0;
        _EndPos = len;      
    }
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::AmountBuffered
// Description      : Will report the number of unread chars in buffer
// Return type      : int
// Argument         : void
//////////////////////////////////////////////////////////
inline size_t       RingBuffer_Slide::AmountBuffered(void) 
{ 
    return _EndPos - _StartPos; 
}


/////////////////////////////////////////////////////////////
// Function name    :      RingBuffer_Slide::BufferAvailabe
// Description      : Will report amount of data that is contiguas that can be writen at
//                      the location returned by GetBufferOpen
// Return type      : inline int 
// Argument         : void
//////////////////////////////////////////////////////////
inline size_t      RingBuffer_Slide::BufferAvailabe(void) 
{ 
    return GetBufferSize() - _EndPos; 
}


/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::ResetContent
// Description      : Throw away all inread information
// Return type      : void 
// Argument         : void
//////////////////////////////////////////////////////////
void RingBuffer_Slide::ResetContent(void) 
{ 
    _StartPos = 0; 
    _EndPos = 0; 
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::RingBuffer_Slide
// Description      : 
// Return type      : inline 
// Argument         : int in_size
//////////////////////////////////////////////////////////
inline RingBuffer_Slide::RingBuffer_Slide(size_t in_size) : MemBuffer(in_size)
{           
    _EndPos = 0;
    _StartPos = 0;
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::FullCompress
// Description      : Force a compress of the data
// Return type      : inline void 
// Argument         : void
//////////////////////////////////////////////////////////
inline void RingBuffer_Slide::FullCompress(void)
{
    if(_StartPos == _EndPos)
    {
        _StartPos = 0;
        _EndPos = 0;
    }
    else 
    {
        ForceWindowSlide();
    }   
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::Compress
// Description      : Try and do a intelegent compress of the data space
//                      the algorithem is really stupid right know.. just say if i have 
//                          read past 1/2 my space do a compress...Im open for sugestions
//  
//
// Return type      : inline void 
// Argument         : void
//////////////////////////////////////////////////////////
inline void RingBuffer_Slide::Compress(void)
{
    if(_StartPos == _EndPos)
    {
        _StartPos = 0;
        _EndPos = 0;
    }
    else if(_StartPos >= GetBufferSize() / 2) 
    {
        ForceWindowSlide();
    }   
}
/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::Put
// Description      : Adds Data to a ring Buffer
//                      Will do a compress if needed so pointers suplied by Get Call are no longer valide
//
// Return type      : inline bool 
// Argument         : char * data
// Argument         : int len
//////////////////////////////////////////////////////////
inline bool RingBuffer_Slide::Put(const char * data, size_t len)
{
    bool answer = false;
    
    if(len > BufferAvailabe() )
        Compress();
    
    if(len <= BufferAvailabe() )
    {
        memcpy(GetBufferOpen(),data,len);
        _EndPos += len;
        answer = true;
    }
    return answer;
}
////////////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::PutFast
// Description      : 
//  
// Return type      : inline bool 
// Argument         : const char * data
// Argument         : int len
////////////////////////////////////////////////////////////////////
inline bool RingBuffer_Slide::PutFast(const char * data, size_t len)
{
    // no checking be carefull
    memcpy(GetBufferOpen(),data,len); // should i be using memcopy..
    _EndPos += len;
    return true;
}

/////////////////////////////////////////////////////////////
// Function name    : RingBuffer_Slide::Get
// Description      : will copy the data ..
//              false indicates not enogh data to read .. sorry...
//
// Return type      : inline bool 
// Argument         : char * data
// Argument         : int len
//////////////////////////////////////////////////////////
inline bool RingBuffer_Slide::Get(char * data, size_t len)
{
    bool answer = false;
    
    if(len <= AmountBuffered() )
    {
        memcpy(data,GetMessageHead(),len);
        _StartPos += len;
        Compress();
        answer = true;
    }
    return answer;
}

