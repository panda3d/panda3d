/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggXfmSAnim.cxx
 * @author drose
 * @date 1999-02-19
 */

#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggParameters.h"
#include "config_egg.h"

#include "indent.h"
#include "compose_matrix.h"
#include "dcast.h"

#include <math.h>

using std::string;

TypeHandle EggXfmSAnim::_type_handle;

const string EggXfmSAnim::_standard_order = "srpht";

/**
 * Converts the older-style XfmAnim table to the newer-style XfmSAnim table.
 */
EggXfmSAnim::
EggXfmSAnim(const EggXfmAnimData &convert_from)
  : EggGroupNode(convert_from.get_name())
{
  _has_fps = false;
  _coordsys = convert_from.get_coordinate_system();

  if (convert_from.has_order()) {
    set_order(convert_from.get_order());
  }
  if (convert_from.has_fps()) {
    set_fps(convert_from.get_fps());
  }

  const string &contents = convert_from.get_contents();
  for (int col = 0; col < convert_from.get_num_cols(); col++) {
    EggSAnimData *sanim = new EggSAnimData(contents.substr(col, 1));
    add_child(sanim);
    for (int row = 0; row < convert_from.get_num_rows(); row++) {
      sanim->add_data(convert_from.get_value(row, col));
    }
  }
}

/**
 * Optimizes the table by collapsing redundant sub-tables.
 */
void EggXfmSAnim::
optimize() {
  iterator ci = begin();
  while (ci != end()) {
    iterator ci_next = ci;
    ++ci_next;

    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);
      sanim->optimize();

      if (sanim->get_num_rows() == 1) {
        // If we've optimized down to one value, check to see if it is a
        // default value.
        double value = sanim->get_value(0);
        double default_value;
        if (sanim->has_name() && strchr("ijk", sanim->get_name()[0]) != nullptr) {
          default_value = 1.0;
        } else {
          default_value = 0.0;
        }

        if (fabs(value - default_value) < egg_parameters->_table_threshold) {
          // It's a default-valued table, and therefore redundant: remove it.
          erase(ci);
        }
      }
    }

    ci = ci_next;
  }
}

/**
 * Optimizes the table by collapsing redundant sub-tables, and simultaneously
 * ensures that the order string is the standard order (which is the same as
 * that supported by compose_matrix() and decompose_matrix()).
 */
void EggXfmSAnim::
optimize_to_standard_order() {
  if (get_order() != get_standard_order()) {
    normalize_by_rebuilding();
  }
  optimize();
}

/**
 * The inverse operation of optimize(), this ensures that all the sub-tables
 * have the same length by duplicating rows as necessary.  This is needed
 * before doing operations like add_data() or set_value() on an existing
 * table.
 */
void EggXfmSAnim::
normalize() {
  if (get_order() != get_standard_order()) {
    // If our order string is wrong, we must fix it now.  This will
    // incidentally also normalize the table, because we are totally
    // rebuilding it.
    normalize_by_rebuilding();

  } else {
    // Otherwise, if the order string is already the standard order string, we
    // can do this the easy way (from a computational standpoint), which is
    // just to lengthen the tables directly.
    normalize_by_expanding();
  }
}

/**
 * Returns true if this node represents a table of animation transformation
 * data, false otherwise.
 */
bool EggXfmSAnim::
is_anim_matrix() const {
  return true;
}

/**
 * Writes the data to the indicated output stream in Egg format.
 */
