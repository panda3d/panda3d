// Filename: ambientLight.cxx
// Created by:  mike (09Jan97)
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
#include "ambientLight.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle AmbientLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
AmbientLight::AmbientLight(const string& name) : NamedNode(name)
{
  set_color(Colorf(0.2, 0.2, 0.2, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AmbientLight::
output(ostream &out) const {
  NamedNode::output(out);
}


////////////////////////////////////////////////////////////////////
//     Function: AmbientLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void AmbientLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2) << "color " << _color << "\n";
}

