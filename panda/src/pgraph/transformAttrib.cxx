// Filename: transformAttrib.cxx
// Created by:  drose (23Feb02)
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

#include "transformAttrib.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "compose_matrix.h"

TypeHandle TransformAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::make_identity
//       Access: Published, Static
//  Description: Constructs an identity transform.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TransformAttrib::
make_identity() {
  TransformAttrib *attrib = new TransformAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::make_components
//       Access: Published, Static
//  Description: Makes a new TransformAttrib with the specified
//               components.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TransformAttrib::
make_components(const LVecBase3f &pos, const LVecBase3f &hpr, 
                const LVecBase3f &scale) {
  // Make a special-case check for the identity transform.
  if (pos == LVecBase3f(0.0f, 0.0f, 0.0f) &&
      hpr == LVecBase3f(0.0f, 0.0f, 0.0f) &&
      scale == LVecBase3f(1.0f, 1.0f, 1.0f)) {
    return make_identity();
  }

  TransformAttrib *attrib = new TransformAttrib;
  attrib->_pos = pos;
  attrib->_hpr = hpr;
  attrib->_scale = scale;
  attrib->_flags = F_components_given | F_components_known | F_has_components;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::make_components
//       Access: Published, Static
//  Description: Makes a new TransformAttrib with the specified
//               transformation matrix.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TransformAttrib::
make_mat(const LMatrix4f &mat) {
  // Make a special-case check for the identity matrix.
  if (mat == LMatrix4f::ident_mat()) {
    return make_identity();
  }

  TransformAttrib *attrib = new TransformAttrib;
  attrib->_mat = mat;
  attrib->_flags = F_mat_known;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_identity()) {
    out << "(identity)";

  } else if (has_components()) {
    out << "(";
    if (get_pos() != LVecBase3f(0.0f, 0.0f, 0.0f)) {
      out << "pos " << get_pos();
    }
    if (get_hpr() != LVecBase3f(0.0f, 0.0f, 0.0f)) {
      out << "hpr " << get_hpr();
    }
    if (get_scale() != LVecBase3f(1.0f, 1.0f, 1.0f)) {
      out << "scale " << get_scale();
    }
    out << ")";

  } else {
    out << get_mat();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TransformAttrib
//               types to return a unique number indicating whether
//               this TransformAttrib is equivalent to the other one.
//
//               This should return 0 if the two TransformAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TransformAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TransformAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TransformAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (is_identity() != ta->is_identity()) {
    return is_identity() < ta->is_identity();
  }
  if (is_identity()) {
    // All identity transforms are equivalent to each other.
    return 0;
  }

  bool components_given = (_flags & F_components_given) != 0;
  bool ta_components_given = (ta->_flags & F_components_given) != 0;
  if (components_given != ta_components_given) {
    return components_given < ta_components_given;
  }
  if (components_given) {
    // If the transform was specified componentwise, compare them
    // componentwise.
    int c = _pos.compare_to(ta->_pos);
    if (c != 0) {
      return c;
    }
    c = _hpr.compare_to(ta->_hpr);
    if (c != 0) {
      return c;
    }
    c = _scale.compare_to(ta->_hpr);
    return c;
    
  }

  // Otherwise, compare the matrices.
  return get_mat().compare_to(ta->get_mat());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TransformAttrib::
compose_impl(const RenderAttrib *other) const {
  const TransformAttrib *ta;
  DCAST_INTO_R(ta, other, other);

  // Identity times anything, duh.
  if (is_identity()) {
    return ta;
  } else if (ta->is_identity()) {
    return this;
  }

  // Perhaps we should be smarter here if both transforms are
  // componentwise, and multiply them componentwise.
  LMatrix4f new_mat = get_mat() * ta->get_mat();
  return make_mat(new_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TransformAttrib
//               types to specify what the default property for a
//               TransformAttrib of this type should be.
//
//               This should return a newly-allocated TransformAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TransformAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TransformAttrib::
make_default_impl() const {
  return new TransformAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::calc_singular
//       Access: Private
//  Description: Determines whether the transform is singular (i.e. it
//               scales to zero, and has no inverse).
////////////////////////////////////////////////////////////////////
void TransformAttrib::
calc_singular() {
  bool singular = false;

  if (has_components()) {
    // The matrix is singular if any component of its scale is 0.
    singular = (_scale[0] == 0.0f || _scale[1] == 0.0f || _scale[2] == 0.0f);
  } else {
    // The matrix is singular if its determinant is zero.
    const LMatrix4f &mat = get_mat();
    singular = (mat.get_upper_3().determinant() == 0.0f);
  }

  if (singular) {
    _flags |= F_is_singular;
  }
  _flags |= F_singular_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::calc_components
//       Access: Private
//  Description: Derives the components from the matrix, if possible.
////////////////////////////////////////////////////////////////////
void TransformAttrib::
calc_components() {
  bool possible = decompose_matrix(get_mat(), _scale, _hpr, _pos);
  if (possible) {
    // Some matrices can't be decomposed into scale, hpr, pos.
    _flags |= F_has_components;
  }
  _flags |= F_components_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::calc_mat
//       Access: Private
//  Description: Computes the matrix from the components.
////////////////////////////////////////////////////////////////////
void TransformAttrib::
calc_mat() {
  compose_matrix(_mat, _scale, _hpr, _pos);
  _flags |= F_mat_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TransformAttrib.
////////////////////////////////////////////////////////////////////
void TransformAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TransformAttrib is encountered
//               in the Bam file.  It should create the TransformAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TransformAttrib::
make_from_bam(const FactoryParams &params) {
  TransformAttrib *attrib = new TransformAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return new_from_bam(attrib, manager);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformAttrib.
////////////////////////////////////////////////////////////////////
void TransformAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);
}
