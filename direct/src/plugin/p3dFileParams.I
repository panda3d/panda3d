// Filename: p3dFileParams.I
// Created by:  drose (23Jun09)
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
//     Function: P3DFileParams::get_p3d_filename
//       Access: Public
//  Description: Returns the filename that was passed to
//               set_p3d_filename().
////////////////////////////////////////////////////////////////////
inline const string &P3DFileParams::
get_p3d_filename() const {
  return _p3d_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::get_p3d_offset
//       Access: Public
//  Description: Returns the p3d file offset, the location
//               in the file where the p3d data starts.
////////////////////////////////////////////////////////////////////
inline int P3DFileParams::
get_p3d_offset() const {
  return _p3d_offset;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::get_p3d_url
//       Access: Public
//  Description: Returns the string that was passed to set_p3d_url().
////////////////////////////////////////////////////////////////////
inline const string &P3DFileParams::
get_p3d_url() const {
  return _p3d_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::get_num_tokens
//       Access: Public
//  Description: Returns the number of tokens in the params.
////////////////////////////////////////////////////////////////////
inline int P3DFileParams::
get_num_tokens() const {
  return _tokens.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::get_token_keyword
//       Access: Public
//  Description: Returns the keyword of the nth token.
////////////////////////////////////////////////////////////////////
inline const string &P3DFileParams::
get_token_keyword(int n) const {
  assert(n >= 0 && n < (int)_tokens.size());
  return _tokens[n]._keyword;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DFileParams::get_token_value
//       Access: Public
//  Description: Returns the value of the nth token.
////////////////////////////////////////////////////////////////////
inline const string &P3DFileParams::
get_token_value(int n) const {
  assert(n >= 0 && n < (int)_tokens.size());
  return _tokens[n]._value;
}
