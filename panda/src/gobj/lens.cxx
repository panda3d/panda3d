/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lens.cxx
 * @author drose
 * @date 1999-02-18
 */

#include "lens.h"
#include "throw_event.h"
#include "compose_matrix.h"
#include "look_at.h"
#include "geom.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"
#include "boundingHexahedron.h"
#include "indent.h"
#include "config_gobj.h"
#include "plane.h"

using std::max;
using std::min;

TypeHandle Lens::_type_handle;
TypeHandle Lens::CData::_type_handle;

/**
 *
 */
Lens::
Lens() {
  clear();
}

/**
 *
 */
Lens::
Lens(const Lens &copy) : _cycler(copy._cycler) {
  // We don't copy the _geom_data.  That's unique to each Lens.
  CDWriter cdata(_cycler, true);
  cdata->_geom_data = nullptr;
}

/**
 *
 */
void Lens::
operator = (const Lens &copy) {
  _cycler = copy._cycler;

  // We don't copy the _geom_data.  That's unique to each Lens.
  CDWriter cdata(_cycler, true);
  cdata->_geom_data = nullptr;
}

/**
 * Specifies the coordinate system that all 3-d computations are performed
 * within for this Lens.  Normally, this is CS_default.
 */
void Lens::
set_coordinate_system(CoordinateSystem cs) {
  CDWriter cdata(_cycler, true);
  cdata->_cs = cs;
  do_adjust_comp_flags(cdata, CF_mat | CF_view_hpr | CF_view_vector, 0);
  do_throw_change_event(cdata);
}

/**
 * Resets all lens parameters to their initial default settings.
 */
void Lens::
clear() {
  CDWriter cdata(_cycler, true);
  cdata->clear();

  do_set_interocular_distance(cdata, default_iod);
  do_set_convergence_distance(cdata, default_converge);
  do_throw_change_event(cdata);
}

/**
 * Sets the field of view of the smallest dimension of the window.  If the
 * window is wider than it is tall, this specifies the vertical field of view;
 * if it is taller than it is wide, this specifies the horizontal field of
 * view.
 *
 * In many cases, this is preferable to setting either the horizontal or
 * vertical field of view explicitly.  Setting this parameter means that
 * pulling the window wider will widen the field of view, which is usually
 * what you expect to happen.
 */
void Lens::
set_min_fov(PN_stdfloat min_fov) {
  nassertv(!cnan(min_fov));
  CDWriter cdata(_cycler, true);
  cdata->_min_fov = min_fov;

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_fov_seq, cdata->_focal_length_seq, cdata->_film_size_seq);

  if (cdata->_focal_length_seq == 0) {
    // Throw out focal length if it's oldest.
    do_adjust_user_flags(cdata, UF_focal_length | UF_vfov | UF_hfov,
                         UF_min_fov);
  } else {
    // Otherwise, throw out film size.
    nassertv(cdata->_film_size_seq == 0);

    // Make sure we save the aspect ratio first.
    do_compute_aspect_ratio(cdata);
    do_adjust_user_flags(cdata, UF_film_width | UF_film_height | UF_vfov | UF_hfov,
                         UF_min_fov);
  }
  do_adjust_comp_flags(cdata, CF_mat | CF_focal_length | CF_fov | CF_film_size,
                       0);
  // We leave CF_fov off of comp_flags, because we will still need to
  // recompute the vertical fov.  It's not exactly the same as hfov *
  // get_aspect_ratio().
  do_throw_change_event(cdata);
}

/**
 * Returns the field of view of the narrowest dimension of the window.  See
 * set_min_fov().
 */
PN_stdfloat Lens::
get_min_fov() const {
  CDReader cdata(_cycler);

  if ((cdata->_comp_flags & CF_fov) == 0) {
    ((Lens *)this)->do_compute_fov((CData *)cdata.p());
  }
  return cdata->_min_fov;
}

/**
 * Returns the default near plane distance that will be assigned to each
 * newly-created lens.  This is read from the Config.prc file.
 */
PN_stdfloat Lens::
get_default_near() {
  return default_near;
}

/**
 * Returns the default far plane distance that will be assigned to each newly-
 * created lens.  This is read from the Config.prc file.
 */
PN_stdfloat Lens::
get_default_far() {
  return default_far;
}

/**
 * Sets the direction in which the lens is facing.  Normally, this is down the
 * forward axis (usually the Y axis), but it may be rotated.  This is only one
 * way of specifying the rotation; you may also specify an explicit vector in
 * which to look, or you may give a complete transformation matrix.
 */
void Lens::
set_view_hpr(const LVecBase3 &view_hpr) {
  nassertv(!view_hpr.is_nan());
  CDWriter cdata(_cycler, true);
  cdata->_view_hpr = view_hpr;
  do_adjust_user_flags(cdata, UF_view_vector | UF_view_mat,
                       UF_view_hpr);
  do_adjust_comp_flags(cdata, CF_mat | CF_view_vector,
                       CF_view_hpr);
  do_throw_change_event(cdata);
}

/**
 * Returns the direction in which the lens is facing.
 */
const LVecBase3 &Lens::
get_view_hpr() const {
  CDReader cdata(_cycler);
  if ((cdata->_comp_flags & CF_view_hpr) == 0) {
    ((Lens *)this)->do_compute_view_hpr((CData *)cdata.p());
  }
  return cdata->_view_hpr;
}

/**
 * Specifies the direction in which the lens is facing by giving an axis to
 * look along, and a perpendicular (or at least non-parallel) up axis.
 *
 * See also set_view_hpr().
 */
void Lens::
set_view_vector(const LVector3 &view_vector, const LVector3 &up_vector) {
  nassertv(!view_vector.is_nan());
  CDWriter cdata(_cycler, true);
  cdata->_view_vector = view_vector;
  cdata->_up_vector = up_vector;
  do_adjust_user_flags(cdata, UF_view_hpr | UF_view_mat,
                       UF_view_vector);
  do_adjust_comp_flags(cdata, CF_mat | CF_view_hpr,
                       CF_view_vector);
  do_throw_change_event(cdata);
}

/**
 * Returns the axis along which the lens is facing.
 */
const LVector3 &Lens::
get_view_vector() const {
  CDReader cdata(_cycler);
  if ((cdata->_comp_flags & CF_view_vector) == 0) {
    ((Lens *)this)->do_compute_view_vector((CData *)cdata.p());
  }
  return cdata->_view_vector;
}

/**
 * Returns the axis perpendicular to the camera's view vector that indicates
 * the "up" direction.
 */
const LVector3 &Lens::
get_up_vector() const {
  CDReader cdata(_cycler);
  if ((cdata->_comp_flags & CF_view_vector) == 0) {
    ((Lens *)this)->do_compute_view_vector((CData *)cdata.p());
  }
  return cdata->_up_vector;
}

/**
 * Returns the center point of the lens: the point from which the lens is
 * viewing.
 */
LPoint3 Lens::
get_nodal_point() const {
  return get_view_mat().get_row3(3);
}

/**
 * Resets the lens transform to identity.
 */
void Lens::
clear_view_mat() {
  CDWriter cdata(_cycler, true);
  cdata->_lens_mat = LMatrix4::ident_mat();
  do_adjust_user_flags(cdata, 0, UF_view_vector | UF_view_hpr | UF_view_mat);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_lens_mat_inv | CF_view_hpr | CF_view_vector,
                       CF_lens_mat);
  do_throw_change_event(cdata);
}

/**
 * Indicates the ratio of keystone correction to perform on the lens, in each
 * of three axes.  This will build a special non-affine scale factor into the
 * projection matrix that will compensate for keystoning of a projected image;
 * this can be used to compensate for a projector that for physical reasons
 * cannot be aimed directly at its screen.
 *
 * The default value is taken from the default-keystone Config variable.  0, 0
 * indicates no keystone correction; specify a small value (usually in the
 * range -1 .. 1) in either the x or y position to generate a keystone
 * correction in that axis.
 */
void Lens::
set_keystone(const LVecBase2 &keystone) {
  nassertv(!keystone.is_nan());
  CDWriter cdata(_cycler, true);
  cdata->_keystone = keystone;
  do_adjust_user_flags(cdata, 0, UF_keystone);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_film_mat | CF_film_mat_inv, 0);
  do_throw_change_event(cdata);
}

/**
 * Disables the lens keystone correction.
 */
void Lens::
clear_keystone() {
  CDWriter cdata(_cycler, true);
  cdata->_keystone.set(0.0f, 0.0f);
  do_adjust_user_flags(cdata, UF_keystone, 0);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_film_mat | CF_film_mat_inv, 0);
  do_throw_change_event(cdata);
}

