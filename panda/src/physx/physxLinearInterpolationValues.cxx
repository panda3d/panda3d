/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxLinearInterpolationValues.cxx
 * @author enn0x
 * @date 2010-02-08
 */

#include "physxLinearInterpolationValues.h"

/**
 *
 */
void PhysxLinearInterpolationValues::
clear() {

  _map.clear();
}

/**
 *
 */
void PhysxLinearInterpolationValues::
insert(float index, float value) {

  if (_map.empty()) {
    _min = _max = index;
  }
  else {
    _min = std::min(_min, index);
    _max = std::max(_max, index);
  }
  _map[index] = value;
}

/**
 *
 */
bool PhysxLinearInterpolationValues::
is_valid(float number) const {

  return (number >= _min) && (number <= _max);
}

/**
 *
 */
unsigned int PhysxLinearInterpolationValues::
get_size() const {

  return _map.size();
}

/**
 *
 */
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

/**
 *
 */
float PhysxLinearInterpolationValues::
get_value_at_index(int index) const {

  MapType::const_iterator it = _map.begin();

  for (int i=0; i<index; i++) {
    ++it;
  }

  return it->second;
}

/**
 *
 */
void PhysxLinearInterpolationValues::
output(std::ostream &out) const {

  MapType::const_iterator it = _map.begin();

  for (; it != _map.end(); ++it) {
    std::cout << it->first << " -> " << it->second << "\n";
  }
}
