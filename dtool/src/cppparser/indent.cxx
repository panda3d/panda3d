// Filename: indent.C
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////


#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: indent
//  Description: A handy function for doing text formatting.  This
//               function simply outputs the indicated number of
//               spaces to the given output stream, returning the
//               stream itself.  Useful for indenting a series of
//               lines of text by a given amount.
////////////////////////////////////////////////////////////////////
ostream &
indent(ostream &out, int indent_level) {
  for (int i = 0; i < indent_level; i++) {
    out << ' ';
  }
  return out;
}
