// Filename: patcher.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef PATCHER_H
#define PATCHER_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <filename.h>
#include <buffer.h>
#include <patchfile.h>

////////////////////////////////////////////////////////////////////
//       Class : Patcher
// Description : Applies a patch synchronously
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patcher {
PUBLISHED:
  Patcher(void);
  Patcher(PT(Buffer) buffer);
  virtual ~Patcher(void);

  int initiate(Filename &patch, Filename &infile);
  int run(void);

  INLINE float get_progress(void) const;

private:
  void init(PT(Buffer) buffer);
  PT(Buffer) _buffer;
  Patchfile *_patchfile;
};

#include "patcher.I"

#endif
