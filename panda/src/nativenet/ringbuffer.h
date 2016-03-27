#ifndef __RINGBUFFER_GM_H__
#define __RINGBUFFER_GM_H__

#include "membuffer.h"

class EXPCL_PANDA_NATIVENET RingBuffer : protected MemBuffer {
protected:
  size_t _StartPos;
  size_t _EndPos;
  inline char *GetMessageHead(void);
  inline char *GetBufferOpen(void);
  inline void ForceWindowSlide(void);

#define FastGetMessageHead() (_Buffer + _StartPos)
#define FastAmountBeffered() (_EndPos - _StartPos)

  inline bool PutFast(const char * data, size_t len);

public:
  inline size_t AmountBuffered(void);
  inline size_t BufferAvailabe(void);
  inline void ResetContent(void);

  inline RingBuffer(size_t in_size = 4096);
  inline void FullCompress(void);
  inline void Compress(void);
  inline bool Put(const char * data, size_t len);
  inline bool Get(char * data, size_t len);
};

#include "ringbuffer.I"

#endif //__RINGBUFFER_GM_H__
