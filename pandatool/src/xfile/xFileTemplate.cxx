/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileTemplate.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileTemplate.h"
#include "indent.h"

TypeHandle XFileTemplate::_type_handle;

/**
 *
 */
XFileTemplate::
XFileTemplate(XFile *x_file, const std::string &name, const WindowsGuid &guid) :
  XFileNode(x_file, name),
  _guid(guid),
  _is_standard(false),
  _open(false)
{
}

/**
 *
 */
XFileTemplate::
~XFileTemplate() {
  clear();
}

/**
 * Returns true if this node has a GUID associated.
 */
bool XFileTemplate::
has_guid() const {
  return true;
}

/**
 * Returns the GUID associated with this template.
 */
const WindowsGuid &XFileTemplate::
get_guid() const {
  return _guid;
}

/**
 * Returns true if this node represents the definition of some template.  This
 * is the template definition, not an actual data object that represents an
 * instance of the template.  If the file strictly uses standard templates,
 * the presence of template definitions is optional.
 *
 * If this returns true, the node must be of type XFileTemplate.
 */
bool XFileTemplate::
is_template_def() const {
  return true;
}

/**
 * Removes all children from the node, and otherwise resets it to its initial
 * state.
 */
void XFileTemplate::
clear() {
  XFileNode::clear();
  _options.clear();
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileTemplate::
write_text(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "template " << get_name() << " {\n";
  indent(out, indent_level + 2)
    << "<" << _guid << ">\n";

  XFileNode::write_text(out, indent_level + 2);

  if (get_open()) {
    // An open template
    indent(out, indent_level + 2)
      << "[ ... ]\n";

  } else if (!_options.empty()) {
    // A restricted template
    indent(out, indent_level + 2);

    char delimiter = '[';
    Options::const_iterator ri;
    for (ri = _options.begin(); ri != _options.end(); ++ri) {
      XFileTemplate *option = (*ri);
      out << delimiter << " "
          << option->get_name() << " <" << option->get_guid()
          << ">";
      delimiter = ',';
    }
    out << " ]\n";
  }

  indent(out, indent_level)
    << "}\n";
}

/**
 * Returns true if the node, particularly a template node, is structurally
 * equivalent to the other node (which must be of the same type).  This checks
 * data element types, but does not compare data element names.
 */
bool XFileTemplate::
matches(const XFileNode *other) const {
  if (!XFileNode::matches(other)) {
    return false;
  }

  // We *could* compare the openclosedoptions associated with the template,
  // but since this is only used for validating the set of children for the
  // instances of this template (which we don't even bother to do anyway), it
  // doesn't seem to matter.
  return true;
}
