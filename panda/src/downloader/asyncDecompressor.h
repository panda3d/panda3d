// Filename: asyncDecompressor.h
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
#ifndef ASYNCDECOMPRESSOR_H
#define ASYNCDECOMPRESSOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <filename.h>
#include <tokenBoard.h>
#include <buffer.h>
#include "zcompressor.h"
#include "asyncUtility.h"

class DecompressorToken;

////////////////////////////////////////////////////////////////////
//       Class : Decompressor
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Decompressor : public AsyncUtility {
PUBLISHED:
  Decompressor(void);
  Decompressor(PT(Buffer) buffer);
  virtual ~Decompressor(void);

  int request_decompress(const Filename &source_file,
                         const string &event_name);
  int request_decompress(const Filename &source_file,
                         const Filename &dest_file,
                         const string &event_name);

  bool decompress(Filename &source_file);
  bool decompress(Filename &source_file, Filename &dest_file);

private:
  void init(PT(Buffer) buffer);
  virtual bool process_request(void);

  typedef TokenBoard<DecompressorToken> DecompressorTokenBoard;
  DecompressorTokenBoard *_token_board;

  PT(Buffer) _buffer;
  int _half_buffer_length;
  Filename _temp_file_name;
};

#endif
