// Filename: textFont.cxx
// Created by:  drose (08Feb02)
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

#include "textFont.h"
#include "config_text.h"
#include <ctype.h>

TypeHandle TextFont::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextFont::
TextFont() {
  _is_valid = false;
  _line_height = 1.0f;
  _space_advance = 0.25f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TextFont::
~TextFont() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextFont::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "TextFont " << get_name() << "\n";
}
