/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoPolygons.I
 * @author drose
 * @date 2001-04-25
 */

/**
 *
 */
INLINE CLwoPolygons::
CLwoPolygons(LwoToEggConverter *converter, const LwoPolygons *polygons,
             CLwoPoints *points) :
  _converter(converter),
  _polygons(polygons),
  _points(points)
{
  _tags = nullptr;
  _surf_ptags = nullptr;
}
