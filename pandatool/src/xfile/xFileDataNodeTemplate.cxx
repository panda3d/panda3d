/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataNodeTemplate.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileDataNodeTemplate.h"
#include "indent.h"
#include "xFileParseData.h"
#include "xLexerDefs.h"
#include "config_xfile.h"

using std::string;

TypeHandle XFileDataNodeTemplate::_type_handle;

/**
 *
 */
XFileDataNodeTemplate::
XFileDataNodeTemplate(XFile *x_file, const string &name,
                      XFileTemplate *xtemplate) :
  XFileDataNode(x_file, name, xtemplate)
{
}

/**
 * Fills the data node with zero-valued elements appropriate to the template.
 */
void XFileDataNodeTemplate::
zero_fill() {
  _template->fill_zero_data(this);
}

/**
 * Returns true if this kind of data object is a complex object that can hold
 * nested data elements, false otherwise.
 */
bool XFileDataNodeTemplate::
is_complex_object() const {
  return true;
}

/**
 * Adds the indicated list of doubles as a data element encountered in the
 * parser.  It will later be processed by finalize_parse_data().
 */
void XFileDataNodeTemplate::
add_parse_double(PTA_double double_list) {
  XFileParseData pdata;
  pdata._double_list = double_list;
  pdata._parse_flags = XFileParseData::PF_double;

  _parse_data_list._list.push_back(pdata);
}

/**
 * Adds the indicated list of ints as a data element encountered in the
 * parser.  It will later be processed by finalize_parse_data().
 */
void XFileDataNodeTemplate::
add_parse_int(PTA_int int_list) {
  XFileParseData pdata;
  pdata._int_list = int_list;
  pdata._parse_flags = XFileParseData::PF_int;

  _parse_data_list._list.push_back(pdata);
}

/**
 * Adds the indicated string as a data element encountered in the parser.  It
 * will later be processed by finalize_parse_data().
 */
void XFileDataNodeTemplate::
add_parse_string(const string &str) {
  XFileParseData pdata;
  pdata._string = str;
  pdata._parse_flags = XFileParseData::PF_string;

  _parse_data_list._list.push_back(pdata);
}

/**
 * Processes all of the data elements added by add_parse_*(), checks them for
 * syntactic and semantic correctness against the Template definition, and
 * stores the appropriate child data elements.  Returns true on success, false
 * if there is a mismatch.
 */
bool XFileDataNodeTemplate::
finalize_parse_data() {
  // Recursively walk through our template definition, while simultaneously
  // walking through the list of parse data elements we encountered, and re-
  // pack them as actual nested elements.
  PrevData prev_data;
  size_t index = 0;
  size_t sub_index = 0;

  if (!_template->repack_data(this, _parse_data_list,
                              prev_data, index, sub_index)) {
    return false;
  }

  if (index != _parse_data_list._list.size()) {
    xyywarning("Too many data elements in structure.");
  }

  return true;
}

/**
 * Adds the indicated element as a nested data element, if this data object
 * type supports it.  Returns true if added successfully, false if the data
 * object type does not support nested data elements.
 */
bool XFileDataNodeTemplate::
add_element(XFileDataObject *element) {
  _nested_elements.push_back(element);
  return true;
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataNodeTemplate::
write_text(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _template->get_name();
  if (has_name()) {
    out << " " << get_name();
  }
  out << " {\n";

  NestedElements::const_iterator ni;
  for (ni = _nested_elements.begin(); ni != _nested_elements.end(); ++ni) {
    (*ni)->write_data(out, indent_level + 2, ";");
  }

  XFileNode::write_text(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataNodeTemplate::
write_data(std::ostream &out, int indent_level, const char *separator) const {
  if (!_nested_elements.empty()) {
    bool indented = false;
    for (size_t i = 0; i < _nested_elements.size() - 1; i++) {
      XFileDataObject *object = _nested_elements[i];
      if (object->is_complex_object()) {
        // If we have a "complex" nested object, output it on its own line.
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

    // The last object is the set is different, because it gets separator
    // appended to it, and it always gets a newline.
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

/**
 * Returns the number of nested data elements within the object.  This may be,
 * e.g.  the size of the array, if it is an array.
 */
int XFileDataNodeTemplate::
get_num_elements() const {
  return _nested_elements.size();
}

/**
 * Returns the nth nested data element within the object.
 */
XFileDataObject *XFileDataNodeTemplate::
get_element(int n) {
  nassertr(n >= 0 && n < (int)_nested_elements.size(), nullptr);
  return _nested_elements[n];
}

/**
 * Returns the nested data element within the object that has the indicated
 * name.
 */
XFileDataObject *XFileDataNodeTemplate::
get_element(const string &name) {
  int child_index = _template->find_child_index(name);
  if (child_index >= 0) {
    return get_element(child_index);
  }
  xfile_cat.warning()
    << "\"" << name << "\" not a member of " << _template->get_name()
    << "\n";
  return nullptr;
}
