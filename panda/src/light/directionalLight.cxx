// Filename: directionalLight.cxx
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
#include <pandabase.h>
#include "directionalLight.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle DirectionalLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
DirectionalLight::DirectionalLight(const string& name) : NamedNode(name)
{
  set_color(Colorf(1, 1, 1, 1));
  set_specular(Colorf(1, 1, 1, 1));
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DirectionalLight::
output(ostream &out) const {
  NamedNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: DirectionalLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void DirectionalLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2) << "color " << _color << "\n";
  indent(out, indent_level + 2) << "specular " << _specular << "\n";
}
