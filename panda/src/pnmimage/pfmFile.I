// Filename: pfmFile.I
// Created by:  drose (23Dec10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PfmFile::is_valid
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
is_valid() const {
  return _num_channels != 0 && (_x_size * _y_size * _num_channels <= (int)_table.size());
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_x_size
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PfmFile::
get_x_size() const {
  return _x_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_y_size
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PfmFile::
get_y_size() const {
  return _y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_scale
//       Access: Published
//  Description: The "scale" is reported in the pfm header and is
//               probably meaningless.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 PfmFile::
get_scale() const {
  return _scale;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_num_channels
//       Access: Published
//  Description: A pfm file can be either 1-channel
//               (get_num_channels() == 1) or 3-channel
//               (get_num_channels() == 3).  In the case of a
//               1-channel file, the values can be found in the x
//               component of each point, and the y and z components
//               are unused.
////////////////////////////////////////////////////////////////////
INLINE int PfmFile::
get_num_channels() const {
  return _num_channels;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point
//       Access: Published
//  Description: Returns true if there is a valid point at x, y.  This
//               always returns true unless a "no data" value has been
//               set, in which case it returns false if the point at
//               x, y is the "no data" value.
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
has_point(int x, int y) const {
  return _has_point(this, x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point1
//       Access: Published
//  Description: Returns the 1-component point value at the indicated
//               point.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 PfmFile::
get_point1(int x, int y) const {
  nassertr(x >= 0 && x < _x_size, 0.0);
  nassertr(y >= 0 && y < _y_size, 0.0);
  return _table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point1
//       Access: Published
//  Description: Replaces the 1-component point value at the indicated
//               point.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point1(int x, int y, PN_float32 point) {
  nassertv(!cnan(point));
  nassertv(x >= 0 && x < _x_size);
  nassertv(y >= 0 && y < _y_size);
  _table[(y * _x_size + x) * _num_channels] = point;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point
//       Access: Published
//  Description: Returns the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &PfmFile::
get_point(int x, int y) const {
  nassertr(x >= 0 && x < _x_size, LPoint3f::zero());
  nassertr(y >= 0 && y < _y_size, LPoint3f::zero());
  return *(LPoint3f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point
//       Access: Published
//  Description: Replaces the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point(int x, int y, const LVecBase3f &point) {
  nassertv(!point.is_nan());
  nassertv(x >= 0 && x < _x_size);
  nassertv(y >= 0 && y < _y_size);
  switch (_num_channels) {
  case 1:
    _table[(y * _x_size + x)] = point[0];
    break;

  case 3:
  case 4:
    *(LPoint3f *)&_table[(y * _x_size + x) * _num_channels] = point;
    break;
  } 
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point
//       Access: Published
//  Description: Replaces the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point(int x, int y, const LVecBase3d &point) {
  set_point(x, y, LCAST(PN_float32, point));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point
//       Access: Published
//  Description: Returns a modifiable 3-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f &PfmFile::
modify_point(int x, int y) {
#ifndef NDEBUG
  static LPoint3f dummy_value = LPoint3f::zero();
  nassertr(x >= 0 && x < _x_size, dummy_value);
  nassertr(y >= 0 && y < _y_size, dummy_value);
#endif

  return *(LPoint3f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point4
//       Access: Published
//  Description: Returns the 4-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE const LPoint4f &PfmFile::
get_point4(int x, int y) const {
  nassertr(x >= 0 && x < _x_size, LPoint4f::zero());
  nassertr(y >= 0 && y < _y_size, LPoint4f::zero());
  return *(LPoint4f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point4
//       Access: Published
//  Description: Replaces the 4-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point4(int x, int y, const LVecBase4f &point) {
  nassertv(!point.is_nan());
  nassertv(x >= 0 && x < _x_size);
  nassertv(y >= 0 && y < _y_size);
  switch (_num_channels) {
  case 1:
    _table[(y * _x_size + x)] = point[0];
    break;

  case 3:
    (*(LPoint3f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1], point[2]);
    break;

  case 4:
    *(LPoint4f *)&_table[(y * _x_size + x) * _num_channels] = point;
    break;
  } 
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point4
//       Access: Published
//  Description: Replaces the 4-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point4(int x, int y, const LVecBase4d &point) {
  set_point4(x, y, LCAST(PN_float32, point));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point4
//       Access: Published
//  Description: Returns a modifiable 4-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint4f &PfmFile::
modify_point4(int x, int y) {
#ifndef NDEBUG
  static LPoint4f dummy_value = LPoint4f::zero();
  nassertr(x >= 0 && x < _x_size, dummy_value);
  nassertr(y >= 0 && y < _y_size, dummy_value);
#endif

  return *(LPoint4f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::calc_autocrop
//       Access: Published
//  Description: Computes the minimum range of x and y across the PFM
//               file that include all points.  If there are no points
//               with no_data_value in the grid--that is, all points
//               are included--then this will return (0, get_x_size(),
//               0, get_y_size()).
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
calc_autocrop(LVecBase4f &range) const {
  int x_begin, x_end, y_begin, y_end;
  bool result = calc_autocrop(x_begin, x_end, y_begin, y_end);
  range.set(x_begin, x_end, y_begin, y_end);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::calc_autocrop
//       Access: Published
//  Description: Computes the minimum range of x and y across the PFM
//               file that include all points.  If there are no points
//               with no_data_value in the grid--that is, all points
//               are included--then this will return (0, get_x_size(),
//               0, get_y_size()).
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
calc_autocrop(LVecBase4d &range) const {
  int x_begin, x_end, y_begin, y_end;
  bool result = calc_autocrop(x_begin, x_end, y_begin, y_end);
  range.set(x_begin, x_end, y_begin, y_end);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_zero_special
//       Access: Published
//  Description: Sets the zero_special flag.  When this flag is true,
//               values of (0, 0, 0) in the pfm file are treated as a
//               special case, and are not processed.
//
//               This is a special case of set_no_data_value().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_zero_special(bool zero_special) {
  if (zero_special) {
    set_no_data_value(LPoint4f::zero());
  } else {
    clear_no_data_value();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_no_data_chan4
//       Access: Published
//  Description: Sets the no_data_chan4 flag.  When this flag is true,
//               and the pfm file has 4 channels, then a negative
//               value in the fourth channel indicates no data.  When
//               it is false, a zero or positive value in the fourth
//               channel indicates valid data.
//
//               This is a special case of set_no_data_value().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_no_data_chan4(bool chan4) {
  if (chan4 && _num_channels == 4) {
    _has_no_data_value = true;
    _no_data_value = LPoint4f::zero();
    _has_point = has_point_chan4;
  } else {
    clear_no_data_value();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_no_data_value
//       Access: Published
//  Description: Sets the special value that means "no data" when it
//               appears in the pfm file.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_no_data_value(const LPoint4d &no_data_value) {
  set_no_data_value(LCAST(PN_float32, no_data_value));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_no_data_value
//       Access: Published
//  Description: Removes the special value that means "no data" when it
//               appears in the pfm file.  All points will thus be
//               considered valid.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
clear_no_data_value() {
  _has_no_data_value = false;
  _no_data_value = LPoint4f::zero();
  _has_point = has_point_noop;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_no_data_value
//       Access: Published
//  Description: Returns whether a "no data" value has been
//               established by set_no_data_value().
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
has_no_data_value() const {
  return _has_no_data_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_no_data_value
//       Access: Published
//  Description: If has_no_data_value() returns true, this returns the
//               particular "no data" value.
////////////////////////////////////////////////////////////////////
INLINE const LPoint4f &PfmFile::
get_no_data_value() const {
  nassertr(_has_no_data_value, LPoint4f::zero());
  return _no_data_value;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::xform
//       Access: Published
//  Description: Applies the indicated transform matrix to all points
//               in-place.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
xform(const TransformState *transform) {
  if (!transform->is_identity()) {
    xform(transform->get_mat());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::xform
//       Access: Published
//  Description: Applies the indicated transform matrix to all points
//               in-place.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
xform(const LMatrix4d &transform) {
  xform(LCAST(PN_float32, transform));
}
////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_planar_bounds
//       Access: Published
//  Description: Computes the minmax bounding volume of the points in
//               3-D space, assuming the points represent a
//               mostly-planar surface.
//
//               This algorithm works by sampling the (square)
//               sample_radius pixels at the four point_dist corners
//               around the center (cx - pd, cx + pd) and so on, to
//               approximate the plane of the surface.  Then all of
//               the points are projected into that plane and the
//               bounding volume of the entire mesh within that plane
//               is determined.  If points_only is true, the bounding
//               volume of only those four points is determined.
//
//               center, point_dist and sample_radius are in UV space,
//               i.e. in the range 0..1.
////////////////////////////////////////////////////////////////////
INLINE PT(BoundingHexahedron) PfmFile::
compute_planar_bounds(const LPoint2d &center, PN_float32 point_dist, PN_float32 sample_radius, bool points_only) const {
  return compute_planar_bounds(LCAST(PN_float32, center), point_dist, sample_radius, points_only);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_vis_inverse
//       Access: Published
//  Description: Sets the vis_inverse flag.  When this flag is true,
//               vis meshes and point clouds are generated with the
//               3-d depth value in the texture coordinates, and the
//               2-d index value in the vertex position.  When it is
//               false, meshes are generated normally, with the 3-d
//               depth value in the vertex position and the 2-d index
//               value in the texture coordinates.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_vis_inverse(bool vis_inverse) {
  _vis_inverse = vis_inverse;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_vis_inverse
//       Access: Published
//  Description: Returns the vis_inverse flag.  See set_vis_inverse().
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
get_vis_inverse() const {
  return _vis_inverse;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_flat_texcoord_name
//       Access: Published
//  Description: If the flat_texcoord_name is specified, it is the
//               name of an additional vertex column that will be
//               created for the "flat" texture coordinates, i.e. the
//               original 0..1 values that correspond to the 2-D index
//               position of each point in the original pfm file.
//
//               These are the same values that will be assigned to
//               the default texture coordinates if the vis_inverse
//               flag is *not* true.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_flat_texcoord_name(InternalName *flat_texcoord_name) {
  _flat_texcoord_name = flat_texcoord_name;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear_flat_texcoord_name
//       Access: Published
//  Description: Resets the flat_texcoord_name to empty, so that
//               additional texture coordinates are not created.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
clear_flat_texcoord_name() {
  _flat_texcoord_name = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_flat_texcoord_name
//       Access: Published
//  Description: Returns the flat_texcoord_name.  See set_flat_texcoord_name().
////////////////////////////////////////////////////////////////////
INLINE InternalName *PfmFile::
get_flat_texcoord_name() const {
  return _flat_texcoord_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_vis_2d
//       Access: Published
//  Description: Sets the vis_2d flag.  When this flag is true,
//               only the first two (x, y) value of each depth point
//               is considered meaningful; the z component is ignored.
//               This is only relevant for generating visualizations.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_vis_2d(bool vis_2d) {
  _vis_2d = vis_2d;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_vis_2d
//       Access: Published
//  Description: Returns the vis_2d flag.  See set_vis_2d().
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
get_vis_2d() const {
  return _vis_2d;
}
