// Filename: textureAttribute.cxx
// Created by:  drose (22Mar00)
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

#include "textureAttribute.h"
#include "textureTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle TextureAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle TextureAttribute::
get_handle() const {
  return TextureTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureAttribute::
make_copy() const {
  return new TextureAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureAttribute::
make_initial() const {
  return new TextureAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void TextureAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_texture(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the indicated transition
//               pointer, which is guaranteed to be of type
//               TextureTransition.
////////////////////////////////////////////////////////////////////
void TextureAttribute::
set_value_from(const OnOffTransition *other) {
  const TextureTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (Texture *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int TextureAttribute::
compare_values(const OnOffAttribute *other) const {
  const TextureAttribute *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value - ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TextureAttribute::
output_value(ostream &out) const {
  nassertv(_value != (Texture *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TextureAttribute::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (Texture *)NULL);
  indent(out, indent_level) << *_value << "\n";
}
