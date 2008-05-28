// Filename: zStream.I
// Created by:  drose (05Aug02)
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


////////////////////////////////////////////////////////////////////
//     Function: IDecompressStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecompressStream::
IDecompressStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: IDecompressStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE IDecompressStream::
IDecompressStream(istream *source, bool owns_source) : istream(&_buf) {
  open(source, owns_source);
}

////////////////////////////////////////////////////////////////////
//     Function: IDecompressStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE IDecompressStream &IDecompressStream::
open(istream *source, bool owns_source) {
  clear((ios_iostate)0);
  _buf.open_read(source, owns_source);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: IDecompressStream::close
//       Access: Public
//  Description: Resets the ZStream to empty, but does not actually
//               close the source istream unless owns_source was true.
////////////////////////////////////////////////////////////////////
INLINE IDecompressStream &IDecompressStream::
close() {
  _buf.close_read();
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: OCompressStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OCompressStream::
OCompressStream() : ostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: OCompressStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OCompressStream::
OCompressStream(ostream *dest, bool owns_dest, int compression_level) :
  ostream(&_buf) 
{
  open(dest, owns_dest, compression_level);
}

////////////////////////////////////////////////////////////////////
//     Function: OCompressStream::open
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE OCompressStream &OCompressStream::
open(ostream *dest, bool owns_dest, int compression_level) {
  clear((ios_iostate)0);
  _buf.open_write(dest, owns_dest, compression_level);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: OCompressStream::close
//       Access: Public
//  Description: Resets the ZStream to empty, but does not actually
//               close the dest ostream unless owns_dest was true.
////////////////////////////////////////////////////////////////////
INLINE OCompressStream &OCompressStream::
close() {
  _buf.close_write();
  return *this;
}

