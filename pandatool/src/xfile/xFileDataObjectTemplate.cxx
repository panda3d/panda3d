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
#include "xFileParseData.h"
#include "xLexerDefs.h"

TypeHandle XFileDataObjectTemplate::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileDataObjectTemplate::
XFileDataObjectTemplate(XFile *x_file, const string &name,
                        XFileTemplate *xtemplate) :
  XFileDataNode(x_file, name),
  _template(xtemplate)
{
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::is_complex_object
//       Access: Public, Virtual
//  Description: Returns true if this kind of data object is a complex
//               object that can hold nested data elements, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool XFileDataObjectTemplate::
is_complex_object() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_double
//       Access: Public
//  Description: Adds the indicated list of doubles as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_double(PTA_double double_list) {
  XFileParseData pdata;
  pdata._double_list = double_list;
  pdata._parse_flags = XFileParseData::PF_double;

  _parse_data_list._list.push_back(pdata);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_int
//       Access: Public
//  Description: Adds the indicated list of ints as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_int(PTA_int int_list) {
  XFileParseData pdata;
  pdata._int_list = int_list;
  pdata._parse_flags = XFileParseData::PF_int;

  _parse_data_list._list.push_back(pdata);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_parse_string
//       Access: Public
//  Description: Adds the indicated string as a data element
//               encountered in the parser.  It will later be
//               processed by finalize_parse_data().
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
add_parse_string(const string &str) {
  XFileParseData pdata;
  pdata._string = str;
  pdata._parse_flags = XFileParseData::PF_string;

  _parse_data_list._list.push_back(pdata);
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
  // Recursively walk through our template definition, while
  // simultaneously walking through the list of parse data elements we
  // encountered, and re-pack them as actual nested elements.
  PrevData prev_data;
  size_t index = 0;
  size_t sub_index = 0;

  if (!_template->repack_data(this, _parse_data_list, 
                              prev_data, index, sub_index)) {
    return false;
  }

  if (index != _parse_data_list._list.size()) {
    xyyerror("Too many data elements in structure.");
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::add_element
//       Access: Public, Virtual
//  Description: Adds the indicated element as a nested data element,
//               if this data object type supports it.  Returns true
//               if added successfully, false if the data object type
//               does not support nested data elements.
////////////////////////////////////////////////////////////////////
bool XFileDataObjectTemplate::
add_element(XFileDataObject *element) {
  _nested_elements.push_back(element);
  return true;
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

  int num_elements = get_num_elements();
  for (int i = 0; i < num_elements; i++) {
    get_element(i)->write_data(out, indent_level + 2, ";");
  }

  XFileNode::write_text(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::write_data
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFileDataObjectTemplate::
write_data(ostream &out, int indent_level, const char *separator) const {
  if (!_nested_elements.empty()) {
    bool indented = false;
    for (size_t i = 0; i < _nested_elements.size() - 1; i++) {
      XFileDataObject *object = _nested_elements[i];
      if (object->is_complex_object()) {
        // If we have a "complex" nested object, output it on its own
        // line.
        if (indented) {
          out << "\n";
          indented = false;
        }
        object->write_data(out, indent_level, ";");

      } else {
        // Otherwise, output them all on the same line.
        if (!indented) {
          indent(out, indent_level);
          indented = true;
        }
        out << *object << "; ";
      }
    }

    // The last object is the set is different, because it gets
    // separator appended to it, and it always gets a newline.
    XFileDataObject *object = _nested_elements.back();
    if (object->is_complex_object()) {
      if (indented) {
        out << "\n";
      }
      string combined_separator = string(";") + string(separator);
      object->write_data(out, indent_level, combined_separator.c_str());

    } else {
      if (!indented) {
        indent(out, indent_level);
      }
      out << *object << ";" << separator << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::get_num_elements
//       Access: Protected, Virtual
//  Description: Returns the number of nested data elements within the
//               object.  This may be, e.g. the size of the array, if
//               it is an array.
////////////////////////////////////////////////////////////////////
int XFileDataObjectTemplate::
get_num_elements() const {
  return _nested_elements.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::get_element
//       Access: Protected, Virtual
//  Description: Returns the nth nested data element within the
//               object.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataObjectTemplate::
get_element(int n) const {
  nassertr(n >= 0 && n < (int)_nested_elements.size(), NULL);
  return _nested_elements[n];
}

////////////////////////////////////////////////////////////////////
//     Function: XFileDataObjectTemplate::get_element
//       Access: Protected, Virtual
//  Description: Returns the nested data element within the
//               object that has the indicated name.
////////////////////////////////////////////////////////////////////
const XFileDataObject *XFileDataObjectTemplate::
get_element(const string &name) const {
  return NULL;
}