void EggXfmSAnim::
write(std::ostream &out, int indent_level) const {
  test_under_integrity();

  write_header(out, indent_level, "<Xfm$Anim_S$>");

  if (has_fps()) {
    indent(out, indent_level + 2) << "<Scalar> fps { " << get_fps() << " }\n";
  }

  if (has_order()) {
    indent(out, indent_level + 2)
      << "<Char*> order { " << get_order() << " }\n";
  }

  // Rather than calling EggGroupNode::write() to write out the children, we
  // do it directly here so we can control the order.  We write out all the
  // non-table children first, then write out the table children in our
  // expected order.  (Normally there are only table children.)
  EggSAnimData *tables[num_matrix_components];
  memset(tables, 0, sizeof(EggSAnimData *) * num_matrix_components);

  const_iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    EggNode *child = (*ci);
    if (child->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);

      // Each child SAnimData table should have a one-letter name.
      nassertv(sanim->get_name().length() == 1);
      char name = sanim->get_name()[0];
      char *p = (char *)strchr(matrix_component_letters, name);
      nassertv(p != nullptr);
      if (p != nullptr) {
        int index = p - matrix_component_letters;
        nassertv(tables[index] == nullptr);
        tables[index] = sanim;
      }
    } else {
      // Any non-table children are written directly.
      child->write(out, indent_level + 2);
    }
  }

  // Now write out the table children in our normal order.
  for (int i = 0; i < num_matrix_components; i++) {
    if (tables[i] != nullptr) {
      tables[i]->write(out, indent_level + 2);
    }
  }

  indent(out, indent_level) << "}\n";
}


/**
 * Composes a matrix out of the nine individual components, respecting the
 * order string.  The components will be applied in the order indicated by the
 * string.
 */
void EggXfmSAnim::
compose_with_order(LMatrix4d &mat,
                   const LVecBase3d &scale,
                   const LVecBase3d &shear,
                   const LVecBase3d &hpr,
                   const LVecBase3d &trans,
                   const string &order,
                   CoordinateSystem cs) {

  mat = LMatrix4d::ident_mat();

  bool reverse_roll = false;

  if (order == "sphrt" && egg_support_old_anims) {
    // As a special case, if the order string is exactly "sphrt" (which is
    // what all our legacy anim files used), we interpret roll in the opposite
    // direction (as our legacy anim files did).
    reverse_roll = true;
  }

  string::const_iterator pi;
  for (pi = order.begin(); pi != order.end(); ++pi) {
    switch (*pi) {
    case 's':
      mat = mat * LMatrix4d::scale_shear_mat(scale, shear, cs);
      break;

    case 'h':
      mat = mat * LMatrix4d::rotate_mat_normaxis(hpr[0], LVector3d::up(cs), cs);
      break;

    case 'p':
      mat = mat * LMatrix4d::rotate_mat_normaxis(hpr[1], LVector3d::right(cs), cs);
      break;

    case 'r':
      if (reverse_roll) {
        mat = mat * LMatrix4d::rotate_mat_normaxis(-hpr[2], LVector3d::forward(cs), cs);
      } else {
        mat = mat * LMatrix4d::rotate_mat_normaxis(hpr[2], LVector3d::forward(cs), cs);
      }
      break;

    case 't':
      mat = mat * LMatrix4d::translate_mat(trans);
      break;

    default:
      egg_cat.warning()
        << "Invalid letter in order string: " << *pi << "\n";
    }
  }
}

/**
 * Returns the effective number of rows in the table.  This is actually the
 * number of rows of the smallest subtable larger than one row.  This is a
 * convenience function that treats the table of tables as if it were a single
 * table of matrices.
 */
int EggXfmSAnim::
get_num_rows() const {
  bool found_any = false;
  int min_rows = 1;

  const_iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);
      if (sanim->get_num_rows() > 1) {
        if (!found_any) {
          min_rows = sanim->get_num_rows();

        } else {
          min_rows = std::min(min_rows, sanim->get_num_rows());
        }
      }
    }
  }

  return min_rows;
}

/**
 * Returns the value of the aggregate row of the table as a matrix.  This is a
 * convenience function that treats the table of tables as if it were a single
 * table of matrices.  It is an error to call this if any SAnimData children
 * of this node have an improper name (e.g.  not a single letter, or not one
 * of "ijkabchprxyz").
 */
