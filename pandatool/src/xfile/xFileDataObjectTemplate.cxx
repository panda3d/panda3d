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
XFileDataObjectTemplate(XFile *x_file, const string &name,
                        XFileTemplate *xtemplate) :
  XFileDataObject(x_file, name),
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

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_object
//       Access: Public
//  Description: Adds the indicated object as a nested object
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_object(XFileDataObjectTemplate *object, bool reference) {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_double
//       Access: Public
//  Description: Adds the indicated list of doubles as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_double(PTA_double double_list, char separator) {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_int
//       Access: Public
//  Description: Adds the indicated list of ints as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_int(PTA_int int_list, char separator) {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_string
//       Access: Public
//  Description: Adds the indicated string as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_string(const string &str, char separator) {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_separator
//       Access: Public
//  Description: Adds the indicated separator character as an isolated
//               separator encountered in the parser.  It will later
//               be processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_separator(char separator) {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::finalize_parse_data
//       Access: Public
//  Description: Processes all of the data elements added by
//               add_parse_*(), checks them for syntactic and semantic
//               correctness against the Template definition, and
//               stores the appropriate child data elements.  Returns
//               true on success, false if there is a mismatch.
////////////////////////////////////////////////////////////////////
bool XFileDataObjectTemplate::
finalize_parse_data() {
  return true;
}
