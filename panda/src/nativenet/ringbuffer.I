
/**
 * This will get a pointer to the fist undelivered data in buffer
 */
inline char  *  RingBuffer::GetMessageHead(void)
{
  return _Buffer+_StartPos;
}
/**
 * This will get the first writabe section of the buffer space
 */
inline char  *  RingBuffer::GetBufferOpen(void)
{
  return _Buffer+_EndPos;
}
/**
 * Will force a compression of data // shift left to start position
 */
inline void RingBuffer::ForceWindowSlide(void)
{
  size_t len = AmountBuffered();
  if(len > 0 && _StartPos != 0)  // basic flush left..
  {
    memmove(_Buffer,GetMessageHead(),len);
    _StartPos = 0;
    _EndPos = len;
  }
}
/**
 * Will report the number of unread chars in buffer
 */
inline size_t    RingBuffer::AmountBuffered(void)
{
  return _EndPos - _StartPos;
}


/**
 * Will report amount of data that is contiguas that can be writen at the
 * location returned by GetBufferOpen
 */
inline size_t      RingBuffer::BufferAvailabe(void)
{
  return GetBufferSize() - _EndPos;
}


/**
 * Throw away all inread information
 */
void RingBuffer::ResetContent(void)
{
  _StartPos = 0;
  _EndPos = 0;
}
/**
 *
 */
inline RingBuffer::RingBuffer(size_t in_size) : MemBuffer(in_size)
{
  _EndPos = 0;
  _StartPos = 0;
}
/**
 * Force a compress of the data
 */
inline void RingBuffer::FullCompress(void)
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
/**
 * Try and do a intelegent compress of the data space the algorithem is really
 * stupid right know.. just say if i have read past 1/2 my space do a
 * compress...Im open for sugestions
 *

 *
 */
inline void RingBuffer::Compress(void)
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
/**
 * Adds Data to a ring Buffer Will do a compress if needed so pointers suplied
 * by Get Call are no longer valide
 *
 */
inline bool RingBuffer::Put(const char * data, size_t len)
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
/**
 *

 *
 */
inline bool RingBuffer::PutFast(const char * data, size_t len)
{
  // no checking be carefull
  memcpy(GetBufferOpen(),data,len); // should i be using memcopy..
  _EndPos += len;
  return true;
}

/**
 * will copy the data .. false indicates not enogh data to read .. sorry...
 *
 */
inline bool RingBuffer::Get(char * data, size_t len)
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
