/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinBackToFront.I
 * @author drose
 * @date 2002-02-28
 */

/**
 *
 */
INLINE CullBinBackToFront::
CullBinBackToFront(const std::string &name, GraphicsStateGuardianBase *gsg,
                   const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_back_to_front, gsg, draw_region_pcollector)
{
}

/**
 *
 */
INLINE CullBinBackToFront::ObjectData::
ObjectData(CullableObject *object, PN_stdfloat dist) :
  _object(object),
  _dist(dist)
{
}

/**
 * Specifies the correct sort ordering for these objects.
 */
INLINE bool CullBinBackToFront::ObjectData::
operator < (const ObjectData &other) const {
  return _dist > other._dist;
}
