// Filename: renderTraverser.cxx
// Created by:  drose (12Apr00)
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

#include "renderTraverser.h"

#include <indent.h>

TypeHandle RenderTraverser::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RenderTraverser::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderTraverser::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void RenderTraverser::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}
