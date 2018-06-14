/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file matrixLens.cxx
 * @author drose
 * @date 2001-12-12
 */

#include "matrixLens.h"
#include "indent.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle MatrixLens::_type_handle;


/**
 * Allocates a new Lens just like this one.
 */
PT(Lens) MatrixLens::
make_copy() const {
  return new MatrixLens(*this);
}

/**
 * Returns true if the lens represents a linear projection (e.g.
 * PerspectiveLens, MatrixLens), and therefore there is a valid matrix
 * returned by get_projection_mat(), or false otherwise.
 */
bool MatrixLens::
is_linear() const {
  return true;
}

/**
 *
 */
void MatrixLens::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << ":\n";
  get_projection_mat().write(out, indent_level + 2);
}

/**
 * Computes the complete transformation matrix from 3-d point to 2-d point, if
 * the lens is linear.
 */
void MatrixLens::
do_compute_projection_mat(Lens::CData *lens_cdata) {
  lens_cdata->_projection_mat = do_get_lens_mat_inv(lens_cdata) * _user_mat * do_get_film_mat(lens_cdata);

  if (_ml_flags & MF_has_left_eye) {
    lens_cdata->_projection_mat_left = do_get_lens_mat_inv(lens_cdata) * _left_eye_mat * do_get_film_mat(lens_cdata);
  } else {
    lens_cdata->_projection_mat_left = lens_cdata->_projection_mat;
  }

  if (_ml_flags & MF_has_right_eye) {
    lens_cdata->_projection_mat_right = do_get_lens_mat_inv(lens_cdata) * _right_eye_mat * do_get_film_mat(lens_cdata);
  } else {
    lens_cdata->_projection_mat_right = lens_cdata->_projection_mat;
  }

  do_adjust_comp_flags(lens_cdata, CF_projection_mat_inv,
                       CF_projection_mat);
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void MatrixLens::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void MatrixLens::
write_datagram(BamWriter *manager, Datagram &dg) {
  Lens::write_datagram(manager, dg);

  if (manager->get_file_minor_ver() >= 41) {
    dg.add_uint8(_ml_flags);
    _user_mat.write_datagram(dg);

    if (_ml_flags & MF_has_left_eye) {
      _left_eye_mat.write_datagram(dg);
    }
    if (_ml_flags & MF_has_right_eye) {
      _left_eye_mat.write_datagram(dg);
    }
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *MatrixLens::
make_from_bam(const FactoryParams &params) {
  MatrixLens *lens = new MatrixLens;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  lens->fillin(scan, manager);

  return lens;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new MatrixLens.
 */
void MatrixLens::
fillin(DatagramIterator &scan, BamReader *manager) {
  Lens::fillin(scan, manager);

  if (manager->get_file_minor_ver() >= 41) {
    _ml_flags = scan.get_uint8();

    _user_mat.read_datagram(scan);
    if (_ml_flags & MF_has_left_eye) {
      _left_eye_mat.read_datagram(scan);
    }
    if (_ml_flags & MF_has_right_eye) {
      _right_eye_mat.read_datagram(scan);
    }
  }
}
