/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file orthographicLens.cxx
 * @author mike
 * @date 1999-02-18
 */

#include "orthographicLens.h"
#include "indent.h"
#include "bamReader.h"

TypeHandle OrthographicLens::_type_handle;


/**
 * Allocates a new Lens just like this one.
 */
PT(Lens) OrthographicLens::
make_copy() const {
  return new OrthographicLens(*this);
}

/**
 * Returns true if the lens represents a linear projection (e.g.
 * PerspectiveLens, OrthographicLens), and therefore there is a valid matrix
 * returned by get_projection_mat(), or false otherwise.
 */
bool OrthographicLens::
is_linear() const {
  return true;
}

/**
 * Returns true if the lens represents a orthographic projection (i.e.  it is
 * a OrthographicLens), false otherwise.
 */
bool OrthographicLens::
is_orthographic() const {
  return true;
}

/**
 *
 */
void OrthographicLens::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " film size = " << get_film_size() << "\n";
}

/**
 * This is the generic implementation, which is based on do_extrude() and
 * assumes a linear distribution of depth values between the near and far
 * points.
 */
bool OrthographicLens::
do_extrude_depth(const CData *cdata,
                 const LPoint3 &point2d, LPoint3 &point3d) const {
  return do_extrude_depth_with_mat(cdata, point2d, point3d);
}

/**
 * Computes the complete transformation matrix from 3-d point to 2-d point, if
 * the lens is linear.
 */
void OrthographicLens::
do_compute_projection_mat(Lens::CData *lens_cdata) {
  CoordinateSystem cs = lens_cdata->_cs;
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  PN_stdfloat a = 2.0f / (lens_cdata->_far_distance - lens_cdata->_near_distance);
  PN_stdfloat b = -(lens_cdata->_far_distance + lens_cdata->_near_distance) / (lens_cdata->_far_distance - lens_cdata->_near_distance);

  LMatrix4 canonical;
  switch (cs) {
  case CS_zup_right:
    canonical.set(1.0f,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  0.0f,
                  0.0f,  1.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  1.0f);
    break;

  case CS_yup_right:
    canonical.set(1.0f,  0.0f,  0.0f,  0.0f,
                  0.0f,  1.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a,  0.0f,
                  0.0f,  0.0f,     b,  1.0f);
    break;

  case CS_zup_left:
    canonical.set(1.0f,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a,  0.0f,
                  0.0f,  1.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  1.0f);
    break;

  case CS_yup_left:
    canonical.set(1.0f,  0.0f,  0.0f,  0.0f,
                  0.0f,  1.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  0.0f,
                  0.0f,  0.0f,     b,  1.0f);
    break;

  default:
    gobj_cat.error()
      << "Invalid coordinate system " << (int)cs << " in OrthographicLens!\n";
    canonical = LMatrix4::ident_mat();
  }

  lens_cdata->_projection_mat = do_get_lens_mat_inv(lens_cdata) * canonical * do_get_film_mat(lens_cdata);
  lens_cdata->_projection_mat_left = lens_cdata->_projection_mat_right = lens_cdata->_projection_mat;

  do_adjust_comp_flags(lens_cdata,
                       CF_projection_mat_inv | CF_projection_mat_left_inv | CF_projection_mat_right_inv,
                       CF_projection_mat);
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void OrthographicLens::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *OrthographicLens::
make_from_bam(const FactoryParams &params) {
  OrthographicLens *lens = new OrthographicLens;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  lens->fillin(scan, manager);

  return lens;
}
