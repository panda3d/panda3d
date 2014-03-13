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
//     Function: PfmFile::set_scale
//       Access: Published
//  Description: The "scale" is reported in the pfm header and is
//               probably meaningless.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_scale(PN_float32 scale) {
  _scale = scale;
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
//     Function: PfmFile::get_channel
//       Access: Published
//  Description: Returns the cth channel of the point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 PfmFile::
get_channel(int x, int y, int c) const {
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size &&
           c >= 0 && c < _num_channels, 0.0f);
  return _table[(y * _x_size + x) * _num_channels + c];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_channel
//       Access: Published
//  Description: Replaces the cth channel of the point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_channel(int x, int y, int c, PN_float32 value) {
  nassertv(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size &&
           c >= 0 && c < _num_channels);
  _table[(y * _x_size + x) * _num_channels + c] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point1
//       Access: Published
//  Description: Returns the 1-component point value at the indicated
//               point.
////////////////////////////////////////////////////////////////////
INLINE PN_float32 PfmFile::
get_point1(int x, int y) const {
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, 0.0);
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
  nassertv(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size);
  _table[(y * _x_size + x) * _num_channels] = point;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point2
//       Access: Published
//  Description: Returns the 2-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE const LPoint2f &PfmFile::
get_point2(int x, int y) const {
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, LPoint2f::zero());
  return *(LPoint2f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point2
//       Access: Published
//  Description: Replaces the 2-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point2(int x, int y, const LVecBase2f &point) {
  nassertv(!point.is_nan());
  nassertv(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size);
  switch (_num_channels) {
  case 1:
    _table[(y * _x_size + x)] = point[0];
    break;

  case 2:
    *(LPoint2f *)&_table[(y * _x_size + x) * _num_channels] = point;
    break;

  case 3:
    (*(LPoint3f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1], 0.0);
    break;

  case 4:
    (*(LPoint4f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1], 0.0, 0.0);
    break;
  } 
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point2
//       Access: Published
//  Description: Replaces the 2-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point2(int x, int y, const LVecBase2d &point) {
  set_point2(x, y, LCAST(PN_float32, point));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point2
//       Access: Published
//  Description: Returns a modifiable 2-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint2f &PfmFile::
modify_point2(int x, int y) {
#ifndef NDEBUG
  static LPoint2f dummy_value = LPoint2f::zero();
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, dummy_value);
#endif

  return *(LPoint2f *)&_table[(y * _x_size + x) * _num_channels];
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
  return get_point3(x, y);
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
  set_point3(x, y, point);
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
  set_point3(x, y, point);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point
//       Access: Published
//  Description: Returns a modifiable 3-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f &PfmFile::
modify_point(int x, int y) {
  return modify_point3(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point3
//       Access: Published
//  Description: Returns the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3f &PfmFile::
get_point3(int x, int y) const {
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, LPoint3f::zero());
  return *(LPoint3f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point3
//       Access: Published
//  Description: Replaces the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point3(int x, int y, const LVecBase3f &point) {
  nassertv(!point.is_nan());
  nassertv(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size);
  switch (_num_channels) {
  case 1:
    _table[(y * _x_size + x)] = point[0];
    break;

  case 2:
    (*(LPoint2f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1]);
    break;

  case 3:
    *(LPoint3f *)&_table[(y * _x_size + x) * _num_channels] = point;
    break;

  case 4:
    (*(LPoint4f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1], 0.0f, 0.0f);
    break;
  } 
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point3
//       Access: Published
//  Description: Replaces the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point3(int x, int y, const LVecBase3d &point) {
  set_point3(x, y, LCAST(PN_float32, point));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point3
//       Access: Published
//  Description: Returns a modifiable 3-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint3f &PfmFile::
modify_point3(int x, int y) {
#ifndef NDEBUG
  static LPoint3f dummy_value = LPoint3f::zero();
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, dummy_value);
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
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, LPoint4f::zero());
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
  nassertv(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size);
  switch (_num_channels) {
  case 1:
    _table[(y * _x_size + x)] = point[0];
    break;

  case 2:
    (*(LPoint2f *)&_table[(y * _x_size + x) * _num_channels]).set(point[0], point[1]);
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
  nassertr(x >= 0 && x < _x_size &&
           y >= 0 && y < _y_size, dummy_value);
#endif

  return *(LPoint4f *)&_table[(y * _x_size + x) * _num_channels];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::fill
//       Access: Published
//  Description: Fills the table with all of the same value.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
fill(PN_float32 value) {
  fill(LPoint4f(value, 0.0f, 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::fill
//       Access: Published
//  Description: Fills the table with all of the same value.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
fill(const LPoint2f &value) {
  fill(LPoint4f(value[0], value[1], 0.0f, 0.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::fill
//       Access: Published
//  Description: Fills the table with all of the same value.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
fill(const LPoint3f &value) {
  fill(LPoint4f(value[0], value[1], value[2], 0.0f));
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
//               it is false, all points are valid.
//
//               This is a special case of set_no_data_value().
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_no_data_chan4(bool chan4) {
  if (chan4 && _num_channels == 4) {
    _has_no_data_value = true;
    _has_no_data_threshold = false;
    _no_data_value.set(0.0, 0.0, 0.0, -1.0);
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
//     Function: PfmFile::set_no_data_threshold
//       Access: Published
//  Description: Sets the special threshold value.  Points that are
//               below this value in all components are considered "no
//               value".
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_no_data_threshold(const LPoint4d &no_data_threshold) {
  set_no_data_threshold(LCAST(PN_float32, no_data_threshold));
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
  _has_no_data_threshold = false;
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
//     Function: PfmFile::has_no_data_threshold
//       Access: Published
//  Description: Returns whether a "no data" threshold value has been
//               established by set_no_data_threshold().
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
has_no_data_threshold() const {
  return _has_no_data_threshold;
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
//     Function: PfmFile::get_table
//       Access: Public
//  Description: This is a very low-level function that returns a
//               read-only reference to the internal table of
//               floating-point numbers.  Use this method at your own
//               risk.
////////////////////////////////////////////////////////////////////
INLINE const vector_float &PfmFile::
get_table() const {
  return _table;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::swap_table
//       Access: Public
//  Description: This is a very low-level function that completely
//               exchanges the PfmFile's internal table of
//               floating-point numbers with whatever you supply.  The
//               provided table must have an appropriate size.  Use
//               this method at your own risk.
////////////////////////////////////////////////////////////////////
void PfmFile::
swap_table(vector_float &table) {
  _table.swap(table);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::setup_sub_image
//       Access: Private
//  Description: Computes xmin, ymin, xmax, and ymax, based on the
//               input parameters for copy_sub_image() and related
//               methods.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
setup_sub_image(const PfmFile &copy, int &xto, int &yto,
                int &xfrom, int &yfrom, int &x_size, int &y_size,
                int &xmin, int &ymin, int &xmax, int &ymax) {
  if (x_size < 0) {
    x_size = copy.get_x_size() - xfrom;
  }
  if (y_size < 0) {
    y_size = copy.get_y_size() - yfrom;
  }

  if (xfrom < 0) {
    xto += -xfrom;
    x_size -= -xfrom;
    xfrom = 0;
  }
  if (yfrom < 0) {
    yto += -yfrom;
    y_size -= -yfrom;
    yfrom = 0;
  }

  if (xto < 0) {
    xfrom += -xto;
    x_size -= -xto;
    xto = 0;
  }
  if (yto < 0) {
    yfrom += -yto;
    y_size -= -yto;
    yto = 0;
  }

  x_size = min(x_size, copy.get_x_size() - xfrom);
  y_size = min(y_size, copy.get_y_size() - yfrom);

  xmin = xto;
  ymin = yto;

  xmax = min(xmin + x_size, get_x_size());
  ymax = min(ymin + y_size, get_y_size());
}