/**
 * Specifies a custom matrix to transform the points on the film after they
 * have been converted into nominal film space (-1 .. 1 in U and V).  This can
 * be used to introduce arbitrary scales, rotations, or other linear
 * transforms to the media plane.  This is normally a 2-d matrix, but a full
 * 4x4 matrix may be specified.  This is applied on top of any film size, lens
 * shift, and/or keystone correction.
 */
void Lens::
set_custom_film_mat(const LMatrix4 &custom_film_mat) {
  nassertv(!custom_film_mat.is_nan());
  CDWriter cdata(_cycler, true);
  cdata->_custom_film_mat = custom_film_mat;
  do_adjust_user_flags(cdata, 0, UF_custom_film_mat);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_film_mat | CF_film_mat_inv, 0);
  do_throw_change_event(cdata);
}

/**
 * Disables the lens custom_film_mat correction.
 */
void Lens::
clear_custom_film_mat() {
  CDWriter cdata(_cycler, true);
  cdata->_custom_film_mat = LMatrix4::ident_mat();
  do_adjust_user_flags(cdata, UF_custom_film_mat, 0);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_film_mat | CF_film_mat_inv, 0);
  do_throw_change_event(cdata);
}

/**
 * Sets up the lens to use the frustum defined by the four indicated points.
 * This is most useful for a PerspectiveLens, but it may be called for other
 * kinds of lenses as well.
 *
 * The frustum will be rooted at the origin (or by whatever translation might
 * have been specified in a previous call to set_view_mat).
 *
 * It is legal for the four points not to be arranged in a rectangle; if this
 * is the case, the frustum will be fitted as tightly as possible to cover all
 * four points.
 *
 * The flags parameter contains the union of one or more of the following bits
 * to control the behavior of this function:
 *
 * FC_roll - If this is included, the camera may be rotated so that its up
 * vector is perpendicular to the top line.  Otherwise, the standard up vector
 * is used.
 *
 * FC_camera_plane - This allows the camera plane to be adjusted to be as
 * nearly perpendicular to the center of the frustum as possible.  Without
 * this bit, the orientation camera plane is defined by position of the four
 * points (which should all be coplanar).  With this bit, the camera plane is
 * arbitarary, and may be chosen so that the four points do not themselves lie
 * in the camera plane (but the points will still be within the frustum).
 *
 * FC_off_axis - This allows the resulting frustum to be off-axis to get the
 * tightest possible fit.  Without this bit, the viewing axis will be centered
 * within the frustum, but there may be more wasted space along the edges.
 *
 * FC_aspect_ratio - This allows the frustum to be scaled non-proportionately
 * in the vertical and horizontal dimensions, if necessary, to get a tighter
 * fit.  Without this bit, the current aspect ratio will be preserved.
 *
 * FC_shear - This allows the frustum to be sheared, if necessary, to get the
 * tightest possible fit.  This may result in a parallelogram-based frustum,
 * which will give a slanted appearance to the rendered image.  Without this
 * bit, the frustum will be rectangle-based.
 *
 * In general, if 0 is passed in as the value for flags, the generated frustum
 * will be a loose fit but sane; if -1 is passed in, it will be a tighter fit
 * and possibly screwy.
 */
void Lens::
set_frustum_from_corners(const LVecBase3 &ul, const LVecBase3 &ur,
                         const LVecBase3 &ll, const LVecBase3 &lr,
                         int flags) {
  nassertv(!ul.is_nan() && !ur.is_nan() && !ll.is_nan() && !lr.is_nan());

  CDWriter cdata(_cycler, true);
  // We'll need to know the pre-existing eyepoint translation from the center,
  // so we can preserve it in the new frustum.  This is usually (0, 0, 0), but
  // it could be an arbitrary vector.
  const LMatrix4 &lens_mat_inv = do_get_lens_mat_inv(cdata);
  LVector3 eye_offset;
  lens_mat_inv.get_row3(eye_offset, 3);

  // Now choose the viewing axis.  If FC_camera_plane is specified, we'll pass
  // it through the centroid for the best camera plane; otherwise, it's
  // perpendicular to the plane in which the points lie.
  LVector3 view_vector;
  if ((flags & FC_camera_plane) != 0) {
    view_vector = (ul + ur + ll + lr) * 0.25;
  } else {
    LPlane plane(ll, ul, ur);
    view_vector = plane.get_normal();
    nassertv(!view_vector.is_nan() && view_vector.length_squared() != 0.0f);
  }

  // Now determine the up axis.  If FC_roll is specified, or if our view
  // vector is straight up, it is the vector perpendicular to both the viewing
  // axis and the top line.  Otherwise, it is the standard up axis.
  LVector3 up_vector = LVector3::up(cdata->_cs);
  if (view_vector == up_vector || ((flags & FC_roll) != 0)) {
    LVector3 top = ul - ur;
    up_vector = view_vector.cross(top);
    nassertv(!up_vector.is_nan() && up_vector.length_squared() != 0.0f);
  }

  // Now compute the matrix that applies this rotation.
  LMatrix4 rot_mat;
  look_at(rot_mat, view_vector, up_vector, CS_zup_right);

  // And invert it.
  LMatrix4 inv_rot_mat;
  inv_rot_mat.invert_affine_from(rot_mat);

  // Use that inverse matrix to convert the four corners to a local coordinate
  // system, looking down the Y axis.
  LPoint3 cul = inv_rot_mat.xform_point(ul);
  LPoint3 cur = inv_rot_mat.xform_point(ur);
  LPoint3 cll = inv_rot_mat.xform_point(ll);
  LPoint3 clr = inv_rot_mat.xform_point(lr);

  // Project all points into the Y == 1 plane, so we can do 2-d manipulation
  // on them.
  nassertv(cul[1] != 0.0f && cur[1] != 0.0f && cll[1] != 0.0f && clr[1] != 0.0f);
  cul /= cul[1];
  cur /= cur[1];
  cll /= cll[1];
  clr /= clr[1];

  LMatrix4 shear_mat = LMatrix4::ident_mat();
  LMatrix4 inv_shear_mat = LMatrix4::ident_mat();

  // Now, if we're allowed to shear the frustum, do so.
  if ((flags & FC_shear) != 0) {
    build_shear_mat(shear_mat, cul, cur, cll, clr);
    inv_shear_mat.invert_from(shear_mat);
  }

  // Now build the complete view matrix.
  LMatrix4 inv_view_mat =
    inv_rot_mat *
    inv_shear_mat;

  // And reapply the eye offset to this matrix.
  inv_view_mat.set_row(3, eye_offset);

  LMatrix4 view_mat;
  view_mat.invert_from(inv_view_mat);
  do_set_view_mat(cdata, view_mat);

  LPoint3 ful = inv_view_mat.xform_point(ul);
  LPoint3 fur = inv_view_mat.xform_point(ur);
  LPoint3 fll = inv_view_mat.xform_point(ll);
  LPoint3 flr = inv_view_mat.xform_point(lr);

  // Normalize *these* points into the y == 1 plane.
  nassertv(ful[1] != 0.0f && fur[1] != 0.0f && fll[1] != 0.0f && flr[1] != 0.0f);
  ful /= ful[1];
  fur /= fur[1];
  fll /= fll[1];
  flr /= flr[1];

  // Determine the minimum field of view necesary to cover all four
  // transformed points.
  PN_stdfloat min_x = min(min(ful[0], fur[0]), min(fll[0], flr[0]));
  PN_stdfloat max_x = max(max(ful[0], fur[0]), max(fll[0], flr[0]));
  PN_stdfloat min_z = min(min(ful[2], fur[2]), min(fll[2], flr[2]));
  PN_stdfloat max_z = max(max(ful[2], fur[2]), max(fll[2], flr[2]));

  PN_stdfloat x_spread, x_center, z_spread, z_center;

  if ((flags & FC_off_axis) != 0) {
    // If we're allowed to make an off-axis projection, then pick the best
    // center.
    x_center = (max_x + min_x) * 0.5f;
    z_center = (max_z + min_z) * 0.5f;
    x_spread = x_center - min_x;
    z_spread = z_center - min_z;
  } else {
    // Otherwise, the center must be (0, 0).
    x_center = 0.0f;
    z_center = 0.0f;
    x_spread = max(cabs(max_x), cabs(min_x));
    z_spread = max(cabs(max_z), cabs(min_z));
  }

  PN_stdfloat aspect_ratio = do_get_aspect_ratio(cdata);
  nassertv(aspect_ratio != 0.0f);
  if ((flags & FC_aspect_ratio) == 0) {
    // If we must preserve the aspect ratio, then the x and z spreads must be
    // adjusted to match.
    if (x_spread < z_spread * aspect_ratio) {
      // x_spread is too small.
      x_spread = z_spread * aspect_ratio;
    } else if (z_spread < x_spread / aspect_ratio) {
      // z_spread is too small.
      z_spread = x_spread / aspect_ratio;
    }
  }

  PN_stdfloat hfov = rad_2_deg(catan(x_spread)) * 2.0f;
  PN_stdfloat vfov = rad_2_deg(catan(z_spread)) * 2.0f;

  do_set_fov(cdata, LVecBase2(hfov, vfov));

  if ((flags & FC_aspect_ratio) == 0) {
    // If we must preserve the aspect ratio, store it one more time.  This is
    // mainly in case we have a non-perspective lens with a funny relationship
    // between fov and aspect ratio.
    do_set_aspect_ratio(cdata, aspect_ratio);
  }

  const LVecBase2 &film_size = do_get_film_size(cdata);
  nassertv(x_spread != 0.0f && z_spread != 0.0f);
  do_set_film_offset(cdata, LVecBase2(film_size[0] * x_center / (x_spread * 2.0f),
                                      film_size[1] * z_center / (z_spread * 2.0f)));
}