void EggXfmSAnim::
get_value(int row, LMatrix4d &mat) const {
  LVector3d scale(1.0, 1.0, 1.0);
  LVector3d shear(0.0, 0.0, 0.0);
  LVector3d hpr(0.0, 0.0, 0.0);
  LVector3d translate(0.0, 0.0, 0.0);

  const_iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);

      if (sanim->get_num_rows() == 0) {
        // If the table is totally empty, let's keep the default value.
        break;
      }

      double value;
      if (sanim->get_num_rows() == 1) {
        value = sanim->get_value(0);
      } else {
        nassertv(row < sanim->get_num_rows());
        value = sanim->get_value(row);
      }

      // Each child SAnimData table should have a one-letter name.
      nassertv(sanim->get_name().length() == 1);

      switch (sanim->get_name()[0]) {
      case 'i':
        scale[0] = value;
        break;

      case 'j':
        scale[1] = value;
        break;

      case 'k':
        scale[2] = value;
        break;

      case 'a':
        shear[0] = value;
        break;

      case 'b':
        shear[1] = value;
        break;

      case 'c':
        shear[2] = value;
        break;

      case 'h':
        hpr[0] = value;
        break;

      case 'p':
        hpr[1] = value;
        break;

      case 'r':
        hpr[2] = value;
        break;

      case 'x':
        translate[0] = value;
        break;

      case 'y':
        translate[1] = value;
        break;

      case 'z':
        translate[2] = value;
        break;

      default:
        // One of the child tables had an invalid name.
        nassert_raise("invalid name in child table");
        return;
      }
    }
  }

  // So now we've got the nine components; build a matrix.
  compose_with_order(mat, scale, shear, hpr, translate, get_order(), _coordsys);
}

/**
 * Replaces the indicated row of the table with the given matrix.
 *
 * This function can only be called if all the constraints of add_data(),
 * below, are met.  Call normalize() first if you are not sure.
 *
 * The return value is true if the matrix can be decomposed and stored as
 * scale, shear, rotate, and translate, or false otherwise.  The data is set
 * in either case.
 */
bool EggXfmSAnim::
set_value(int row, const LMatrix4d &mat) {
  nassertr(get_order() == get_standard_order(), false);

  double components[num_matrix_components];
  bool add_ok = decompose_matrix(mat, components, _coordsys);

  // Sanity check our sub-tables.
#ifndef NDEBUG
  int table_length = -1;
#endif

  for (int i = 0; i < num_matrix_components; i++) {
    string name(1, matrix_component_letters[i]);
    EggNode *child = find_child(name);
    nassertr(child != nullptr &&
             child->is_of_type(EggSAnimData::get_class_type()), false);
    EggSAnimData *sanim = DCAST(EggSAnimData, child);

#ifndef NDEBUG
    // Each table must have the same length.
    if (table_length < 0) {
      table_length = sanim->get_num_rows();
    } else {
      nassertr(sanim->get_num_rows() == table_length, false);
    }
#endif
    sanim->set_value(row, components[i]);
  }

#ifndef NDEBUG
  // Sanity check the result.
  LMatrix4d new_mat;
  get_value(row, new_mat);
  if (!new_mat.almost_equal(mat, 0.005)) {
    egg_cat.warning()
      << "After set_row(" << row << ", ...) to:\n";
    mat.write(egg_cat.warning(false), 2);
    egg_cat.warning(false)
      << "which produces components:\n";
    for (int i = 0; i < num_matrix_components; i += 3) {
      egg_cat.warning(false)
        << "  "
        << matrix_component_letters[i]
        << matrix_component_letters[i + 1]
        << matrix_component_letters[i + 2]
        << ": "
        << components[i] << " "
        << components[i + 1] << " "
        << components[i + 2] << "\n";
    }
    egg_cat.warning(false)
      << "new mat set was:\n";
    new_mat.write(egg_cat.warning(false), 2);
    return false;
  }
#endif

  return add_ok;
}

