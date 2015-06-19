// Filename: pandaIOStream.cxx
// Created by:  rdb (29Mar11)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandaIOStream.h"


////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PandaIOStream::
PandaIOStream(istream &stream) : _istream(stream) {
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::FileSize
//       Access: Public
//  Description: Returns the size of this file.
////////////////////////////////////////////////////////////////////
size_t PandaIOStream::
FileSize() const {
  streampos cur = _istream.tellg();
  _istream.seekg(0, ios::end);
  streampos end = _istream.tellg();
  _istream.seekg(cur, ios::beg);
  return end;
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Flush
//       Access: Public
//  Description: See fflush.
////////////////////////////////////////////////////////////////////
void PandaIOStream::
Flush() {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Read
//       Access: Public
//  Description: See fread.
////////////////////////////////////////////////////////////////////
size_t PandaIOStream::
Read(void *buffer, size_t size, size_t count) {
  _istream.read((char*) buffer, size * count);
  return _istream.gcount();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Seek
//       Access: Public
//  Description: See fseek.
////////////////////////////////////////////////////////////////////
aiReturn PandaIOStream::
Seek(size_t offset, aiOrigin origin) {
  switch (origin) {
  case aiOrigin_SET:
    _istream.seekg(offset, ios::beg);
    break;

  case aiOrigin_CUR:
    _istream.seekg(offset, ios::cur);
    break;

  case aiOrigin_END:
    _istream.seekg(offset, ios::end);
    break;

  default:
    // Keep compiler happy
    nassertr(false, AI_FAILURE);
    break;
  }

  if (_istream.good()) {
    return AI_SUCCESS;
  } else {
    return AI_FAILURE;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Tell
//       Access: Public
//  Description: See ftell.
////////////////////////////////////////////////////////////////////
size_t PandaIOStream::
Tell() const {
  return _istream.tellg();
}

////////////////////////////////////////////////////////////////////
//     Function: PandaIOStream::Write
//       Access: Public
//  Description: See fwrite.
////////////////////////////////////////////////////////////////////
size_t PandaIOStream::
Write(const void *buffer, size_t size, size_t count) {
  nassertr(false, 0);
  return 0;
}
