// Filename: xFileDataObjectTemplate.cxx
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

#include "xFileDataObjectTemplate.h"
#include "indent.h"

TypeHandle XFileDataObjectTemplate::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileDataObjectTemplate::
XFileDataObjectTemplate(XFileTemplate *xtemplate, const string &name) :
  XFileDataObject(name),
  _template(xtemplate)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
write_text(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _template->get_name();
  if (has_name()) {
    out << " " << get_name();
  }
  out << " {\n";
  XFileDataObject::write_text(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}