/**
 * Adds a new matrix to the table, by adding a new row to each of the
 * subtables.
 *
 * This is a convenience function that treats the table of tables as if it
 * were a single table of matrices.  It is an error to call this if any
 * SAnimData children of this node have an improper name (e.g.  not a single
 * letter, or not one of "ijkabchprxyz").
 *
 * This function has the further requirement that all nine of the subtables
 * must exist and be of the same length.  Furthermore, the order string must
 * be the standard order string, which matches the system compose_matrix() and
 * decompose_matrix() functions.
 *
 * Thus, you probably cannot take an existing EggXfmSAnim object and start
 * adding matrices to the end; you must clear out the original data first.
 * (As a special exception, if no tables exist, they will be created.)  The
 * method normalize() will do this for you on an existing EggXfmSAnim.
 *
 * This function may fail silently if the matrix cannot be decomposed into
 * scale, shear, rotate, and translate.  In this case, the closest
 * approximation is added to the table, and false is returned.
 */
bool EggXfmSAnim::
add_data(const LMatrix4d &mat) {
  double components[num_matrix_components];
  bool add_ok = decompose_matrix(mat, components, _coordsys);

  if (empty()) {
    // If we have no children, create all twelve tables now.
    for (int i = 0; i < num_matrix_components; i++) {
      char name = matrix_component_letters[i];
      EggSAnimData *sanim = new EggSAnimData(string(1, name));
      add_child(sanim);
    }

    // Also insist on the correct ordering right off the bat.
    set_order(get_standard_order());
  }

  nassertr(get_order() == get_standard_order(), false);

#ifndef NDEBUG
  int table_length = -1;
#endif

  for (int i = 0; i < num_matrix_components; i++) {
    string name(1, matrix_component_letters[i]);
    EggNode *child = find_child(name);
    nassertr(child != nullptr &&
             child->is_of_type(EggSAnimData::get_class_type()), false);
    EggSAnimData *sanim = DCAST(EggSAnimData, child);

#ifndef NDEBUG
    // Each table must have the same length.
    if (table_length < 0) {
      table_length = sanim->get_num_rows();
    } else {
      nassertr(sanim->get_num_rows() == table_length, false);
    }
#endif
    sanim->add_data(components[i]);
  }

#ifndef NDEBUG
  // Sanity check the result.
  LMatrix4d new_mat;
  if (table_length >= 0) {
    get_value(table_length, new_mat);
  } else {
    get_value(0, new_mat);
  }
  if (!new_mat.almost_equal(mat, 0.005)) {
    egg_cat.warning()
      << "After add_data():\n";
    mat.write(egg_cat.warning(false), 2);
    egg_cat.warning(false)
      << "which produces components:\n";
    for (int i = 0; i < num_matrix_components; i += 3) {
      egg_cat.warning(false)
        << "  "
        << matrix_component_letters[i]
        << matrix_component_letters[i + 1]
        << matrix_component_letters[i + 2]
        << ": "
        << components[i] << " "
        << components[i + 1] << " "
        << components[i + 2] << "\n";
    }
    egg_cat.warning(false)
      << "new mat set was:\n";
    new_mat.write(egg_cat.warning(false), 2);
    return false;
  }
#endif

  return add_ok;
}

/**
 * Adds a new row to the named component (one of matrix_component_letters) of
 * the table.
 */
void EggXfmSAnim::
add_component_data(const string &component_name, double value) {
  EggNode *child = find_child(component_name);
  EggSAnimData *sanim;
  if (child == nullptr) {
    // We don't have this component yet; create it.
    sanim = new EggSAnimData(component_name);
    add_child(sanim);

  } else {
    DCAST_INTO_V(sanim, child);
  }

  sanim->add_data(value);
}

/**
 * Adds a new row to the indicated component (0-12) of the table.
 */
void EggXfmSAnim::
add_component_data(int component, double value) {
  nassertv(component >= 0 && component < num_matrix_components);

  string name(1, matrix_component_letters[component]);
  add_component_data(name, value);
}

/**
 * Applies the indicated transform to all the rows of the table.  This
 * actually forces the generation of a totally new set of rows, and will
 * quietly change the order to the standard order (if it is different).
 */
