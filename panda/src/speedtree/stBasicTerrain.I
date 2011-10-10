// Filename: stBasicTerrain.I
// Created by:  drose (12Oct10)
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
//     Function: STBasicTerrain::set_height_map
//       Access: Published
//  Description: Specifies the image filename that will define the
//               height map of the terrain.  This will require a
//               subsequent call to load_data() to actually read the
//               data.
////////////////////////////////////////////////////////////////////
INLINE void STBasicTerrain::
set_height_map(const Filename &height_map) {
  _height_map = height_map;
  _is_valid = false;
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::get_height_map
//       Access: Published
//  Description: Returns the image filename that defines the
//               height map of the terrain.
////////////////////////////////////////////////////////////////////
INLINE const Filename &STBasicTerrain::
get_height_map() const {
  return _height_map;
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::get_size
//       Access: Published
//  Description: Returns the length, in scene graph units, of one edge
//               of the heightmap as it is manifested by the terrain.
//               Increasing this number spreads the heightmap out over
//               a greater area.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat STBasicTerrain::
get_size() const {
  return _size;
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::interpolate
//       Access: Protected, Static
//  Description: Convenience function to calculate the linear
//               interpolation from A to B.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat STBasicTerrain::
interpolate(PN_stdfloat a, PN_stdfloat b, PN_stdfloat t) {
  return (a + (b - a) * t);
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class ValueType>
STBasicTerrain::InterpolationData<ValueType>::
InterpolationData() : _width(0), _height(0) 
{
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::reset
//       Access: Public
//  Description: Resets the array to an empty array of width x height
//               cells.
////////////////////////////////////////////////////////////////////
template<class ValueType>
void STBasicTerrain::InterpolationData<ValueType>::
reset(int width, int height) {
  _width = width;
  _height = height;
  _data.clear();
  _data.insert(_data.begin(), width * height, ValueType());
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::get_nearest_neighbor
//       Access: Public
//  Description: Returns the value nearest to (u, v) in the data.
////////////////////////////////////////////////////////////////////
template<class ValueType>
ValueType STBasicTerrain::InterpolationData<ValueType>::
get_nearest_neighbor(PN_stdfloat u, PN_stdfloat v) const {
  int u = int(u * _width + 0.5f);
  int v = int(v * _height + 0.5f);
  int index = u + v * _width;
  nassertr(index >= 0 && index < (int)_data.size(), 0);
  return _data[index];
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::calc_bilnear_interpolation
//       Access: Public
//  Description: Interpolates the value at (u, v) between its four
//               nearest neighbors.
////////////////////////////////////////////////////////////////////
template<class ValueType>
ValueType STBasicTerrain::InterpolationData<ValueType>::
calc_bilinear_interpolation(PN_stdfloat u, PN_stdfloat v) const {
  u -= cfloor(u);
  v -= cfloor(v);
  
  u *= (PN_stdfloat)_width;
  v *= (PN_stdfloat)_height;
  
  const int lower_x = int(u);
  const int lower_y = int(v);
  const int higher_x = (lower_x + 1) % _width;
  const int higher_y = (lower_y + 1) % _height;
  
  const PN_stdfloat ratio_x = u - PN_stdfloat(lower_x);
  const PN_stdfloat ratio_y = v - PN_stdfloat(lower_y);
  const PN_stdfloat inv_ratio_x = 1.0f - ratio_x;
  const PN_stdfloat inv_ratio_y = 1.0f - ratio_y;
  
  nassertr(lower_x + lower_y * _width >= 0 && higher_x + higher_y * _width < (int)_data.size(), 0);

  const ValueType &t1 = _data[lower_x + lower_y * _width];
  const ValueType &t2 = _data[higher_x + lower_y * _width];
  const ValueType &t3 = _data[lower_x + higher_y * _width];
  const ValueType &t4 = _data[higher_x + higher_y * _width];
  
  return (t1 * inv_ratio_x + t2 * ratio_x) * inv_ratio_y + 
    (t3 * inv_ratio_x + t4 * ratio_x) * ratio_y;
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::calc_smooth
//       Access: Public
//  Description: Approximates the average value at (u, v) over the
//               indicated radius, assuming a polynomial curve.
////////////////////////////////////////////////////////////////////
template<class ValueType>
ValueType STBasicTerrain::InterpolationData<ValueType>::
calc_smooth(PN_stdfloat u, PN_stdfloat v, PN_stdfloat radius) const {
  ValueType retval = 0;

  if (radius <= 0.0f) {
    retval = calc_bilinear_interpolation(u, v);

  } else {
    const PN_stdfloat test_points[9][2] = {
      {  0.0f * radius,   0.0f * radius },
      {  0.8f * radius,   0.0f * radius },
      { -0.8f * radius,   0.0f * radius },
      {  0.0f * radius,   0.8f * radius },
      {  0.0f * radius,  -0.8f * radius },
      {  0.25f * radius,  0.25f * radius },
      {  0.25f * radius, -0.25f * radius },
      { -0.25f * radius,  0.25f * radius },
      { -0.25f * radius, -0.25f * radius }
    };

    PN_stdfloat total_weight = 0.0f;
    for (int i = 0; i < 9; ++i) {
      const PN_stdfloat *test_point = test_points[i];
      PN_stdfloat weight = (1.0f - sqrt((test_point[0] * test_point[0]) + (test_point[1] * test_point[1])));
      total_weight += weight;
      retval += weight * calc_bilinear_interpolation(u + test_point[0], v + test_point[1]);
    }   
    
    retval /= total_weight;
  }
  
  return retval;
}

////////////////////////////////////////////////////////////////////
//     Function: STBasicTerrain::InterpolationData::is_present
//       Access: Public
//  Description: Returns true if the data is present--that is, reset()
//               was called with non-zero values--or false otherwise.
////////////////////////////////////////////////////////////////////
template<class ValueType>
bool STBasicTerrain::InterpolationData<ValueType>::
is_present() const {
  return !_data.empty();
}
