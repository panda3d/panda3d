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
  return _num_channels != 0 && (_x_size * _y_size == (int)_table.size());
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
INLINE PN_stdfloat PfmFile::
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
  if ((x >= 0 && x < _x_size) && 
      (y >= 0 && y < _y_size)) {
    return (!_has_no_data_value || _table[y * _x_size + x] != _no_data_value);
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_point
//       Access: Published
//  Description: Returns the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE const LPoint3 &PfmFile::
get_point(int x, int y) const {
  nassertr(x >= 0 && x < _x_size, LPoint3::zero());
  nassertr(y >= 0 && y < _y_size, LPoint3::zero());
  return _table[y * _x_size + x];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_point
//       Access: Published
//  Description: Replaces the 3-component point value at the indicated
//               point.  In a 1-channel image, the channel value is in
//               the x component.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_point(int x, int y, const LVecBase3 &point) {
  nassertv(!point.is_nan());
  nassertv(x >= 0 && x < _x_size);
  nassertv(y >= 0 && y < _y_size);
  _table[y * _x_size + x] = point;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::modify_point
//       Access: Published
//  Description: Returns a modifiable 3-component point value at the
//               indicated point.
////////////////////////////////////////////////////////////////////
INLINE LPoint3 &PfmFile::
modify_point(int x, int y) {
#ifndef NDEBUG
  static LPoint3 dummy_value = LPoint3::zero();
  nassertr(x >= 0 && x < _x_size, dummy_value);
  nassertr(y >= 0 && y < _y_size, dummy_value);
#endif

  return _table[y * _x_size + x];
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
calc_autocrop(LVecBase4 &range) const {
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
    set_no_data_value(LPoint3::zero());
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
set_no_data_value(const LPoint3 &no_data_value) {
  _has_no_data_value = true;
  _no_data_value = no_data_value;
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
  _no_data_value = LPoint3::zero();
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
INLINE const LPoint3 &PfmFile::
get_no_data_value() const {
  nassertr(_has_no_data_value, LPoint3::zero());
  return _no_data_value;
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
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_vis_inverse(bool vis_inverse) {
  _vis_inverse = vis_inverse;
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
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_flat_texcoord_name(InternalName *flat_texcoord_name) {
  _flat_texcoord_name = flat_texcoord_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear_flat_texcoord_name
//       Access: Published
//  Description: Resets the flat_texcoord_name to empty, so that
//               additional texture coordinates are not created.
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
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_vis_2d(bool vis_2d) {
  _vis_2d = vis_2d;
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
