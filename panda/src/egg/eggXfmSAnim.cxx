// Filename: eggXfmSAnim.cxx
// Created by:  drose (19Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "eggXfmAnimData.h"
#include "eggParameters.h"

#include <indent.h>
#include <compose_matrix.h>

#include <math.h>

TypeHandle EggXfmSAnim::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::Conversion constructor
//       Access: Public
//  Description: Converts the older-style XfmAnim table to the
//               newer-style XfmSAnim table.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::optimize
//       Access: Public
//  Description: Optimizes the table by collapsing redundant
//               sub-tables.
////////////////////////////////////////////////////////////////////
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
	// If we've optimized down to one value, check to see if it is
	// a default value.
	double value = sanim->get_value(0);
	double default_value;
	if (sanim->has_name() && strchr("ijk", sanim->get_name()[0]) != NULL) {
	  default_value = 1.0;
	} else {
	  default_value = 0.0;
	}

	if (fabs(value - default_value) < egg_parameters->_table_threshold) {
	  // It's a default-valued table, and therefore redundant:
	  // remove it.
	  erase(ci);
	}
      }
    }

    ci = ci_next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::write
//       Access: Public, Virtual
//  Description: Writes the data to the indicated output stream in Egg
//               format.
////////////////////////////////////////////////////////////////////
void EggXfmSAnim::
write(ostream &out, int indent_level) const {
  test_under_integrity();

  write_header(out, indent_level, "<Xfm$Anim_S$>");

  if (has_fps()) {
    indent(out, indent_level + 2) << "<Scalar> fps { " << get_fps() << " }\n";
  }

  if (has_order()) {
    indent(out, indent_level + 2)
      << "<Char*> order { " << get_order() << " }\n";
  }

  EggGroupNode::write(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}

  
////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::get_num_rows
//       Access: Public
//  Description: Returns the effective number of rows in the table.
//               This is actually the number of rows of the smallest
//               subtable larger than one row.  This is a convenience
//               function that treats the table of tables as if it
//               were a single table of matrices.
////////////////////////////////////////////////////////////////////
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
	  min_rows = min(min_rows, sanim->get_num_rows());
	}
      }
    }
  }

  return min_rows;
}
  
////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::get_value
//       Access: Public
//  Description: Returns the value of the aggregate row of the table
//               as a matrix.  This is a convenience function that
//               treats the table of tables as if it were a single
//               table of matrices.  It is an error to call this if
//               any SAnimData children of this node have an improper
//               name (e.g. not a single letter, or not one of
//               "ijkphrxyz").
////////////////////////////////////////////////////////////////////
void EggXfmSAnim::
get_value(int row, LMatrix4d &mat) const {
  LVector3d scale(1.0, 1.0, 1.0);
  LVector3d hpr(0.0, 0.0, 0.0);
  LVector3d translate(0.0, 0.0, 0.0);

  const_iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);

      if (sanim->get_num_rows() == 0) {
	// If the table is totally empty, let's keep the default
	// value.
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
	nassertv(false);
      }
    }
  }

  // So now we've got the nine components; build a matrix.
  compose_matrix(mat, scale, hpr, translate, _coordsys);
}

  
////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::add_data
//       Access: Public
//  Description: Adds a new matrix to the table, by adding a new row
//               to each of the subtables.
//
//               This is a convenience function that
//               treats the table of tables as if it were a single
//               table of matrices.  It is an error to call this if
//               any SAnimData children of this node have an improper
//               name (e.g. not a single letter, or not one of
//               "ijkphrxyz").
//
//               This function has the further requirement that all
//               nine of the subtables must exist and be of the same
//               length.  Thus, you probably cannot take an existing
//               EggXfmSAnim object and start adding matrices to the
//               end; you must clear out the original data first.  (As
//               a special exception, if no tables exist, they will be
//               created.)
//
//               This function may fail silently if the matrix cannot
//               be decomposed into scale, rotate, and translate.  In
//               this case, nothing is done and the function returns
//               false.
////////////////////////////////////////////////////////////////////
bool EggXfmSAnim::
add_data(const LMatrix4d &mat) {
  LVector3d scale, hpr, translate;
  bool result = decompose_matrix(mat, scale, hpr, translate, _coordsys);
  if (!result) {
    return false;
  }

  if (empty()) {
    // If we have no children, create all nine tables now.
    const char *table_ids = "ijkphrxyz";
    for (const char *p = table_ids; *p; p++) {
      EggSAnimData *sanim = new EggSAnimData(string(1, *p));
      add_child(sanim);
    }
  }

#ifndef NDEBUG
  int table_length = -1;
  int num_tables = 0;
#endif

  const_iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggSAnimData::get_class_type())) {
      EggSAnimData *sanim = DCAST(EggSAnimData, *ci);

#ifndef NDEBUG
      num_tables++;

      // Each table must have the same length.
      if (table_length < 0) {
	table_length = sanim->get_num_rows();
      } else {
	nassertr(sanim->get_num_rows() == table_length, false);
      }
#endif

      // Each child SAnimData table should have a one-letter name.
      nassertr(sanim->get_name().length() == 1, false);

      switch (sanim->get_name()[0]) {
      case 'i':
	sanim->add_data(scale[0]);
	break;

      case 'j':
	sanim->add_data(scale[1]);
	break;

      case 'k':
	sanim->add_data(scale[2]);
	break;

      case 'h':
	sanim->add_data(hpr[0]);
	break;

      case 'p':
	sanim->add_data(hpr[1]);
	break;

      case 'r':
	sanim->add_data(hpr[2]);
	break;

      case 'x':
	sanim->add_data(translate[0]);
	break;
     
      case 'y':
	sanim->add_data(translate[1]);
	break;

      case 'z':
	sanim->add_data(translate[2]);
	break;

      default:
	// One of the child tables had an invalid name.
	nassertr(false, false);
      }
    }
  }

  nassertr(num_tables == 9, false);
  return true;
}
  
////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::r_transform
//       Access: Protected, Virtual
//  Description: Applies the indicated transform to all the rows of
//               the table.  This actually forces the generation of a
//               totally new set of rows.
////////////////////////////////////////////////////////////////////
void EggXfmSAnim::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
	    CoordinateSystem to_cs) {
  // We need to build an inverse matrix that doesn't reflect the
  // translation component.
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

    // If this assertion fails, we attempted to transform by a skew
    // matrix or some such thing that cannot be represented in an anim
    // file.
    nassertv(result);
  }

  // Now clean out the redundant columns we created.
  optimize();
}

////////////////////////////////////////////////////////////////////
//     Function: EggXfmSAnim::r_mark_coordsys
//       Access: Protected, Virtual
//  Description: This is only called immediately after loading an egg
//               file from disk, to propagate the value found in the
//               CoordinateSystem entry (or the default Y-up
//               coordinate system) to all nodes that care about what
//               the coordinate system is.
////////////////////////////////////////////////////////////////////
void EggXfmSAnim::
r_mark_coordsys(CoordinateSystem cs) {
  _coordsys = cs;
}
