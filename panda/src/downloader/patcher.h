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
#include <tokenBoard.h>
#include <buffer.h>
#include "asyncUtility.h"
#include <patchfile.h>

class PatcherToken;

////////////////////////////////////////////////////////////////////
//       Class : Patcher 
// Description : Applys a patch asynchronously
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patcher : public AsyncUtility {
PUBLISHED:
  Patcher(void);
  Patcher(PT(Buffer) buffer);
  virtual ~Patcher(void);

  int request_patch(const Filename &patch,
		    const Filename &infile, const string &event_name);

  bool patch(Filename &patch, Filename &infile);

private:
  void init(PT(Buffer) buffer);
  virtual bool process_request(void);

  typedef TokenBoard<PatcherToken> PatcherTokenBoard;
  PatcherTokenBoard *_token_board;

  PT(Buffer) _buffer;
};

#endif
