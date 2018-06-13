/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileArrayDef.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFileArrayDef.h"
#include "xFileDataDef.h"
#include "xFileDataObject.h"

/**
 * Returns the size of the array dimension.  If this is a fixed array, the
 * size is trivial; if it is dynamic, the size is determined by looking up the
 * dynamic_size element in the prev_data table (which lists all of the data
 * values already defined at this scoping level).
 */
int XFileArrayDef::
get_size(const XFileNode::PrevData &prev_data) const {
  if (is_fixed_size()) {
    return _fixed_size;
  } else {
    XFileNode::PrevData::const_iterator pi;
    pi = prev_data.find(_dynamic_size);
    nassertr_always(pi != prev_data.end(), 0);
    nassertr((*pi).second != nullptr, 0);
    return (*pi).second->i();
  }
}

/**
 *
 */
void XFileArrayDef::
output(std::ostream &out) const {
  if (is_fixed_size()) {
    out << "[" << _fixed_size << "]";
  } else {
    out << "[" << _dynamic_size->get_name() << "]";
  }
}

/**
 * Returns true if the node, particularly a template node, is structurally
 * equivalent to the other node (which must be of the same type).  This checks
 * data element types, but does not compare data element names.
 */
bool XFileArrayDef::
matches(const XFileArrayDef &other, const XFileDataDef *parent,
        const XFileDataDef *other_parent) const {
  if (other.is_fixed_size() != is_fixed_size()) {
    return false;
  }
  if (is_fixed_size()) {
    if (other.get_fixed_size() != get_fixed_size()) {
      return false;
    }

  } else {
    int child_index = parent->find_child_index(get_dynamic_size());
    int other_child_index =
      other_parent->find_child_index(other.get_dynamic_size());
    if (other_child_index != child_index) {
      return false;
    }
  }

  return true;
}
