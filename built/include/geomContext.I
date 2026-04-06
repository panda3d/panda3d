/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomContext.I
 * @author drose
 * @date 2004-03-19
 */

/**
 *
 */
INLINE GeomContext::
GeomContext(Geom *geom) :
  _geom(geom)
{
}

/**
 *
 */
INLINE Geom *GeomContext::
get_geom() const {
  return _geom;
}
