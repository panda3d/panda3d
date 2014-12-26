// Filename: textureContext.cxx
// Created by:  drose (07Oct99)
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

#include "textureContext.h"

TypeHandle TextureContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextureContext::
output(ostream &out) const {
  out << *get_texture() << ", " << get_data_size_bytes();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureContext::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextureContext::
write(ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
