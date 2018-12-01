/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTable.cxx
 * @author drose
 * @date 1999-02-19
 */

#include "eggTable.h"

#include "string_utils.h"
#include "indent.h"

TypeHandle EggTable::_type_handle;

/**
 * Returns true if the table contains a transform description, false
 * otherwise.
 */
bool EggTable::
has_transform() const {
  const_iterator ci;

  for (ci = begin(); ci != end(); ++ci) {
    EggNode *child = (*ci);
    if (child->is_anim_matrix()) {
      return true;
    }
  }

  return false;
}

/**
 * Writes the table and all of its children to the indicated output stream in
 * Egg format.
 */
void EggTable::
write(std::ostream &out, int indent_level) const {
  test_under_integrity();

  switch (get_table_type()) {
  case TT_table:
    write_header(out, indent_level, "<Table>");
    break;

  case TT_bundle:
    write_header(out, indent_level, "<Bundle>");
    break;

  default:
    // invalid table type
    nassertv(false);
  }

  EggGroupNode::write(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}


/**
 * Returns the TableType value associated with the given string
 * representation, or TT_invalid if the string does not match any known
 * TableType value.
 */
EggTable::TableType EggTable::
string_table_type(const std::string &string) {
  if (cmp_nocase_uh(string, "table") == 0) {
    return TT_table;
  } else if (cmp_nocase_uh(string, "bundle") == 0) {
    return TT_bundle;
  } else {
    return TT_invalid;
  }
}

/**
 * This is called from within the egg code by transform().  It applies a
 * transformation matrix to the current node in some sensible way, then
 * continues down the tree.
 *
 * The first matrix is the transformation to apply; the second is its inverse.
 * The third parameter is the coordinate system we are changing to, or
 * CS_default if we are not changing coordinate systems.
 */
void EggTable::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
            CoordinateSystem to_cs) {
  // We need to duplicate the logic in EggGroup: if we have a matrix transform
  // witin this table, apply the transformation to it, but then apply only the
  // scalerotational part of the transformation to any children.

  // On the other hand, if we have no matrix transform within this table, pass
  // the transformation through.

  // This logic is complicated by the fact that matrix transforms with a
  // <Table> group are not stored within the table itself, but rather within a
  // child named "xform".  Fortunately, has_transform() abstracts out this
  // detail for us.

  if (has_transform()) {
    // At least one child of this table represents an animation matrix
    // transform: that child gets the real matrix, while all other children
    // get the truncated matrix.

    LMatrix4d mat1 = mat;
    LMatrix4d inv1 = inv;

    // If we have a translation component, we should only apply it to the top
    // matrix.  All subsequent matrices get just the rotational component.
    mat1.set_row(3, LVector3d(0.0, 0.0, 0.0));
    inv1.set_row(3, LVector3d(0.0, 0.0, 0.0));

    iterator ci;
    for (ci = begin(); ci != end(); ++ci) {
      EggNode *child = (*ci);
      if (child->is_anim_matrix()) {
        child->r_transform(mat, inv, to_cs);
      } else {
        child->r_transform(mat1, inv1, to_cs);
      }
    }

  } else {
    // No children of this table represent an animation matrix transform: all
    // children get the real matrix.
    EggGroupNode::r_transform(mat, inv, to_cs);
  }
}


/**
 *
 */
std::ostream &operator << (std::ostream &out, EggTable::TableType t) {
  switch (t) {
  case EggTable::TT_invalid:
    return out << "invalid table";
  case EggTable::TT_table:
    return out << "table";
  case EggTable::TT_bundle:
    return out << "bundle";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}