/**
 * Forces all internal parameters of the Lens to be recomputed.  Normally,
 * this should never need to be called; it is provided only to assist in
 * debugging.
 */
void Lens::
recompute_all() {
  CDWriter cdata(_cycler);
  cdata->_comp_flags = 0;
}

/**
 * Returns true if the lens represents a linear projection (e.g.
 * PerspectiveLens, OrthographicLens), and therefore there is a valid matrix
 * returned by get_projection_mat(), or false otherwise.
 */
bool Lens::
is_linear() const {
  return false;
}

/**
 * Returns true if the lens represents a perspective projection (i.e.  it is a
 * PerspectiveLens), false otherwise.
 */
bool Lens::
is_perspective() const {
  return false;
}

/**
 * Returns true if the lens represents a orthographic projection (i.e.  it is
 * a OrthographicLens), false otherwise.
 */
bool Lens::
is_orthographic() const {
  return false;
}

/**
 * Allocates and returns a new Geom that can be rendered to show a visible
 * representation of the frustum used for this kind of lens, if it makes sense
 * to do so.  If a visible representation cannot be created, returns NULL.
 */
PT(Geom) Lens::
make_geometry() {
  CDWriter cdata(_cycler, true);

  // The default behavior for make_geometry() will be to draw a hexahedron
  // around the eight vertices of the frustum.  If the lens is non-linear, the
  // hexahedron will be curved; in that case, we'll subdivide the lines into
  // several segments to get an approximation of the curve.

  // First, define all the points we'll use in this Geom.  That's one point at
  // each corner of the near and far planes (and possibly more points along
  // the edges).
  int num_segments = do_define_geom_data(cdata);
  if (num_segments == 0) {
    // Can't do a frustum.
    cdata->_geom_data.clear();
    return nullptr;
  }

  // Now string together the line segments.
  PT(GeomLinestrips) line = new GeomLinestrips(Geom::UH_static);

  // Draw a frame around the near plane.
  int i, si;
  for (i = 0; i < 4; ++i) {
    for (si = 0; si < num_segments; ++si) {
      line->add_vertex(i * 2 + si * (4 * 2) + 0);
    }
  }
  line->add_vertex(0);
  line->close_primitive();

  // Draw a frame around the far plane.
  for (i = 0; i < 4; ++i) {
    for (si = 0; si < num_segments; ++si) {
      line->add_vertex(i * 2 + si * (4 * 2) + 1);
    }
  }
  line->add_vertex(1);
  line->close_primitive();

  // Draw connecting lines at the corners.
  line->add_vertex(0 * 2 + 0);
  line->add_vertex(0 * 2 + 1);
  line->close_primitive();

  line->add_vertex(1 * 2 + 0);
  line->add_vertex(1 * 2 + 1);
  line->close_primitive();

  line->add_vertex(2 * 2 + 0);
  line->add_vertex(2 * 2 + 1);
  line->close_primitive();

  line->add_vertex(3 * 2 + 0);
  line->add_vertex(3 * 2 + 1);
  line->close_primitive();

  // And one more line for the viewing axis.
  line->add_vertex(num_segments * (4 * 2) + 0);
  line->add_vertex(num_segments * (4 * 2) + 1);
  line->close_primitive();

  PT(Geom) geom = new Geom(cdata->_geom_data);
  geom->add_primitive(line);

  return geom;
}

/**
 * Allocates and returns a new BoundingVolume that encloses the frustum used
 * for this kind of lens, if possible.  If a suitable bounding volume cannot
 * be created, returns NULL.
 */
PT(BoundingVolume) Lens::
make_bounds() const {
  CDReader cdata(_cycler);

  // The default bounding volume is a hexahedron based on the eight corners of
  // the frustum.
  LPoint3 fll, flr, ful, fur;
  LPoint3 nll, nlr, nul, nur;
  LPoint3 corner;

  // Upper left.
  corner.set(-1.0f, 1.0f, 0.0f);
  if (!do_extrude(cdata, corner, nul, ful)) {
    return nullptr;
  }

  // Upper right.
  corner.set(1.0f, 1.0f, 0.0f);
  if (!do_extrude(cdata, corner, nur, fur)) {
    return nullptr;
  }

  // Lower right.
  corner.set(1.0f, -1.0f, 0.0f);
  if (!do_extrude(cdata, corner, nlr, flr)) {
    return nullptr;
  }

  // Lower left.
  corner.set(-1.0f, -1.0f, 0.0f);
  if (!do_extrude(cdata, corner, nll, fll)) {
    return nullptr;
  }

  return new BoundingHexahedron(fll, flr, fur, ful, nll, nlr, nur, nul);
}

/**
 *
 */
void Lens::
output(std::ostream &out) const {
  out << get_type();
}

/**
 *
 */
void Lens::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << " fov = " << get_fov() << "\n";
}

/**
 *
 */
void Lens::
do_set_film_size(CData *cdata, PN_stdfloat width) {
  nassertv(!cnan(width));
  cdata->_film_size.set(width, width / do_get_aspect_ratio(cdata));

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_film_size_seq, cdata->_focal_length_seq, cdata->_fov_seq);

  if (cdata->_fov_seq == 0) {
    // Throw out fov if it's oldest.
    do_adjust_user_flags(cdata, UF_hfov | UF_vfov | UF_min_fov | UF_film_height,
                         UF_film_width);
  } else {
    // Otherwise, throw out focal length.
    nassertv(cdata->_focal_length_seq == 0);
    do_adjust_user_flags(cdata, UF_focal_length | UF_film_height,
                         UF_film_width);
  }
  do_adjust_comp_flags(cdata, CF_mat | CF_focal_length | CF_fov,
                       CF_film_size);
  do_throw_change_event(cdata);
}

/**
 *
 */
void Lens::
do_set_film_size(CData *cdata, const LVecBase2 &film_size) {
  nassertv(!film_size.is_nan());
  cdata->_film_size = film_size;

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_film_size_seq, cdata->_focal_length_seq, cdata->_fov_seq);

  if (cdata->_fov_seq == 0) {
    // Throw out fov if it's oldest.
    do_adjust_user_flags(cdata, UF_hfov | UF_vfov | UF_min_fov | UF_aspect_ratio,
                         UF_film_width | UF_film_height);
  } else {
    // Otherwise, throw out focal length.
    nassertv(cdata->_focal_length_seq == 0);
    do_adjust_user_flags(cdata, UF_focal_length | UF_vfov | UF_aspect_ratio,
                         UF_film_width | UF_film_height);
  }
  do_adjust_comp_flags(cdata, CF_mat | CF_focal_length | CF_fov | CF_aspect_ratio,
                       CF_film_size);

  // Also, the user has implicitly specified an aspect ratio.  Make it stick
  // until the user tells us something different.
  do_compute_aspect_ratio(cdata);
  do_adjust_user_flags(cdata, 0, UF_aspect_ratio);

  do_throw_change_event(cdata);
}

/**
 *
 */
const LVecBase2 &Lens::
do_get_film_size(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_film_size) == 0) {
    // We pretend this is a const method, even though it may call a non-const
    // method to recompute the internal values.  We can do this because this
    // is just compute-on-demand.
    ((Lens *)this)->do_compute_film_size((CData *)cdata);
  }
  return cdata->_film_size;
}

/**
 *
 */
