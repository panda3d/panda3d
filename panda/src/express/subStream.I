// Filename: subStream.I
// Created by:  drose (02Aug02)
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
//     Function: ISubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ISubStream::
ISubStream() : istream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: ISubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ISubStream::
ISubStream(IStreamWrapper *source, streampos start, streampos end) : istream(&_buf) {
  open(source, start, end);
}

////////////////////////////////////////////////////////////////////
//     Function: ISubStream::open
//       Access: Public
//  Description: Starts the SubStream reading from the indicated
//               source, with the first character being the character
//               at position "start" within the source, for end -
//               start total characters.  The character at "end"
//               within the source will never be read; this will
//               appear to be EOF.
//
//               If end is zero, it indicates that the ISubStream will
//               continue until the end of the source stream.
////////////////////////////////////////////////////////////////////
INLINE ISubStream &ISubStream::
open(IStreamWrapper *source, streampos start, streampos end) {
  clear((ios_iostate)0);
  _buf.open(source, NULL, start, end, false);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: ISubStream::close
//       Access: Public
//  Description: Resets the SubStream to empty, but does not actually
//               close the source istream.
////////////////////////////////////////////////////////////////////
INLINE ISubStream &ISubStream::
close() {
  _buf.close();
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: OSubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OSubStream::
OSubStream() : ostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: OSubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE OSubStream::
OSubStream(OStreamWrapper *dest, streampos start, streampos end, bool append) : ostream(&_buf) {
  open(dest, start, end, append);
}

////////////////////////////////////////////////////////////////////
//     Function: OSubStream::open
//       Access: Public
//  Description: Starts the SubStream reading from the indicated
//               dest, with the first character being the character
//               at position "start" within the dest, for end -
//               start total characters.  The character at "end"
//               within the dest will never be read; this will
//               appear to be EOF.
//
//               If end is zero, it indicates that the OSubStream will
//               continue until the end of the dest stream.
////////////////////////////////////////////////////////////////////
INLINE OSubStream &OSubStream::
open(OStreamWrapper *dest, streampos start, streampos end, bool append) {
  clear((ios_iostate)0);
  _buf.open(NULL, dest, start, end, append);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: OSubStream::close
//       Access: Public
//  Description: Resets the SubStream to empty, but does not actually
//               close the dest ostream.
////////////////////////////////////////////////////////////////////
INLINE OSubStream &OSubStream::
close() {
  _buf.close();
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SubStream::
SubStream() : iostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: SubStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SubStream::
SubStream(StreamWrapper *nested, streampos start, streampos end, bool append) : iostream(&_buf) {
  open(nested, start, end, append);
}

////////////////////////////////////////////////////////////////////
//     Function: SubStream::open
//       Access: Public
//  Description: Starts the SubStream reading and writing from the
//               indicated nested stream, within the indicated range.
//               "end" is the first character outside of the range.
//
//               If end is zero, it indicates that the SubStream will
//               continue until the end of the nested stream.
////////////////////////////////////////////////////////////////////
INLINE SubStream &SubStream::
open(StreamWrapper *nested, streampos start, streampos end, bool append) {
  clear((ios_iostate)0);
  _buf.open(nested, nested, start, end, append);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: SubStream::close
//       Access: Public
//  Description: Resets the SubStream to empty, but does not actually
//               close the nested ostream.
////////////////////////////////////////////////////////////////////
INLINE SubStream &SubStream::
close() {
  _buf.close();
  return *this;
}


