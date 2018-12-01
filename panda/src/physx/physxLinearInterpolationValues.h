/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxLinearInterpolationValues.h
 * @author enn0x
 * @date 2010-02-08
 */

#ifndef PHYSXLINEARINTERPOLATIONVALUES_H
#define PHYSXLINEARINTERPOLATIONVALUES_H

#include "pandabase.h"
#include "pmap.h"

#include "physx_includes.h"

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxLinearInterpolationValues {

public:
  INLINE PhysxLinearInterpolationValues();
  INLINE ~PhysxLinearInterpolationValues();

  void output(std::ostream &out) const;

  void clear();
  void insert(float index, float value);
  bool is_valid(float number) const;
  float get_value(float number) const;
  float get_value_at_index(int index) const;
  unsigned int get_size() const;

private:
  float _min;
  float _max;

  typedef pmap<float, float> MapType;
  MapType _map;
};

INLINE std::ostream &operator << (std::ostream &out, const PhysxLinearInterpolationValues &values) {
  values.output(out);
  return out;
}

#include "physxLinearInterpolationValues.I"

#endif // PHYSXLINEARINTERPOLATIONVALUES_H