void Lens::
do_set_focal_length(CData *cdata, PN_stdfloat focal_length) {
  nassertv(!cnan(focal_length));
  cdata->_focal_length = focal_length;

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_focal_length_seq, cdata->_film_size_seq, cdata->_fov_seq);

  if (cdata->_film_size_seq == 0) {
    // Throw out film size if it's oldest.
    do_adjust_user_flags(cdata, UF_film_width | UF_film_height,
                         UF_focal_length);
  } else {
    // Otherwise, throw out the fov.
    nassertv(cdata->_fov_seq == 0);
    do_adjust_user_flags(cdata, UF_hfov | UF_vfov | UF_min_fov,
                         UF_focal_length);
  }

  do_adjust_comp_flags(cdata, CF_mat | CF_fov | CF_film_size,
                       CF_focal_length);
  do_throw_change_event(cdata);
}

/**
 *
 */
PN_stdfloat Lens::
do_get_focal_length(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_focal_length) == 0) {
    ((Lens *)this)->do_compute_focal_length((CData *)cdata);
  }
  return cdata->_focal_length;
}

/**
 *
 */
void Lens::
do_set_fov(CData *cdata, PN_stdfloat hfov) {
  nassertv(!cnan(hfov));
  cdata->_fov[0] = hfov;

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_fov_seq, cdata->_focal_length_seq, cdata->_film_size_seq);

  if (cdata->_focal_length_seq == 0) {
    // Throw out focal length if it's oldest.
    do_adjust_user_flags(cdata, UF_focal_length | UF_vfov | UF_min_fov,
                         UF_hfov);
  } else {
    // Otherwise, throw out film size.
    nassertv(cdata->_film_size_seq == 0);

    // Make sure we save the aspect ratio first.
    do_compute_aspect_ratio(cdata);
    do_adjust_user_flags(cdata, UF_film_width | UF_film_height | UF_vfov | UF_min_fov,
                         UF_hfov);
  }
  do_adjust_comp_flags(cdata, CF_mat | CF_focal_length | CF_fov | CF_film_size,
                       0);
  // We leave CF_fov off of comp_flags, because we will still need to
  // recompute the vertical fov.  It's not exactly the same as hfov *
  // get_aspect_ratio().
  do_throw_change_event(cdata);
}

/**
 *
 */
void Lens::
do_set_fov(CData *cdata, const LVecBase2 &fov) {
  nassertv(!fov.is_nan());
  cdata->_fov = fov;

  // We can't specify all three of focal length, fov, and film size.  Throw
  // out the oldest one.
  do_resequence_fov_triad(cdata, cdata->_fov_seq, cdata->_focal_length_seq, cdata->_film_size_seq);

  if (cdata->_focal_length_seq == 0) {
    // Throw out focal length if it's oldest.
    do_adjust_user_flags(cdata, UF_focal_length | UF_film_height | UF_min_fov | UF_aspect_ratio,
                         UF_hfov | UF_vfov);
  } else {
    // Otherwise, throw out film size.
    nassertv(cdata->_film_size_seq == 0);
    do_adjust_user_flags(cdata, UF_film_width | UF_film_height | UF_min_fov | UF_aspect_ratio,
                         UF_hfov | UF_vfov);
  }
  do_adjust_comp_flags(cdata, CF_mat | CF_focal_length | CF_film_size | CF_aspect_ratio,
                       CF_fov);

  // Also, the user has implicitly specified an aspect ratio.  Make it stick
  // until the user tells us something different.
  do_compute_aspect_ratio(cdata);
  do_adjust_user_flags(cdata, 0, UF_aspect_ratio);

  do_throw_change_event(cdata);
}

/**
 *
 */
const LVecBase2 &Lens::
do_get_fov(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_fov) == 0) {
    ((Lens *)this)->do_compute_fov((CData *)cdata);
  }
  return cdata->_fov;
}

/**
 *
 */
void Lens::
do_set_aspect_ratio(CData *cdata, PN_stdfloat aspect_ratio) {
  nassertv(!cnan(aspect_ratio));
  cdata->_aspect_ratio = aspect_ratio;
  do_adjust_user_flags(cdata, UF_film_height | UF_vfov,
                       UF_aspect_ratio);
  do_adjust_comp_flags(cdata, CF_mat | CF_film_size | CF_fov | CF_focal_length,
                       CF_aspect_ratio);
  do_throw_change_event(cdata);
}

/**
 *
 */
PN_stdfloat Lens::
do_get_aspect_ratio(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_aspect_ratio) == 0) {
    ((Lens *)this)->do_compute_aspect_ratio((CData *)cdata);
  }
  return cdata->_aspect_ratio;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_projection_mat(const CData *cdata, StereoChannel channel) const {
  if ((cdata->_comp_flags & CF_projection_mat) == 0) {
    ((Lens *)this)->do_compute_projection_mat((CData *)cdata);
  }

  switch (channel) {
  case SC_left:
    return cdata->_projection_mat_left;
  case SC_right:
    return cdata->_projection_mat_right;
  case SC_mono:
  case SC_stereo:
    return cdata->_projection_mat;
  }

  return cdata->_projection_mat;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_projection_mat_inv(const CData *cdata, StereoChannel stereo_channel) const {
  switch (stereo_channel) {
  case SC_left:
    {
      if ((cdata->_comp_flags & CF_projection_mat_left_inv) == 0) {
        const LMatrix4 &projection_mat_left = do_get_projection_mat(cdata, SC_left);
        ((CData *)cdata)->_projection_mat_left_inv.invert_from(projection_mat_left);
        ((Lens *)this)->do_adjust_comp_flags((CData *)cdata, 0, CF_projection_mat_left_inv);
      }
    }
    return cdata->_projection_mat_left_inv;

  case SC_right:
    {
      if ((cdata->_comp_flags & CF_projection_mat_right_inv) == 0) {
        const LMatrix4 &projection_mat_right = do_get_projection_mat(cdata, SC_right);
        ((CData *)cdata)->_projection_mat_right_inv.invert_from(projection_mat_right);
        ((Lens *)this)->do_adjust_comp_flags((CData *)cdata, 0, CF_projection_mat_right_inv);
      }
    }
    return cdata->_projection_mat_right_inv;

  case SC_mono:
  case SC_stereo:
    break;
  }

  if ((cdata->_comp_flags & CF_projection_mat_inv) == 0) {
    const LMatrix4 &projection_mat = do_get_projection_mat(cdata);
    ((CData *)cdata)->_projection_mat_inv.invert_from(projection_mat);
    ((Lens *)this)->do_adjust_comp_flags((CData *)cdata, 0, CF_projection_mat_inv);
  }
  return cdata->_projection_mat_inv;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_film_mat(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_film_mat) == 0) {
    ((Lens *)this)->do_compute_film_mat((CData *)cdata);
  }
  return cdata->_film_mat;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_film_mat_inv(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_film_mat_inv) == 0) {
    const LMatrix4 &film_mat = do_get_film_mat(cdata);
    ((CData *)cdata)->_film_mat_inv.invert_from(film_mat);
    ((Lens *)this)->do_adjust_comp_flags((CData *)cdata, 0, CF_film_mat_inv);
  }
  return cdata->_film_mat_inv;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_lens_mat(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_lens_mat) == 0) {
    ((Lens *)this)->do_compute_lens_mat((CData *)cdata);
  }
  return cdata->_lens_mat;
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_lens_mat_inv(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_lens_mat_inv) == 0) {
    const LMatrix4 &lens_mat = do_get_lens_mat(cdata);
    ((CData *)cdata)->_lens_mat_inv.invert_from(lens_mat);
    ((Lens *)this)->do_adjust_comp_flags((CData *)cdata, 0, CF_lens_mat_inv);
  }
  return cdata->_lens_mat_inv;
}

/**
 *
 */
void Lens::
do_set_interocular_distance(CData *cdata, PN_stdfloat interocular_distance) {
  nassertv(!cnan(interocular_distance));
  cdata->_interocular_distance = interocular_distance;
  if (cdata->_interocular_distance == 0.0f) {
    do_adjust_user_flags(cdata, UF_interocular_distance, 0);
  } else {
    do_adjust_user_flags(cdata, 0, UF_interocular_distance);
  }

  do_adjust_comp_flags(cdata, CF_mat, 0);
}

/**
 *
 */
void Lens::
do_set_convergence_distance(CData *cdata, PN_stdfloat convergence_distance) {
  nassertv(!cnan(convergence_distance));
  cdata->_convergence_distance = convergence_distance;
  if (cdata->_convergence_distance == 0.0f) {
    do_adjust_user_flags(cdata, UF_convergence_distance, 0);
  } else {
    do_adjust_user_flags(cdata, 0, UF_convergence_distance);
  }

  do_adjust_comp_flags(cdata, CF_mat, 0);
}

/**
 *
 */
