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
//  Function: Shader::get_filename
//  Access: Public
//  Description: Return the Shader's filename.
////////////////////////////////////////////////////////////////////
INLINE const Filename &Shader::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::get_text
//  Access: Public
//  Description: Return the Shader's text.
////////////////////////////////////////////////////////////////////
INLINE const string &Shader::
get_text() const {
  return _text;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::get_header
//  Access: Public
//  Description: Return the Shader's header line.
////////////////////////////////////////////////////////////////////
INLINE const string &Shader::
get_header() const {
  return _header;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::get_error_flag
//  Access: Public
//  Description: Returns true if the shader contains a compile-time
//               error.  This doesn't tell you whether or not the
//               shader is supported on the current video card.
////////////////////////////////////////////////////////////////////
INLINE bool Shader::
get_error_flag() const {
  return _error_flag;
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderCapabilities Constructor
//  Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Shader::ShaderCaps::
ShaderCaps() {
  clear();
}

////////////////////////////////////////////////////////////////////
//  Function: Shader::ShaderCapabilities::operator ==
//  Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool Shader::ShaderCaps::
operator == (const ShaderCaps &other) const {
#ifdef HAVE_CG
  if ((_active_vprofile != other._active_vprofile) ||
      (_active_fprofile != other._active_fprofile) ||
      (_ultimate_vprofile != other._ultimate_vprofile) ||
      (_ultimate_fprofile != other._ultimate_fprofile)) {
     return false;
  }
#endif
  return true;
}
