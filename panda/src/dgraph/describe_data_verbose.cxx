// Filename: describe_data_verbose.cxx
// Created by:  drose (27Mar00)
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

#include "describe_data_verbose.h"
#include "dataGraphTraversal.h"

#include <indent.h>
#include <nodeAttributes.h>

// The number of columns in from the start of the name to print the
// data value.
static const int data_indent_level = 12;


////////////////////////////////////////////////////////////////////
//     Function: describe_data_verbose
//  Description: Writes to the indicated output stream a
//               nicely-formatted, multi-line description of all the
//               data values included in the indicated state.
////////////////////////////////////////////////////////////////////
void
describe_data_verbose(ostream &out, const NodeAttributes &state,
                      int indent_level) {
  NodeAttributes::const_iterator nai;

  for (nai = state.begin(); nai != state.end(); ++nai) {
    const PT(NodeAttribute) &attrib = (*nai).second;
    if (attrib != (NodeAttribute *)NULL) {
      // Now extract the type name out of the type flag.
      TypeHandle type = (*nai).first;
      string actual_name = type.get_name();
      string::size_type underscore = actual_name.rfind('_');
      string name;
      if (underscore == string::npos) {
        // There was no underscore, so this name wasn't created via
        // register_data_transition().  Huh.  Well, that's legitimate
        // (if unexpected), so just print the whole name.
        name = actual_name;
      } else {
        // Assume that, if the name was created via
        // register_data_transition(), the original name was the part
        // before the last underscore.
        name = actual_name.substr(0, underscore);
      }

      indent(out, indent_level) << name;
      if ((int)name.length() < data_indent_level) {
        indent(out, data_indent_level - name.length());
      } else {
        out << "  ";
      }
      out << *attrib << "\n";
    }
  }
}