void Lens::
do_set_view_mat(CData *cdata, const LMatrix4 &view_mat) {
  nassertv(!view_mat.is_nan());
  cdata->_lens_mat = view_mat;
  do_adjust_user_flags(cdata, UF_view_vector | UF_view_hpr,
                       UF_view_mat);
  do_adjust_comp_flags(cdata, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv |
                       CF_lens_mat_inv | CF_view_hpr | CF_view_vector,
                       CF_lens_mat);
  do_throw_change_event(cdata);
}

/**
 *
 */
const LMatrix4 &Lens::
do_get_view_mat(const CData *cdata) const {
  if ((cdata->_comp_flags & CF_lens_mat) == 0) {
    ((Lens *)this)->do_compute_lens_mat((CData *)cdata);
  }
  return cdata->_lens_mat;
}

/**
 * Throws the event associated with changing properties on this Lens, if any.
 */
void Lens::
do_throw_change_event(CData *cdata) {
  ++(cdata->_last_change);

  if (!cdata->_change_event.empty()) {
    throw_event(cdata->_change_event, this);
  }

  if (!cdata->_geom_data.is_null()) {
    if (cdata->_geom_data->get_ref_count() == 1) {
      // No one's using the data any more (there are no references to it other
      // than this one), so don't bother to recompute it; just release it.
      cdata->_geom_data.clear();
    } else {
      // Someone still has a handle to the data, so recompute it for them.
      do_define_geom_data(cdata);
    }
  }
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * corresponding vector in space that maps to this point, if such a vector can
 * be determined.  The vector is returned by indicating the points on the near
 * plane and far plane that both map to the indicated 2-d point.
 *
 * The z coordinate of the 2-d point is ignored.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
bool Lens::
do_extrude(const CData *cdata,
           const LPoint3 &point2d, LPoint3 &near_point, LPoint3 &far_point) const {
  const LMatrix4 &projection_mat_inv = do_get_projection_mat_inv(cdata);
  {
    LVecBase4 full(point2d[0], point2d[1], -1.0f, 1.0f);
    full = projection_mat_inv.xform(full);

    PN_stdfloat recip_full3 = 1.0 / max((double)full[3], (double)lens_far_limit);
    near_point.set(full[0] * recip_full3,
                   full[1] * recip_full3,
                   full[2] * recip_full3);
  }
  {
    LVecBase4 full(point2d[0], point2d[1], 1.0f, 1.0f);
    full = projection_mat_inv.xform(full);

    // We can truncate the weight factor at near 0.  If it goes too close to
    // zero, or becomes negative, the far plane moves out past infinity and
    // comes back in behind the lens, which is just crazy.  Truncating it to
    // zero keeps the far plane from moving too far out.
    PN_stdfloat recip_full3 = 1.0 / max((double)full[3], (double)lens_far_limit);
    far_point.set(full[0] * recip_full3,
                  full[1] * recip_full3,
                  full[2] * recip_full3);
  }
  return true;
}

/**
 * This is the generic implementation, which is based on do_extrude() and
 * assumes a linear distribution of depth values between the near and far
 * points.
 */
bool Lens::
do_extrude_depth(const CData *cdata,
                 const LPoint3 &point2d, LPoint3 &point3d) const {
  LPoint3 near_point, far_point;
  bool result = extrude(point2d, near_point, far_point);

  // The depth point is, by convention, in the range -1 to 1.  Scale this to 0
  // .. 1 for the linear interpolation.
  PN_stdfloat t = point2d[2] * 0.5 + 0.5;
  point3d = near_point + (far_point - near_point) * t;
  return result;
}

/**
 * Implements do_extrude_depth() by using the projection matrix.  This is
 * efficient, but works only for a linear (Perspective or Orthographic) lens.
 */
bool Lens::
do_extrude_depth_with_mat(const CData *cdata,
                          const LPoint3 &point2d, LPoint3 &point3d) const {
  const LMatrix4 &projection_mat_inv = do_get_projection_mat_inv(cdata);
  point3d = projection_mat_inv.xform_point_general(point2d);
  return true;
}

/**
 * Given a 2-d point in the range (-1,1) in both dimensions, where (0,0) is
 * the center of the lens and (-1,-1) is the lower-left corner, compute the
 * vector that corresponds to the view direction.  This will be parallel to
 * the normal on the surface (the far plane) corresponding to the lens shape
 * at this point.
 *
 * Generally, for all rational lenses, the center of the film at (0,0)
 * computes a vector that is in the same direction as the vector specified by
 * set_view_vector().
 *
 * For all linear lenses, including perspective and orthographic lenses, all
 * points on the film compute this same vector (the far plane is a flat plane,
 * so the normal is the same everywhere).  For curved lenses like fisheye and
 * cylindrical lenses, different points may compute different vectors (the far
 * "plane" on these lenses is a curved surface).
 *
 * The z coordinate of the 2-d point is ignored.
 *
 * Returns true if the vector is defined, or false otherwise.
 */
bool Lens::
do_extrude_vec(const CData *cdata, const LPoint3 &point2d, LVector3 &vec) const {
  vec = LVector3::forward(cdata->_cs) * do_get_lens_mat(cdata);
  return true;
}

/**
 * Given a 3-d point in space, determine the 2-d point this maps to, in the
 * range (-1,1) in both dimensions, where (0,0) is the center of the lens and
 * (-1,-1) is the lower-left corner.
 *
 * The z coordinate will also be set to a value in the range (-1, 1), where -1
 * represents a point on the near plane, and 1 represents a point on the far
 * plane.
 *
 * Returns true if the 3-d point is in front of the lens and within the
 * viewing frustum (in which case point2d is filled in), or false otherwise
 * (in which case point2d will be filled in with something, which may or may
 * not be meaningful).
 */
bool Lens::
do_project(const CData *cdata, const LPoint3 &point3d, LPoint3 &point2d) const {
  const LMatrix4 &projection_mat = do_get_projection_mat(cdata);
  LVecBase4 full(point3d[0], point3d[1], point3d[2], 1.0f);
  full = projection_mat.xform(full);
  if (full[3] == 0.0f) {
    point2d.set(0.0f, 0.0f, 0.0f);
    return false;
  }
  PN_stdfloat recip_full3 = 1.0f/full[3];
  point2d.set(full[0] * recip_full3, full[1] * recip_full3, full[2] * recip_full3);
  return
    (full[3] > 0.0f) &&
    (point2d[0] >= -1.0f - NEARLY_ZERO(PN_stdfloat)) && (point2d[0] <= 1.0f + NEARLY_ZERO(PN_stdfloat)) &&
    (point2d[1] >= -1.0f - NEARLY_ZERO(PN_stdfloat)) && (point2d[1] <= 1.0f + NEARLY_ZERO(PN_stdfloat));
}

/**
 * Computes the size and shape of the film behind the camera, based on the
 * aspect ratio and fov.
 */
void Lens::
do_compute_film_size(CData *cdata) {
  if ((cdata->_user_flags & (UF_min_fov | UF_focal_length)) == (UF_min_fov | UF_focal_length)) {
    // If we just have a min FOV and a focal length, that determines the
    // smaller of the two film_sizes, and the larger is simply chosen
    // according to the aspect ratio.
    PN_stdfloat fs = fov_to_film(cdata->_min_fov, cdata->_focal_length, true);
    nassertv((cdata->_user_flags & UF_aspect_ratio) != 0 ||
             (cdata->_comp_flags & CF_aspect_ratio) != 0);

    if (cdata->_aspect_ratio < 1.0f) {
      cdata->_film_size[1] = fs / cdata->_aspect_ratio;
      cdata->_film_size[0] = fs;

    } else {
      cdata->_film_size[0] = fs * cdata->_aspect_ratio;
      cdata->_film_size[1] = fs;
    }

  } else {
    if ((cdata->_user_flags & UF_film_width) == 0) {
      if ((cdata->_user_flags & (UF_hfov | UF_focal_length)) == (UF_hfov | UF_focal_length)) {
        cdata->_film_size[0] = fov_to_film(cdata->_fov[0], cdata->_focal_length, true);
      } else {
        cdata->_film_size[0] = 1.0f;
      }
    }

    if ((cdata->_user_flags & UF_film_height) == 0) {
      if ((cdata->_user_flags & (UF_vfov | UF_focal_length)) == (UF_vfov | UF_focal_length)) {
        cdata->_film_size[1] = fov_to_film(cdata->_fov[1], cdata->_focal_length, false);

      } else if ((cdata->_user_flags & (UF_hfov | UF_vfov)) == (UF_hfov | UF_vfov)) {
        // If we don't have a focal length, but we have an explicit vfov and
        // hfov, we can infer the focal length is whatever makes the film
        // width, above, be what it is.
        if ((cdata->_comp_flags & CF_focal_length) == 0) {
          cdata->_focal_length = fov_to_focal_length(cdata->_fov[0], cdata->_film_size[0], true);
          do_adjust_comp_flags(cdata, 0, CF_focal_length);
        }
        cdata->_film_size[1] = fov_to_film(cdata->_fov[1], cdata->_focal_length, false);

      } else if ((cdata->_user_flags & UF_aspect_ratio) != 0 ||
                 (cdata->_comp_flags & CF_aspect_ratio) != 0) {
        cdata->_film_size[1] = cdata->_film_size[0] / cdata->_aspect_ratio;

      } else {
        // Default is an aspect ratio of 1.
        cdata->_film_size[1] = cdata->_film_size[0];
      }
    }
  }

  do_adjust_comp_flags(cdata, 0, CF_film_size);
}

/**
 * Computes the focal length of the lens, based on the fov and film size.
 * This is based on the horizontal dimension.
 */
void Lens::
do_compute_focal_length(CData *cdata) {
  if ((cdata->_user_flags & UF_focal_length) == 0) {
    const LVecBase2 &film_size = do_get_film_size(cdata);
    const LVecBase2 &fov = do_get_fov(cdata);
    cdata->_focal_length = fov_to_focal_length(fov[0], film_size[0], true);
  }

  do_adjust_comp_flags(cdata, 0, CF_focal_length);
}

/**
 * Computes the field of view of the lens, based on the film size and focal
 * length.
 */
void Lens::
do_compute_fov(CData *cdata) {
  const LVecBase2 &film_size = do_get_film_size(cdata);

  bool got_hfov = ((cdata->_user_flags & UF_hfov) != 0);
  bool got_vfov = ((cdata->_user_flags & UF_vfov) != 0);
  bool got_min_fov = ((cdata->_user_flags & UF_min_fov) != 0);

  if (!got_hfov && !got_vfov && !got_min_fov) {
    // If the user hasn't specified any FOV, we have to compute it.
    if ((cdata->_user_flags & UF_focal_length) != 0) {
      // The FOV is determined from the film size and focal length.
      cdata->_fov[0] = film_to_fov(film_size[0], cdata->_focal_length, true);
      cdata->_fov[1] = film_to_fov(film_size[1], cdata->_focal_length, true);
      got_hfov = true;
      got_vfov = true;

    } else {
      // We can't compute the FOV; take the default.
      cdata->_min_fov = default_fov;
      got_min_fov = true;
    }
  }

  if (got_min_fov) {
    // If we have just a min_fov, use it to derive whichever fov is smaller.
    if (film_size[0] < film_size[1]) {
      cdata->_fov[0] = cdata->_min_fov;
      got_hfov = true;
    } else {
      cdata->_fov[1] = cdata->_min_fov;
      got_vfov = true;
    }
  }

  // Now compute whichever fov is remaining.
  if (!got_hfov) {
    if ((cdata->_user_flags & UF_focal_length) == 0 &&
        (cdata->_comp_flags & CF_focal_length) == 0) {
      // If we don't have an explicit focal length, we can infer it from the
      // above.
      nassertv(got_vfov);
      cdata->_focal_length = fov_to_focal_length(cdata->_fov[1], film_size[1], true);
      do_adjust_comp_flags(cdata, 0, CF_focal_length);
    }
    cdata->_fov[0] = film_to_fov(film_size[0], cdata->_focal_length, false);
    got_hfov = true;
  }

  if (!got_vfov) {
    if ((cdata->_user_flags & UF_focal_length) == 0 &&
        (cdata->_comp_flags & CF_focal_length) == 0) {
      // If we don't have an explicit focal length, we can infer it from the
      // above.
      nassertv(got_hfov);
      cdata->_focal_length = fov_to_focal_length(cdata->_fov[0], film_size[0], true);
      do_adjust_comp_flags(cdata, 0, CF_focal_length);
    }
    cdata->_fov[1] = film_to_fov(film_size[1], cdata->_focal_length, false);
    got_vfov = true;
  }

  if (!got_min_fov) {
    cdata->_min_fov = film_size[0] < film_size[1] ? cdata->_fov[0] : cdata->_fov[1];
    got_min_fov = true;
  }

  nassertv(got_hfov && got_vfov && got_min_fov);
  do_adjust_comp_flags(cdata, 0, CF_fov);
}

/**
 * Computes the aspect ratio of the film rectangle, as a ratio of width to
 * height.
 */
void Lens::
do_compute_aspect_ratio(CData *cdata) {
  if ((cdata->_user_flags & UF_aspect_ratio) == 0) {
    const LVecBase2 &film_size = do_get_film_size(cdata);
    if (film_size[1] == 0.0f) {
      cdata->_aspect_ratio = 1.0f;
    } else {
      cdata->_aspect_ratio = film_size[0] / film_size[1];
    }
  }
  do_adjust_comp_flags(cdata, 0, CF_aspect_ratio);
}

/**
 * Computes the Euler angles representing the lens' rotation.
 */
void Lens::
do_compute_view_hpr(CData *cdata) {
  if ((cdata->_user_flags & UF_view_hpr) == 0) {
    const LMatrix4 &view_mat = do_get_view_mat(cdata);
    LVecBase3 scale, shear, translate;
    decompose_matrix(view_mat, scale, shear, cdata->_view_hpr, translate, cdata->_cs);
  }
  do_adjust_comp_flags(cdata, 0, CF_view_hpr);
}

/**
 * Computes the view vector and up vector for the lens.
 */
void Lens::
do_compute_view_vector(CData *cdata) {
  if ((cdata->_user_flags & UF_view_vector) == 0) {
    const LMatrix4 &view_mat = do_get_view_mat(cdata);
    cdata->_view_vector = LVector3::forward(cdata->_cs) * view_mat;
    cdata->_up_vector = LVector3::up(cdata->_cs) * view_mat;
  }
  do_adjust_comp_flags(cdata, 0, CF_view_vector);
}

/**
 * Computes the complete transformation matrix from 3-d point to 2-d point, if
 * the lens is linear.
 */
void Lens::
do_compute_projection_mat(CData *lens_cdata) {
  // This is the implementation used by non-linear lenses.  The linear lenses
  // (PerspectiveLens and OrthographicLens) will customize this method
  // appropriate for themselves.

  // By convention, the coordinate-system conversion is baked into the
  // projection mat.  Our non-linear lenses are implemented with code that
  // assumes CS_zup_right, so we bake the appropriate rotation in here.
  CoordinateSystem cs = lens_cdata->_cs;
  if (cs == CS_default) {
    cs = get_default_coordinate_system();
  }
  lens_cdata->_projection_mat = LMatrix4::convert_mat(cs, CS_zup_right);
  lens_cdata->_projection_mat_inv = LMatrix4::convert_mat(CS_zup_right, cs);

  // We don't apply any leftright offsets for non-linear lenses by default, at
  // least not here in the projection matrix.
  lens_cdata->_projection_mat_left = lens_cdata->_projection_mat_right = lens_cdata->_projection_mat;
  lens_cdata->_projection_mat_left_inv = lens_cdata->_projection_mat_right_inv = lens_cdata->_projection_mat_inv;

  do_adjust_comp_flags(lens_cdata, 0, CF_projection_mat | CF_projection_mat_inv |
                       CF_projection_mat_left_inv | CF_projection_mat_right_inv);
}

/**
 * Computes the matrix that transforms from a point behind the lens to a point
 * on the film.
 */
void Lens::
do_compute_film_mat(CData *cdata) {
  // The lens will return a point in the range [-film_size2, film_size2] in
  // each dimension.  Convert this to [-1, 1], and also apply the offset.

  // We declare these two as local variables, instead of references, to work
  // around a VC7 compiler bug.
  LVecBase2 film_size = do_get_film_size(cdata);
  LVector2 film_offset = do_get_film_offset(cdata);

  PN_stdfloat scale_x = 2.0f / film_size[0];
  PN_stdfloat scale_y = 2.0f / film_size[1];
  cdata->_film_mat.set(scale_x,      0.0f,   0.0f,  0.0f,
                       0.0f,   scale_y,   0.0f,  0.0f,
                       0.0f,      0.0f,   1.0f,  0.0f,
                       -film_offset[0] * scale_x, -film_offset[1] * scale_y, 0.0f,  1.0f);

  if ((cdata->_user_flags & UF_keystone) != 0) {
    cdata->_film_mat = LMatrix4(1.0f, 0.0f, cdata->_keystone[0], cdata->_keystone[0],
                                0.0f, 1.0f, cdata->_keystone[1], cdata->_keystone[1],
                                0.0f, 0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 0.0f, 1.0f) * cdata->_film_mat;
  }

  if ((cdata->_user_flags & UF_custom_film_mat) != 0) {
    cdata->_film_mat = cdata->_film_mat * cdata->_custom_film_mat;
  }

  do_adjust_comp_flags(cdata, CF_film_mat_inv, CF_film_mat);
}

/**
 * Computes the matrix that transforms from a point in front of the lens to a
 * point in space.
 */
void Lens::
do_compute_lens_mat(CData *cdata) {
  if ((cdata->_user_flags & UF_view_mat) == 0) {
    if ((cdata->_user_flags & UF_view_hpr) != 0) {
      compose_matrix(cdata->_lens_mat,
                     LVecBase3(1.0f, 1.0f, 1.0f),
                     LVecBase3(0.0f, 0.0f, 0.0f),
                     cdata->_view_hpr,
                     LVecBase3(0.0f, 0.0f, 0.0f), cdata->_cs);

    } else if ((cdata->_user_flags & UF_view_vector) != 0) {
      look_at(cdata->_lens_mat, cdata->_view_vector, cdata->_up_vector, cdata->_cs);

    } else {
      cdata->_lens_mat = LMatrix4::ident_mat();
    }
  }
  do_adjust_comp_flags(cdata, CF_lens_mat_inv, CF_lens_mat);
}

/**
 * Given a field of view in degrees and a focal length, compute the
 * corresponding width (or height) on the film.  If horiz is true, this is in
 * the horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat Lens::
fov_to_film(PN_stdfloat, PN_stdfloat, bool) const {
  return 1.0f;
}

/**
 * Given a field of view in degrees and a width (or height) on the film,
 * compute the focal length of the lens.  If horiz is true, this is in the
 * horizontal direction; otherwise, it is in the vertical direction (some
 * lenses behave differently in each direction).
 */
PN_stdfloat Lens::
fov_to_focal_length(PN_stdfloat, PN_stdfloat, bool) const {
  return 1.0f;
}

/**
 * Given a width (or height) on the film and a focal length, compute the field
 * of view in degrees.  If horiz is true, this is in the horizontal direction;
 * otherwise, it is in the vertical direction (some lenses behave differently
 * in each direction).
 */
PN_stdfloat Lens::
film_to_fov(PN_stdfloat, PN_stdfloat, bool) const {
  return default_fov;
}

/**
 * Called whenever the user changes one of the three FOV parameters: fov,
 * focal length, or film size.  This rearranges the three sequence numbers so
 * the newest parameter has value 2, and the older parameters are kept in
 * sequence order.
 *
 * This is used to determine which two parameters of the three are the most
 * recently changed, and conversely, which one the user has *not* changed
 * recently.  It is this third value which should be discarded.
 */
void Lens::
do_resequence_fov_triad(const CData *cdata, char &newest, char &older_a, char &older_b) const {
  nassertv(newest + older_a + older_b == 3);
  switch (newest) {
  case 0:
    newest = 2;
    older_a--;
    older_b--;
    nassertv(older_a + older_b == 1);
    break;

  case 1:
    newest = 2;
    if (older_a == 2) {
      nassertv(older_b == 0);
      older_a = 1;
    } else {
      nassertv(older_a == 0 && older_b == 2);
      older_b = 1;
    }
    break;

  case 2:
    nassertv(older_a + older_b == 1);
    break;

  default:
    gobj_cat.error()
      << "Invalid fov sequence numbers in lens: "
      << (int)newest << ", " << (int)older_a << ", " << (int)older_b << "\n";
    nassertv(false);
    return;
  }

  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "Lens.do_resequence_fov_triad():";
    for (int i = 2; i >= 0; --i) {
      if (cdata->_fov_seq == i) {
        gobj_cat.debug(false)
          << " fov";
      } else if (cdata->_focal_length_seq == i) {
        gobj_cat.debug(false)
          << " focal_length";
      } else if (cdata->_film_size_seq == i) {
        gobj_cat.debug(false)
          << " film_size";
      }
    }
    gobj_cat.debug(false)
      << "\n";
  }
}

