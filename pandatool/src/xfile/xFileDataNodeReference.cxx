/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataNodeReference.cxx
 * @author drose
 * @date 2004-10-08
 */

#include "xFileDataNodeReference.h"
#include "indent.h"

TypeHandle XFileDataNodeReference::_type_handle;

/**
 *
 */
XFileDataNodeReference::
XFileDataNodeReference(XFileDataNodeTemplate *object) :
  XFileDataNode(object->get_x_file(), object->get_name(),
                object->get_template()),
  _object(object)
{
  // We steal a copy of the referenced object's children.  This is just a one-
  // time copy, so if you go and change the list of children of the referenced
  // object, it won't be reflected here in the reference.  Since presumably
  // the reference is only used when parsing static files, that shouldn't be a
  // problem; but you do need to be aware of it.
  _children = object->_children;
  _objects = object->_objects;
  _children_by_name = object->_children_by_name;
}

/**
 * Returns true if this node represents an indirect reference to an object
 * defined previously in the file.  References are generally transparent, so
 * in most cases you never need to call this, unless you actually need to
 * differentiate between references and instances; you can simply use the
 * reference node as if it were itself the object it references.
 *
 * If this returns true, the node must be of type XFileDataNodeReference.
 */
bool XFileDataNodeReference::
is_reference() const {
  return true;
}

/**
 * Returns true if this kind of data object is a complex object that can hold
 * nested data elements, false otherwise.
 */
bool XFileDataNodeReference::
is_complex_object() const {
  return true;
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFileDataNodeReference::
write_text(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "{ " << _object->get_name() << " }\n";
}

/**
 * Returns the number of nested data elements within the object.  This may be,
 * e.g.  the size of the array, if it is an array.
 */
int XFileDataNodeReference::
get_num_elements() const {
  return _object->size();
}

/**
 * Returns the nth nested data element within the object.
 */
XFileDataObject *XFileDataNodeReference::
get_element(int n) {
  return &((*_object)[n]);
}

/**
 * Returns the nested data element within the object that has the indicated
 * name.
 */
XFileDataObject *XFileDataNodeReference::
get_element(const std::string &name) {
  return &((*_object)[name]);
}
