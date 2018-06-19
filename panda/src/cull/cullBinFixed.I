/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinFixed.I
 * @author drose
 * @date 2002-05-29
 */

/**
 *
 */
INLINE CullBinFixed::
CullBinFixed(const std::string &name, GraphicsStateGuardianBase *gsg,
             const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_fixed, gsg, draw_region_pcollector)
{
}

/**
 *
 */
INLINE CullBinFixed::ObjectData::
ObjectData(CullableObject *object, int draw_order) :
  _object(object),
  _draw_order(draw_order)
{
}

/**
 * Specifies the correct sort ordering for these objects.
 */
INLINE bool CullBinFixed::ObjectData::
operator < (const ObjectData &other) const {
  return _draw_order < other._draw_order;
}
