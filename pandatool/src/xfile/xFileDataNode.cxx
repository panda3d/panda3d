/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataNode.cxx
 * @author drose
 * @date 2004-10-08
 */

#include "xFileDataNode.h"
#include "indent.h"

TypeHandle XFileDataNode::_type_handle;

/**
 *
 */
XFileDataNode::
XFileDataNode(XFile *x_file, const std::string &name,
              XFileTemplate *xtemplate) :
  XFileNode(x_file, name),
  _template(xtemplate)
{
}

/**
 * Returns true if this node represents a data object that is the instance of
 * some template, or false otherwise.  This also returns true for references
 * to objects (which are generally treated just like the objects themselves).
 *
 * If this returns true, the node must be of type XFileDataNode (it is either
 * an XFileDataNodeTemplate or an XFileDataNodeReference).
 */
bool XFileDataNode::
is_object() const {
  return true;
}

/**
 * Returns true if this node represents an instance of the standard template
 * with the indicated name, or false otherwise.  If this returns true, the
 * object must be of type XFileDataNode.
 */
bool XFileDataNode::
is_standard_object(const std::string &template_name) const {
  if (_template->is_standard() &&
      _template->get_name() == template_name) {
    return true;
  }

  return false;
}

/**
 * Returns a string that represents the type of object this data object
 * represents.
 */
std::string XFileDataNode::
get_type_name() const {
  return _template->get_name();
}
