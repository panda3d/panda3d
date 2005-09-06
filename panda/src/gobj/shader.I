// Filename: shader.I
// Heavily Modified:  jyelon (Sep05)
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
//  Function: Shader::get_text
//  Access: Public
//  Description: Return the shader's text.
////////////////////////////////////////////////////////////////////
INLINE const string &Shader::
get_text() {
  return _text;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::get_file
//  Access: Public
//  Description: Return the shader's filename, or null string.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Shader::
get_file() {
  return _file;
}

