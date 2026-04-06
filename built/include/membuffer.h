#ifndef MEMBUFFER_H
#define MEMBUFFER_H

class EXPCL_PANDA_NATIVENET MemBuffer {
public:
  inline      MemBuffer(void);
  inline      MemBuffer(size_t len);
  inline      MemBuffer(char * data, size_t len);
  virtual ~MemBuffer();
  inline void SetBuffer(char * data, size_t len);
  inline void GrowBuffer(size_t len);
  inline size_t  GetBufferSize(void ) const;
  inline char * GetBuffer(void);
  inline const char * GetBuffer(void) const;
  inline bool InBufferRange(char * );

protected:
  bool        _BufferLocal;  // indicates responsibility of managment of the data
  size_t          _BufferLen; // the length of the data
  char    *   _Buffer;        // the data

  inline void ClearBuffer(void);
  inline void AllocBuffer(size_t len);
};

#include "membuffer.I"

#endif // MEMBUFFER_H
