// Filename: pfstream.h
// Created by:  cary (27Aug98)
//
////////////////////////////////////////////////////////////////////

#ifndef __PFSTREAM_H__
#define __PFSTREAM_H__

#include "pfstreambuf.h"

class EXPCL_DTOOL ipfstream : public istream {
PUBLISHED:
  INLINE ipfstream(const string);

  INLINE void flush(void);
private:
  PipeStreamBuf _psb;

  INLINE ipfstream(void);
};

class EXPCL_DTOOL opfstream : public ostream {
PUBLISHED:
  INLINE opfstream(const string);

  INLINE void flush(void);
private:
  PipeStreamBuf _psb;

  INLINE opfstream(void);
};

#endif /* __PFSTREAM_H__ */