/**
 * Adjusts (or defines for the first time) all the vertices in the _geom_data
 * to match the properties of the lens.  This will update the visual
 * representation of the lens's frustum to match the changing parameters.
 * Returns the number of line segments per edge.
 */
int Lens::
do_define_geom_data(CData *cdata) {
  int num_segments = 1;
  if (!is_linear()) {
    num_segments = lens_geom_segments;
  }

  if (cdata->_geom_data == nullptr) {
    cdata->_geom_data = new GeomVertexData
      ("lens", GeomVertexFormat::get_v3(),
       Geom::UH_dynamic);
  }
  cdata->_geom_data->unclean_set_num_rows(num_segments * 8 + 2);

  GeomVertexWriter vertex(cdata->_geom_data, InternalName::get_vertex());
  LPoint3 near_point, far_point;
  for (int si = 0; si < num_segments; si++) {
    PN_stdfloat t = 2.0f * (PN_stdfloat)si / (PN_stdfloat)num_segments;

    // Upper left, top edge.
    LPoint3 p1(-1.0f + t, 1.0f, 0.0f);
    if (!do_extrude(cdata, p1, near_point, far_point)) {
      // Hey, this point is off the lens!  Can't do a frustum.
      return 0;
    }
    vertex.set_data3(near_point);
    vertex.set_data3(far_point);

    // Upper right, right edge.
    LPoint3 p2(1.0f, 1.0f - t, 0.0f);
    if (!do_extrude(cdata, p2, near_point, far_point)) {
      // Hey, this point is off the lens!  Can't do a frustum.
      return 0;
    }
    vertex.set_data3(near_point);
    vertex.set_data3(far_point);

    // Lower right, bottom edge.
    LPoint3 p3(1.0f - t, -1.0f, 0.0f);
    if (!do_extrude(cdata, p3, near_point, far_point)) {
      // Hey, this point is off the lens!  Can't do a frustum.
      return 0;
    }
    vertex.set_data3(near_point);
    vertex.set_data3(far_point);

    // Lower left, left edge.
    LPoint3 p4(-1.0f, -1.0f + t, 0.0f);
    if (!do_extrude(cdata, p4, near_point, far_point)) {
      // Hey, this point is off the lens!  Can't do a frustum.
      return 0;
    }
    vertex.set_data3(near_point);
    vertex.set_data3(far_point);
  }


  // Finally, add one more pair for the viewing axis (or more specifically,
  // the center of the lens).
  LPoint3 pc(0);
  if (!do_extrude(cdata, pc, near_point, far_point)) {
    vertex.set_data3(0.0f, 0.0f, 0.0f);
    vertex.set_data3(0.0f, 0.0f, 0.0f);
  } else {
    vertex.set_data3(near_point);
    vertex.set_data3(far_point);
  }

  return num_segments;
}

