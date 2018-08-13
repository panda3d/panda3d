/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perspectiveLens.cxx
 * @author drose
 * @date 1999-02-18
 */

#include "perspectiveLens.h"
#include "bamReader.h"

TypeHandle PerspectiveLens::_type_handle;


/**
 * Allocates a new Lens just like this one.
 */
PT(Lens) PerspectiveLens::
make_copy() const {
  return new PerspectiveLens(*this);
}

/**
 * Returns true if the lens represents a linear projection (e.g.
 * PerspectiveLens, OrthographicLens), and therefore there is a valid matrix
 * returned by get_projection_mat(), or false otherwise.
 */
bool PerspectiveLens::
is_linear() const {
  return true;
}

/**
 * Returns true if the lens represents a perspective projection (i.e.  it is a
 * PerspectiveLens), false otherwise.
 */
bool PerspectiveLens::
is_perspective() const {
  return true;
}

/**
 * This is the generic implementation, which is based on do_extrude() and
 * assumes a linear distribution of depth values between the near and far
 * points.
 */
bool PerspectiveLens::
do_extrude_depth(const CData *cdata,
                 const LPoint3 &point2d, LPoint3 &point3d) const {
  return do_extrude_depth_with_mat(cdata, point2d, point3d);
}

/**
 * Computes the complete transformation matrix from 3-d point to 2-d point, if
 * the lens is linear.
 */
void PerspectiveLens::
do_compute_projection_mat(Lens::CData *lens_cdata) {
  CoordinateSystem cs = lens_cdata->_cs;
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }

  PN_stdfloat fl = do_get_focal_length(lens_cdata);
  PN_stdfloat fFar = do_get_far(lens_cdata);
  PN_stdfloat fNear = do_get_near(lens_cdata);
  PN_stdfloat a, b;

  // Take the limits if either near or far is infinite.
  if (cinf(fFar)) {
    a = 1;
    b = -2 * fNear;
  } else if (cinf(fNear)) {
    // This is valid if the near/far planes are inverted.
    a = -1;
    b = 2 * fFar;
  } else {
    PN_stdfloat far_minus_near = fFar-fNear;
    a = (fFar + fNear);
    b = -2 * fFar * fNear;
    a /= far_minus_near;
    b /= far_minus_near;
  }

  LMatrix4 canonical;
  switch (cs) {
  case CS_zup_right:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  1.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_yup_right:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a, -1.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_zup_left:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a, -1.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_yup_left:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  1.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  default:
    gobj_cat.error()
      << "Invalid coordinate system " << (int)cs << " in PerspectiveLens!\n";
    canonical = LMatrix4::ident_mat();
  }

  lens_cdata->_projection_mat = do_get_lens_mat_inv(lens_cdata) * canonical * do_get_film_mat(lens_cdata);

  if ((lens_cdata->_user_flags & UF_interocular_distance) == 0) {
    lens_cdata->_projection_mat_left = lens_cdata->_projection_mat_right = lens_cdata->_projection_mat;

  } else {
    // Compute the left and right projection matrices in case this lens is
    // assigned to a stereo DisplayRegion.

    LVector3 iod = lens_cdata->_interocular_distance * 0.5f * LVector3::left(lens_cdata->_cs);
    lens_cdata->_projection_mat_left = do_get_lens_mat_inv(lens_cdata) * LMatrix4::translate_mat(-iod) * canonical * do_get_film_mat(lens_cdata);
    lens_cdata->_projection_mat_right = do_get_lens_mat_inv(lens_cdata) * LMatrix4::translate_mat(iod) * canonical * do_get_film_mat(lens_cdata);

    if ((lens_cdata->_user_flags & UF_convergence_distance) != 0 &&
        !cinf(lens_cdata->_convergence_distance)) {
      nassertv(lens_cdata->_convergence_distance != 0.0f);
      LVector3 cd;
      if (stereo_lens_old_convergence) { // The old, incorrect calculation was requested.
        cd = (0.25f / lens_cdata->_convergence_distance) * LVector3::left(lens_cdata->_cs);
      } else {
        const LVecBase2 &fov = do_get_fov(lens_cdata);
        cd = (2.0f / fov_to_film(fov[0], lens_cdata->_convergence_distance, true)) * iod;
      }
      lens_cdata->_projection_mat_left *= LMatrix4::translate_mat(cd);
      lens_cdata->_projection_mat_right *= LMatrix4::translate_mat(-cd);
    }
  }

  do_adjust_comp_flags(lens_cdata,
                       CF_projection_mat_inv | CF_projection_mat_left_inv | CF_projection_mat_right_inv,
                       CF_projection_mat);
}

/**
 * Given a field of view in degrees and a focal length, compute the
 * correspdonding width (or height) on the film.  If horiz is true, this is in
 * the horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat PerspectiveLens::
fov_to_film(PN_stdfloat fov, PN_stdfloat focal_length, bool) const {
  return (ctan(deg_2_rad(fov * 0.5f)) * focal_length) * 2.0f;
}

/**
 * Given a field of view in degrees and a width (or height) on the film,
 * compute the focal length of the lens.  If horiz is true, this is in the
 * horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat PerspectiveLens::
fov_to_focal_length(PN_stdfloat fov, PN_stdfloat film_size, bool) const {
  return film_size * 0.5f / ctan(deg_2_rad(fov * 0.5f));
}

/**
 * Given a width (or height) on the film and a focal length, compute the field
 * of view in degrees.  If horiz is true, this is in the horizontal direction;
 * otherwise, it is in the vertical direction (some lenses behave differently
 * in each direction).
 */
PN_stdfloat PerspectiveLens::
film_to_fov(PN_stdfloat film_size, PN_stdfloat focal_length, bool) const {
  return rad_2_deg(catan(film_size * 0.5f / focal_length)) * 2.0f;
}

/**
 * Tells the BamReader how to create objects of type Lens.
 */
void PerspectiveLens::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Lens is encountered in the Bam file.  It should create the Lens and
 * extract its information from the file.
 */
TypedWritable *PerspectiveLens::
make_from_bam(const FactoryParams &params) {
  PerspectiveLens *lens = new PerspectiveLens;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  lens->fillin(scan, manager);

  return lens;
}