void EggXfmSAnim::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
            CoordinateSystem to_cs) {
  // We need to build an inverse matrix that doesn't reflect the translation
  // component.
  LMatrix4d inv1 = inv;
  inv1.set_row(3, LVector3d(0.0, 0.0, 0.0));

  // Save a temporary copy of the original data.
  EggXfmSAnim original;
  original.steal_children(*this);
  original = (*this);

  // Now we have no children, so our data is clear.  Rebuild it.
  if (to_cs != CS_default) {
    _coordsys = to_cs;
  }

  int num_rows = original.get_num_rows();
  LMatrix4d orig_mat;
  for (int r = 0; r < num_rows; r++) {
    original.get_value(r, orig_mat);
    bool result = add_data(inv1 * orig_mat * mat);

    // If this assertion fails, we attempted to transform by a skew matrix or
    // some such thing that cannot be represented in an anim file.
    nassertv(result);
  }

  // Now clean out the redundant columns we created.
  optimize();
}

/**
 * This is only called immediately after loading an egg file from disk, to
 * propagate the value found in the CoordinateSystem entry (or the default
 * Y-up coordinate system) to all nodes that care about what the coordinate
 * system is.
 */
void EggXfmSAnim::
r_mark_coordsys(CoordinateSystem cs) {
  _coordsys = cs;
}

/**
 * One implementation of normalize() that rebuilds the entire table by
 * composing and decomposing the rows.  This has the advantage that it will
 * also reset the order string to the standard order string, but it is more
 * computationally intensive and is subject to roundoff error.
 */
void EggXfmSAnim::
normalize_by_rebuilding() {
  // Save a temporary copy of the original data.
  EggXfmSAnim original;
  original.steal_children(*this);
  original = (*this);

  // Now we have no children, so our data is clear.  Rebuild it.
  int num_rows = original.get_num_rows();
  LMatrix4d orig_mat;
  for (int r = 0; r < num_rows; r++) {
    original.get_value(r, orig_mat);
    bool result = add_data(orig_mat);

    // If this assertion fails, we somehow got a matrix out of the original
    // table that we could not represent in the new table.  That shouldn't be
    // possible; there's probably something wrong in decompose_matrix().
    nassertv(result);
  }
}

/**
 * Another implementation of normalize() that simply expands any one-row
 * tables and creates default-valued tables where none were before.  This will
 * not change the order string, but is much faster and does not introduce
 * roundoff error.
 */
void EggXfmSAnim::
normalize_by_expanding() {
  iterator ci;

  // First, determine which tables we already have, and how long they are.
  int num_tables = 0;
  int table_length = 1;
  string remaining_tables = matrix_component_letters;

  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);

      nassertv(sanim->get_name().length() == 1);
      char name = sanim->get_name()[0];
      size_t p = remaining_tables.find(name);
      nassertv(p != string::npos);
      remaining_tables[p] = ' ';

      num_tables++;
      if (sanim->get_num_rows() > 1) {
        if (table_length == 1) {
          table_length = sanim->get_num_rows();
        } else {
          nassertv(sanim->get_num_rows() == table_length);
        }
      }
    }
  }

  if (num_tables < num_matrix_components) {
    // Create new, default, children for each table we lack.
    for (size_t p = 0; p < remaining_tables.length(); p++) {
      if (remaining_tables[p] != ' ') {
        double default_value;
        switch (remaining_tables[p]) {
        case 'i':
        case 'j':
        case 'k':
          default_value = 1.0;
          break;

        default:
          default_value = 0.0;
        }

        string name(1, remaining_tables[p]);
        EggSAnimData *sanim = new EggSAnimData(name);
        add_child(sanim);
        sanim->add_data(default_value);
      }
    }
  }

  // Now expand any one-row tables as needed.
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);
      if (sanim->get_num_rows() == 1) {
        double value = sanim->get_value(0);
        for (int i = 1; i < table_length; i++) {
          sanim->add_data(value);
        }
      }
      nassertv(sanim->get_num_rows() == table_length);
    }
  }
}
