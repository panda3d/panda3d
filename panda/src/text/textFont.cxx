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
#include "string_utils.h"
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

////////////////////////////////////////////////////////////////////
//     Function: TextFont::string_render_mode
//       Access: Public
//  Description: Returns the RenderMode value associated with the given
//               string representation, or RM_invalid if the string
//               does not match any known RenderMode value.
////////////////////////////////////////////////////////////////////
TextFont::RenderMode TextFont::
string_render_mode(const string &string) {
  if (cmp_nocase_uh(string, "texture") == 0) {
    return RM_texture;
  } else if (cmp_nocase_uh(string, "wireframe") == 0) {
    return RM_wireframe;
  } else if (cmp_nocase_uh(string, "polygon") == 0) {
    return RM_polygon;
  } else if (cmp_nocase_uh(string, "extruded") == 0) {
    return RM_extruded;
  } else if (cmp_nocase_uh(string, "solid") == 0) {
    return RM_solid;
  } else {
    return RM_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::string_winding_order
//       Access: Public
//  Description: Returns the WindingOrder value associated with the given
//               string representation, or WO_invalid if the string
//               does not match any known WindingOrder value.
////////////////////////////////////////////////////////////////////
TextFont::WindingOrder TextFont::
string_winding_order(const string &string) {
  if (cmp_nocase_uh(string, "default") == 0) {
    return WO_default;
  } else if (cmp_nocase_uh(string, "left") == 0) {
    return WO_left;
  } else if (cmp_nocase_uh(string, "right") == 0) {
    return WO_right;
  } else {
    return WO_invalid;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::RenderMode output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, TextFont::RenderMode rm) {
  switch (rm) {
  case TextFont::RM_texture:
    return out << "texture";
  case TextFont::RM_wireframe:
    return out << "wireframe";
  case TextFont::RM_polygon:
    return out << "polygon";
  case TextFont::RM_extruded:
    return out << "extruded";
  case TextFont::RM_solid:
    return out << "solid";

  case TextFont::RM_invalid:
    return out << "invalid";
  }

  return out << "(**invalid TextFont::RenderMode(" << (int)rm << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::RenderMode input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, TextFont::RenderMode &rm) {
  string word;
  in >> word;

  rm = TextFont::string_render_mode(word);
  return in;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::WindingOrder output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, TextFont::WindingOrder wo) {
  switch (wo) {
  case TextFont::WO_default:
    return out << "default";
  case TextFont::WO_left:
    return out << "left";
  case TextFont::WO_right:
    return out << "right";

  case TextFont::WO_invalid:
    return out << "invalid";
  }

  return out << "(**invalid TextFont::WindingOrder(" << (int)wo << ")**)";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::WindingOrder input operator
//  Description:
////////////////////////////////////////////////////////////////////
istream &
operator >> (istream &in, TextFont::WindingOrder &wo) {
  string word;
  in >> word;

  wo = TextFont::string_winding_order(word);
  return in;
}
