// Filename: stringStream.I
// Created by:  drose (03Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: StringStream::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StringStream::
StringStream() : iostream(&_buf) {
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::Constructor
//       Access: Published
//  Description: This version of the constructor preloads the buffer
//               with the indicated data.
////////////////////////////////////////////////////////////////////
INLINE StringStream::
StringStream(const string &source) : iostream(&_buf) {
  write(source);
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::clear_data
//       Access: Published
//  Description: Empties the buffer.
////////////////////////////////////////////////////////////////////
INLINE void StringStream::
clear_data() {
  _buf.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::get_data_size
//       Access: Published
//  Description: Returns the number of characters available to be read
//               from the data stream.
////////////////////////////////////////////////////////////////////
INLINE size_t StringStream::
get_data_size() {
  flush();
  return _buf.get_data_size();
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::read
//       Access: Published
//  Description: Extracts all the remaining characters from the data
//               stream and stores them in the indicated string.
////////////////////////////////////////////////////////////////////
INLINE void StringStream::
read(string &data) {
  read(data, get_data_size());
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::read
//       Access: Published
//  Description: Extracts up to max_length characters from the data
//               stream and returns them as a string.
////////////////////////////////////////////////////////////////////
INLINE string StringStream::
read(size_t max_length) {
  string data;
  read(data, max_length);
  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::read
//       Access: Published
//  Description: Extracts all the remaining characters from the data
//               stream and returns them as a string.
////////////////////////////////////////////////////////////////////
INLINE string StringStream::
read() {
  string data;
  read(data);
  return data;
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::write
//       Access: Published
//  Description: Appends the indicated data to the data stream.
////////////////////////////////////////////////////////////////////
INLINE void StringStream::
write(const string &data) {
  _buf.write_chars(data.data(), data.size());
}
