// Filename: zcompressor.h
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
#ifndef ZCOMPRESSOR_H
#define ZCOMPRESSOR_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <zlib.h>
#include "typedef.h"

////////////////////////////////////////////////////////////////////
//       Class : ZCompressorBase
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ZCompressorBase {
public:
  enum status {
    S_ok,
    S_finished,
    S_error,
  };

  INLINE ulong get_total_in(void) const;
  INLINE ulong get_total_out(void) const;
  INLINE void reset(void);
  INLINE void reset_stream(z_stream *strm) const;
#ifndef CPPPARSER
  INLINE void put_stream(z_stream *strm, char *next_in, int avail_in,
                        char *next_out, int avail_out) const;
  INLINE void get_stream(z_stream *strm, char *&next_in, int &avail_in,
                        char *&next_out, int &avail_out) const;
#endif
  int handle_zerror(int code, const z_stream *strm = NULL) const;

protected:
  z_stream *_stream;
};

////////////////////////////////////////////////////////////////////
//       Class : ZCompressor
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ZCompressor : public ZCompressorBase {
public:
  ZCompressor(void);
  ~ZCompressor(void);

  int compress(char *&next_in, int &avail_in, char *&next_out,
                                int &avail_out, bool finish = false);
  int compress_to_stream(char *&next_in, int &avail_in, char *&next_out,
                int &avail_out, char *out_buffer, int out_buffer_length,
                ofstream &write_stream, bool finish = false);
};

////////////////////////////////////////////////////////////////////
//       Class : ZDecompressor
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS ZDecompressor : public ZCompressorBase {
public:
  ZDecompressor(void);
  ~ZDecompressor(void);

  int decompress(char *&next_in, int &avail_in, char *&next_out,
                                int &avail_out, bool finish = false);
  int decompress_to_stream(char *&next_in, int &avail_in, char *&next_out,
                int &avail_out, char *out_buffer, int out_buffer_length,
                ostream &write_stream, bool finish = false);
};

#include "zcompressor.I"

#endif
