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
// Description : Applys a patch synchronously 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patcher {
PUBLISHED:
  enum PatcherStatus {
    PS_success = 1,
    PS_error = -1,
  };

  Patcher(void);
  Patcher(PT(Buffer) buffer);
  virtual ~Patcher(void);

  int initialize(Filename &patch, Filename &infile);
  int run(void);

private:
  void init(PT(Buffer) buffer);
  PT(Buffer) _buffer;
};

#endif
