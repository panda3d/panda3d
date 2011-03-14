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
INLINE float PfmFile::
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
//               always returns true unless the zero_special flag is
//               enabled, in which case it returns false if the point
//               at x, y is zero.
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
has_point(int x, int y) const {
  if ((x >= 0 && x < _x_size) && 
      (y >= 0 && y < _y_size)) {
    return (!_zero_special || _table[y * _x_size + x] != LPoint3f::zero());
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
INLINE const LPoint3f &PfmFile::
get_point(int x, int y) const {
  nassertr(x >= 0 && x < _x_size, LPoint3f::zero());
  nassertr(y >= 0 && y < _y_size, LPoint3f::zero());
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
set_point(int x, int y, const LVecBase3f &point) {
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
INLINE LPoint3f &PfmFile::
modify_point(int x, int y) {
#ifndef NDEBUG
  static LPoint3f dummy_value = LPoint3f::zero();
  nassertr(x >= 0 && x < _x_size, dummy_value);
  nassertr(y >= 0 && y < _y_size, dummy_value);
#endif

  return _table[y * _x_size + x];
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_zero_special
//       Access: Published
//  Description: Sets the zero_special flag.  When this flag is true,
//               values of (0, 0, 0) in the pfm file are treated as a
//               special case, and are not processed.
////////////////////////////////////////////////////////////////////
INLINE void PfmFile::
set_zero_special(bool zero_special) {
  _zero_special = zero_special;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::get_zero_special
//       Access: Published
//  Description: Returns the zero_special flag.  When this flag is true,
//               values of (0, 0, 0) in the pfm file are treated as a
//               special case, and are not processed.
////////////////////////////////////////////////////////////////////
INLINE bool PfmFile::
get_zero_special() const {
  return _zero_special;
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