/**
 * A support function for set_frustum_from_corners(), this computes a matrix
 * that will shear the four indicated points to the most nearly rectangular.
 */
void Lens::
build_shear_mat(LMatrix4 &shear_mat,
                const LPoint3 &cul, const LPoint3 &cur,
                const LPoint3 &cll, const LPoint3 &clr) {
  // Fit a parallelogram around these four points.

  // Put the points in an array so we can rotate it around to find the longest
  // edge.
  LPoint3 points[4] = {
    cul, cur, clr, cll
  };

  PN_stdfloat max_edge_length = -1.0f;
  int base_edge = -1;
  for (int i = 0; i < 4; i++) {
    LVector3 edge = points[(i + 1) % 4] - points[i];
    PN_stdfloat length = edge.length_squared();
    if (length > max_edge_length) {
      base_edge = i;
      max_edge_length = length;
    }
  }

  const LPoint3 &base_origin = points[base_edge];
  LVector3 base_vec = points[(base_edge + 1) % 4] - base_origin;

  PN_stdfloat base_edge_length = csqrt(max_edge_length);

  // The longest edge is the base of our parallelogram.  The parallel edge
  // must pass through the point furthest from this edge.

  int a = (base_edge + 2) % 4;
  int b = (base_edge + 3) % 4;

  PN_stdfloat a_dist = sqr_dist_to_line(points[a], base_origin, base_vec);
  PN_stdfloat b_dist = sqr_dist_to_line(points[b], base_origin, base_vec);

  int far_point;
  PN_stdfloat dist;
  if (a_dist > b_dist) {
    far_point = a;
    dist = csqrt(a_dist);
  } else {
    far_point = b;
    dist = csqrt(b_dist);
  }

  // Try to make the parallelogram as nearly rectangular as possible.  How
  // suitable is a true rectangle?
  LVector3 perpendic = base_vec.cross(LVector3(0.0f, -1.0f, 0.0f));
  perpendic.normalize();
  perpendic *= dist;
  LPoint3 parallel_origin = points[base_edge] + perpendic;

  // It follows that far_point is on the line passing through the parallel
  // edge.  Is it within the endpoints?
  LVector3 base_norm_vec = base_vec / base_edge_length;

  LVector3 far_point_delta = points[far_point] - parallel_origin;
  PN_stdfloat far_point_pos = far_point_delta.dot(base_norm_vec);

  if (far_point_pos < 0.0f) {
    // We have to slide the parallel_origin back to include far_point.
    parallel_origin += base_norm_vec * far_point_pos;

  } else if (far_point_pos > base_edge_length) {
    // We have to slide the parallel_origin forward to include far_point.
    parallel_origin += base_norm_vec * (far_point_pos - base_edge_length);
  }

  // Finally, is the other point within the parallelogram?
  PN_stdfloat t;
  PN_stdfloat Ox = parallel_origin[0];
  PN_stdfloat Oy = parallel_origin[2];
  PN_stdfloat Vx = base_vec[0];
  PN_stdfloat Vy = base_vec[2];
  PN_stdfloat Ax, Ay, Bx, By;

  if (far_point == a) {
    // near point is b
    LVector3 v = points[b] - base_origin;
    Ax = points[b][0];
    Ay = points[b][2];
    Bx = v[0];
    By = v[2];
  } else {
    // near point is a
    LVector3 v = points[a] - (base_origin + base_vec);
    Ax = points[a][0];
    Ay = points[a][2];
    Bx = v[0];
    By = v[2];
  }
  t = ((Ox - Ax) * By + (Ay - Oy) * Bx) / (Bx * Vy - By * Vx);

  if (t < 0.0f) {
    // We need to slide the parallel_origin back to include the near point.
    parallel_origin += base_vec * t;
  } else if (t > 1.0f) {
    // We need to slide the parallel_origin forward to include the far point.
    parallel_origin += base_vec * (1.0f - t);
  }

  LVector3 adjacent_norm_vec = parallel_origin - base_origin;
  adjacent_norm_vec.normalize();

  // Now we've defined a parallelogram that includes all four points, and
  // we're ready to build a shear transform.
  shear_mat = LMatrix4::ident_mat();

  // The edges of the parallelogram become the axes.
  switch (base_edge) {
  case 0:
    // The base_origin is the upper-left corner.  X axis is base_norm_vec, Z
    // axis is -adjacent_norm_vec.
    shear_mat.set_row(0, base_norm_vec);
    shear_mat.set_row(2, -adjacent_norm_vec);
    break;

  case 1:
    // The base_origin is the upper-right corner.  X axis is
    // -adjacent_norm_vec, Z axis is -base_norm_vec.
    shear_mat.set_row(0, -adjacent_norm_vec);
    shear_mat.set_row(2, -base_norm_vec);
    break;

  case 2:
    // The base_origin is the lower-right corner.  X axis is -base_norm_vec, Z
    // axis is adjacent_norm_vec.
    shear_mat.set_row(0, -base_norm_vec);
    shear_mat.set_row(2, adjacent_norm_vec);
    break;

  case 3:
    // The base_origin is the lower-left corner.  X axis is adjacent_norm_vec,
    // Z axis is base_norm_vec.
    shear_mat.set_row(0, adjacent_norm_vec);
    shear_mat.set_row(2, base_norm_vec);
    break;

  default:
    nassertv(false);
  }
}

