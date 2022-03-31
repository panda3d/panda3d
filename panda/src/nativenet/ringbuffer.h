#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "membuffer.h"

class EXPCL_PANDA_NATIVENET RingBuffer : protected MemBuffer {
protected:
  size_t _StartPos;
  size_t _EndPos;
  inline char *GetMessageHead(void);
  inline char *GetBufferOpen(void);
  inline void ForceWindowSlide(void);

public:
  inline size_t AmountBuffered(void);
  inline size_t BufferAvailable(void);
  inline void ResetContent(void);

  inline RingBuffer(size_t in_size = 4096);
  inline void FullCompress(void);
  inline void Compress(void);
  inline bool Put(const char * data, size_t len);
  inline bool Get(char * data, size_t len);
};

#include "ringbuffer.I"

#endif // RINGBUFFER_H
