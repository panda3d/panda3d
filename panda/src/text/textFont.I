// Filename: textFont.I
// Created by:  drose (03May01)
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
