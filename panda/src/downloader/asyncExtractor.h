// Filename: asyncExtractor.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#ifndef ASYNCEXTRACTOR_H
#define ASYNCEXTRACTOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <filename.h>
#include <tokenBoard.h>
#include <buffer.h>
#include <multifile.h>
#include "asyncUtility.h"

class ExtractorToken;

////////////////////////////////////////////////////////////////////
//       Class : Extractor
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Extractor : public AsyncUtility {
PUBLISHED:
  Extractor(void);
  Extractor(PT(Buffer) buffer);
  virtual ~Extractor(void);

  int request_extract(const Filename &source_file,
                      const string &event_name, const Filename &rel_path = "");

  bool extract(Filename &source_file, const Filename &rel_path);

private:
  void init(PT(Buffer) buffer);
  virtual bool process_request(void);

  typedef TokenBoard<ExtractorToken> ExtractorTokenBoard;
  ExtractorTokenBoard *_token_board;

  PT(Buffer) _buffer;
};

#endif
