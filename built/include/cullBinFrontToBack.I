/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinFrontToBack.I
 * @author drose
 * @date 2002-05-29
 */

/**
 *
 */
INLINE CullBinFrontToBack::
CullBinFrontToBack(const std::string &name, GraphicsStateGuardianBase *gsg,
                   const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_front_to_back, gsg, draw_region_pcollector)
{
}

/**
 *
 */
INLINE CullBinFrontToBack::ObjectData::
ObjectData(CullableObject *object, PN_stdfloat dist) :
  _object(object),
  _dist(dist)
{
}

/**
 * Specifies the correct sort ordering for these objects.
 */
INLINE bool CullBinFrontToBack::ObjectData::
operator < (const ObjectData &other) const {
  return _dist < other._dist;
}
