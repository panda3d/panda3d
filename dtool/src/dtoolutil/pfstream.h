// Filename: pfstream.h
// Created by:  cary (27Aug98)
//
////////////////////////////////////////////////////////////////////

#ifndef __PFSTREAM_H__
#define __PFSTREAM_H__

#include "pfstreamBuf.h"

class EXPCL_DTOOL IPipeStream : public istream {
PUBLISHED:
  INLINE IPipeStream(const std::string);

  INLINE void flush(void);
private:
  PipeStreamBuf _psb;

  INLINE IPipeStream(void);
};

class EXPCL_DTOOL OPipeStream : public ostream {
PUBLISHED:
  INLINE OPipeStream(const std::string);

  INLINE void flush(void);
private:
  PipeStreamBuf _psb;

  INLINE OPipeStream(void);
};

#include "pfstream.I"

#endif /* __PFSTREAM_H__ */


