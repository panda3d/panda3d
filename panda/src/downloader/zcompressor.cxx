// Filename: zcompressor.cxx
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

// This file is compiled only if we have zlib installed.

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#if defined(WIN32_VC) && !defined(NO_PCH)
#include "downloader_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "config_downloader.h"
#include <zlib.h>
#endif

#include "zcompressor.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: ZCompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ZCompressor::
ZCompressor(void) {
  _stream = new z_stream;
  reset_stream(_stream);
  int ret = deflateInit(_stream, 9);
  if (ret == Z_OK)
    downloader_cat.debug()
      << "ZCompressor::ZCompressor() - Zlib deflate initialized" << endl;
  else
    handle_zerror(ret, _stream);
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ZCompressor::
~ZCompressor(void) {
  handle_zerror(deflateEnd(_stream), _stream);
  delete _stream;
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressor::compress
//       Access: Public
//  Description: Compress from next_in into next_out until avail_in
//               is 0 or avail_out is 0 or an error occurs.
////////////////////////////////////////////////////////////////////
int ZCompressor::
compress(char *&next_in, int &avail_in, char *&next_out, int &avail_out,
                                        bool finish) {

  int fin_flag = (finish == true) ? Z_FINISH : Z_NO_FLUSH;

  put_stream(_stream, next_in, avail_in, next_out, avail_out);
  int ret = handle_zerror(deflate(_stream, fin_flag), _stream);
  get_stream(_stream, next_in, avail_in, next_out, avail_out);

  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressor::compress_to_stream
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int ZCompressor::
compress_to_stream(char *&next_in, int &avail_in, char *&next_out,
                int &avail_out, char *out_buffer,
                int out_buffer_length, ofstream &write_stream,
                bool finish) {
  int ret = compress(next_in, avail_in, next_out, avail_out, finish);
  if (ret == S_error)
    return ret;

  // See if there is any output to write
  int out_bytes = out_buffer_length - avail_out;
  if (out_bytes > 0) {
    write_stream.write(out_buffer, out_bytes);
    next_out = out_buffer;
    avail_out = out_buffer_length;
  }

  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: ZDecompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ZDecompressor::
ZDecompressor(void) {
  _stream = new z_stream;
  reset_stream(_stream);
  int ret = inflateInit(_stream);
  if (ret == Z_OK)
    downloader_cat.debug()
      << "ZDecompressor::ZDecompressor() - Zlib inflate initialized" << endl;
  else
    handle_zerror(ret, _stream);
}

////////////////////////////////////////////////////////////////////
//     Function: ZDecompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ZDecompressor::
~ZDecompressor(void) {
  handle_zerror(inflateEnd(_stream), _stream);
}

////////////////////////////////////////////////////////////////////
//     Function: ZDecompressor::decompress
//       Access: Public
//  Description: Decompress from next_in into next_out until avail_in
//               is 0 or avail_out is 0 or an error occurs.
////////////////////////////////////////////////////////////////////
int ZDecompressor::
decompress(char *&next_in, int &avail_in, char *&next_out, int &avail_out,
                                        bool finish) {

  int fin_flag = (finish == true) ? Z_FINISH : Z_NO_FLUSH;

  put_stream(_stream, next_in, avail_in, next_out, avail_out);
  int ret = handle_zerror(inflate(_stream, fin_flag), _stream);
  get_stream(_stream, next_in, avail_in, next_out, avail_out);

  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: ZDecompressor::decompress_to_stream
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int ZDecompressor::
decompress_to_stream(char *&next_in, int &avail_in, char *&next_out,
                int &avail_out, char *out_buffer,
                int out_buffer_length, ostream &write_stream,
                bool finish) {
  int ret = decompress(next_in, avail_in, next_out, avail_out, finish);
  if (ret == S_error)
    return ret;

  // See if there is any output to write
  int out_bytes = out_buffer_length - avail_out;
  if (out_bytes > 0) {
    write_stream.write(out_buffer, out_bytes);
    next_out = out_buffer;
    avail_out = out_buffer_length;
  }

  return ret;
}

////////////////////////////////////////////////////////////////////
//     Function: ZCompressorBase::handle_zerror
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
int ZCompressorBase::
handle_zerror(int code, const z_stream *strm) const {
  if (code == Z_OK)
    return S_ok;
  else if (code == Z_STREAM_END)
    return S_finished;
  else {
    downloader_cat.error()
      << "ZCompressorBase::handle_zerror() - ";
    switch (code) {
      case Z_DATA_ERROR:
        downloader_cat.error()
          << "corrupt data";
        break;
      case Z_NEED_DICT:
        downloader_cat.error()
          << "need a dictionary";
        break;
      case Z_STREAM_ERROR:
        downloader_cat.error()
          << "next_in or next_out = NULL";
        break;
      case Z_MEM_ERROR:
        downloader_cat.error()
          << "not enough memory";
        break;
      case Z_BUF_ERROR:
        downloader_cat.error()
          << "no progress is possible";
        break;
      case Z_VERSION_ERROR:
        downloader_cat.error()
          << "zlib version conflict";
        break;
      default:
        downloader_cat.error()
          << "unknown error: " << code;
        break;
    }
    if (strm != NULL) {
      if (strm->msg != NULL) {
        downloader_cat.error()
          << " - zlib message: " << strm->msg;
      }
    }
    downloader_cat.error()
      << endl;
  }
  return S_error;
}
