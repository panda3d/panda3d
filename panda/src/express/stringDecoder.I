// Filename: stringDecoder.I
// Created by:  drose (11Feb02)
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
//     Function: StringDecoder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE StringDecoder::
StringDecoder(const string &input) : _input(input) {
  _p = 0;
  _eof = false;
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
  return _eof;
}

////////////////////////////////////////////////////////////////////
//     Function: StringDecoder::test_eof
//       Access: Protected
//  Description: If the pointer is past the last character of the
//               string, set the eof flag and return true.
////////////////////////////////////////////////////////////////////
INLINE bool StringDecoder::
test_eof() {
  if (_p >= _input.size()) {
    _eof = true;
    return true;
  }
  return false;
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
