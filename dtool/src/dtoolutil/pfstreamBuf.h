// Filename: pfstreamBuf.h
// Created by:  cary (12Dec00)
// 
////////////////////////////////////////////////////////////////////

#ifndef __PFSTREAMBUF_H__
#define __PFSTREAMBUF_H__

#include <dtoolbase.h>
#include <iostream>

class EXPCL_PANDA PipeStreamBuf : public streambuf {
public:
  PipeStreamBuf(void);
  virtual ~PipeStreamBuf(void);

  void flush();
protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();
private:
};

#endif /* __PFSTREAMBUF_H__ */
