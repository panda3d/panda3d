// Filename: textFont.I
// Created by:  drose (03May01)
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
//     Function: TextFont::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.  This is based on the font.
////////////////////////////////////////////////////////////////////
INLINE float TextFont::
get_line_height() const {
  return _font_height;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::get_line_height
//       Access: Published
//  Description: Returns the number of units high each line of text
//               is.  This is based on the font.
////////////////////////////////////////////////////////////////////
INLINE const TextFont::CharDef *TextFont::
get_char(int character) const {
  CharDefs::const_iterator cdi = _defs.find(character);
  if (cdi == _defs.end()) {
    // No definition for this character.
    return (CharDef *)NULL;
  } else {
    return &(*cdi).second;
  }
}
