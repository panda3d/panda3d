// Filename: xFileTemplate.cxx
// Created by:  drose (03Oct04)
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

#include "xFileTemplate.h"
#include "indent.h"

TypeHandle XFileTemplate::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileTemplate::
XFileTemplate(const string &name, const WindowsGuid &guid) : 
  XFileNode(name),
  _guid(guid)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileTemplate::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileTemplate::
write_text(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "template " << get_name() << " {\n";
  indent(out, indent_level + 2)
    << "<" << _guid << ">\n";

  XFileNode::write_text(out, indent_level + 2);

  indent(out, indent_level)
    << "}\n";
}