/**
 * A support function for build_shear_mat(), this computes the minimum
 * distance from a point to a line, and returns the distance squared.
 */
PN_stdfloat Lens::
sqr_dist_to_line(const LPoint3 &point, const LPoint3 &origin,
                 const LVector3 &vec) {
  LVector3 norm = vec;
  norm.normalize();
  LVector3 d = point - origin;
  PN_stdfloat hyp_2 = d.length_squared();
  PN_stdfloat leg = d.dot(norm);
  return hyp_2 - leg * leg;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Lens::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Lens.
 */
void Lens::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

/**
 *
 */
Lens::CData::
CData() {
  clear();
}

/**
 *
 */
Lens::CData::
CData(const Lens::CData &copy) {
  _change_event = copy._change_event;
  _cs = copy._cs;
  _film_size = copy._film_size;
  _film_offset = copy._film_offset;
  _focal_length = copy._focal_length;
  _fov = copy._fov;
  _min_fov = copy._min_fov;
  _aspect_ratio = copy._aspect_ratio;
  _near_distance = copy._near_distance;
  _far_distance = copy._far_distance;

  _view_hpr = copy._view_hpr;
  _view_vector = copy._view_vector;
  _interocular_distance = copy._interocular_distance;
  _convergence_distance = copy._convergence_distance;
  _keystone = copy._keystone;

  // This matrix might have been explicitly set by the user (if UF_view_mat is
  // applied), so we must preserve it.  Other matrices are implicitly
  // computed.
  _lens_mat = copy._lens_mat;

  _user_flags = copy._user_flags;
  _comp_flags = 0;

  _focal_length_seq = copy._focal_length_seq;
  _fov_seq = copy._fov_seq;
  _film_size_seq = copy._film_size_seq;

  _geom_data = copy._geom_data;
}

/**
 *
 */
CycleData *Lens::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Lens::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_string(_change_event);
  dg.add_uint8((int)_cs);
  _film_size.write_datagram(dg);
  _film_offset.write_datagram(dg);
  dg.add_stdfloat(_focal_length);
  _fov.write_datagram(dg);
  dg.add_stdfloat(_aspect_ratio);
  dg.add_stdfloat(_near_distance);
  dg.add_stdfloat(_far_distance);
  dg.add_uint16(_user_flags);

  if (manager->get_file_minor_ver() < 41) {
    return;
  }

  dg.add_stdfloat(_min_fov);
  dg.add_stdfloat(_interocular_distance);
  dg.add_stdfloat(_convergence_distance);

  if (_user_flags & UF_view_hpr) {
    _view_hpr.write_datagram(dg);
  }

  if (_user_flags & UF_view_vector) {
    _view_vector.write_datagram(dg);
    _up_vector.write_datagram(dg);
  }

  if (_user_flags & UF_view_mat) {
    _lens_mat.write_datagram(dg);
  }

  if (_user_flags & UF_keystone) {
    _keystone.write_datagram(dg);
  }

  if (_user_flags & UF_custom_film_mat) {
    _custom_film_mat.write_datagram(dg);
  }
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Geom.
 */
void Lens::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _change_event = scan.get_string();
  _cs = (CoordinateSystem)scan.get_uint8();
  _film_size.read_datagram(scan);
  _film_offset.read_datagram(scan);
  _focal_length = scan.get_stdfloat();
  _fov.read_datagram(scan);
  _aspect_ratio = scan.get_stdfloat();
  _near_distance = scan.get_stdfloat();
  _far_distance = scan.get_stdfloat();
  _user_flags = scan.get_uint16();

  if (manager->get_file_minor_ver() >= 41) {
    _min_fov = scan.get_stdfloat();
    _interocular_distance = scan.get_stdfloat();
    _convergence_distance = scan.get_stdfloat();

    if (_user_flags & UF_view_hpr) {
      _view_hpr.read_datagram(scan);
    }

    if (_user_flags & UF_view_vector) {
      _view_vector.read_datagram(scan);
      _up_vector.read_datagram(scan);
    }

    if (_user_flags & UF_view_mat) {
      _lens_mat.read_datagram(scan);
    }

    if (_user_flags & UF_keystone) {
      _keystone.read_datagram(scan);
    }

    if (_user_flags & UF_custom_film_mat) {
      _custom_film_mat.read_datagram(scan);
    }
  }

  _comp_flags = 0;
}

/**
 *
 */
void Lens::CData::
clear() {
  _change_event = "";
  _cs = CS_default;
  _film_size.set(1.0f, 1.0f);
  _film_offset.set(0.0f, 0.0f);
  _focal_length = 1.0f;
  _fov.set(default_fov, default_fov);
  _aspect_ratio = 1.0f;
  _near_distance = default_near;
  _far_distance = default_far;
  _view_hpr.set(0.0f, 0.0f, 0.0f);
  _view_vector.set(0.0f, 1.0f, 0.0f);
  _up_vector.set(0.0f, 0.0f, 1.0f);
  _keystone.set(0.0f, 0.0f);
  _custom_film_mat = LMatrix4::ident_mat();

  _user_flags = 0;
  _comp_flags = CF_fov;

  _interocular_distance = 0.0;
  _convergence_distance = 0.0;

  if (default_keystone.has_value()) {
    _keystone.set(default_keystone[0], default_keystone[1]);
    _user_flags |= UF_keystone;
  }

  // Assign an initial arbitrary sequence to these three.
  _film_size_seq = 0;
  _focal_length_seq = 1;
  _fov_seq = 2;
}
