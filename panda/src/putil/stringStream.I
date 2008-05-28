// Filename: stringStream.I
// Created by:  drose (03Jul07)
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
  set_data(source);
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
  return _buf.get_data().size();
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::get_data
//       Access: Published
//  Description: Returns the contents of the data stream as a string.
////////////////////////////////////////////////////////////////////
INLINE string StringStream::
get_data() {
  flush();
  const pvector<unsigned char> &data = _buf.get_data();
  if (!data.empty()) {
    return string((char *)&data[0], data.size());
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::set_data
//       Access: Published
//  Description: Replaces the contents of the data stream.  This
//               implicitly reseeks to 0.
////////////////////////////////////////////////////////////////////
INLINE void StringStream::
set_data(const string &data) {
  _buf.clear();
  if (!data.empty()) {
    pvector<unsigned char> pv;
    pv.insert(pv.end(), (const unsigned char *)&data[0], (const unsigned char *)&data[0] + data.size());
    _buf.swap_data(pv);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StringStream::swap_data
//       Access: Published
//  Description: Swaps the indicated buffer for the contents of the
//               internal buffer.
////////////////////////////////////////////////////////////////////
INLINE void StringStream::
swap_data(pvector<unsigned char> &data) {
  flush();
  _buf.swap_data(data);
}
