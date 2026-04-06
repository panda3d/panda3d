/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoPoints.I
 * @author drose
 * @date 2001-04-25
 */

/**
 *
 */
INLINE CLwoPoints::
CLwoPoints(LwoToEggConverter *converter, const LwoPoints *points,
           CLwoLayer *layer) :
  _converter(converter),
  _points(points),
  _layer(layer)
{
}
