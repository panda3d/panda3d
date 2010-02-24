// Filename: physxLinearInterpolationValues.cxx
// Created by:  enn0x (08Feb10)
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

#include "physxLinearInterpolationValues.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::clear
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxLinearInterpolationValues::
clear() {

  _map.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::insert
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxLinearInterpolationValues::
insert(float index, float value) {

  if (_map.empty()) {
    _min = _max = index;
  }
  else {
    _min = min(_min, index);
    _max = max(_max, index);
  }
  _map[index] = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::is_valid
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxLinearInterpolationValues::
is_valid(float number) const {

  return (number >= _min) && (number <= _max);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::get_size
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
unsigned int PhysxLinearInterpolationValues::
get_size() const {

  return _map.size();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::get_value
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxLinearInterpolationValues::
get_value(float number) const {

  MapType::const_iterator lower = _map.begin();
  if (number < _min) {
    return lower->second;
  }

  MapType::const_iterator upper = _map.end();
  upper--;
  if (number > _max) {
    return upper->second;
  }

  upper = _map.lower_bound(number);
  if (upper == lower) {
    return upper->second;
  }

  lower = upper;
  lower--;
    
  float w1 = number - lower->first;
  float w2 = upper->first - number;

  return ((w2 * lower->second) + (w1 * upper->second)) / (w1 + w2);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::get_value_at_index
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
float PhysxLinearInterpolationValues::
get_value_at_index(int index) const {

  MapType::const_iterator it = _map.begin();

  for (int i=0; i<index; i++) {
    ++it;
  }

  return it->second;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxLinearInterpolationValues::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxLinearInterpolationValues::
output(ostream &out) const {

  MapType::const_iterator it = _map.begin();

  for (; it != _map.end(); ++it) {
    cout << it->first << " -> " << it->second << "\n";
  }
}

