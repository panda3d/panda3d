// Filename: stringDecoder.I
// Created by:  drose (11Feb02)
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


////////////////////////////////////////////////////////////////////
//     Function: StringDecoder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StringDecoder::
StringDecoder(const string &input) : _input(input) {
  _p = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: StringDecoder::is_eof
//       Access: Public
//  Description: Returns true if the decoder has returned the last
//               character in the string, false if there are more to
//               go.
////////////////////////////////////////////////////////////////////
INLINE bool StringDecoder::
is_eof() {
  return (_p >= _input.size());
}

////////////////////////////////////////////////////////////////////
//     Function: StringUtf8Decoder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StringUtf8Decoder::
StringUtf8Decoder(const string &input) : StringDecoder(input) {
}

////////////////////////////////////////////////////////////////////
//     Function: StringUnicodeDecoder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StringUnicodeDecoder::
StringUnicodeDecoder(const string &input) : StringDecoder(input) {
}
