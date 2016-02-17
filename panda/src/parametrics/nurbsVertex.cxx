/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nurbsVertex.cxx
 * @author drose
 * @date 2002-12-04
 */

#include "nurbsVertex.h"


/**
 * Sets an n-dimensional vertex value.  This allows definition of a NURBS
 * surface or curve in a sparse n-dimensional space, typically used for
 * associating additional properties (like color or joint membership) with each
 * vertex of a surface.  The value d is an arbitrary integer value and specifies
 * the dimension of question for this particular vertex.  Any number of
 * dimensions may be specified, and they need not be consecutive.  If a value
 * for a given dimension is not specified, is it implicitly 0.0.  The value is
 * implicitly scaled by the homogenous weight value--that is, the fourth
 * component of the value passed to set_vertex().  This means the ordinary
 * vertex must be set first, before the extended vertices can be set.
 */
void NurbsVertex::
set_extended_vertex(int d, PN_stdfloat value) {
  _extended[d] = value * _vertex[3];
}

/**
 * Returns an n-dimensional vertex value.  See set_extended_vertex().  This
 * returns the value set for the indicated dimension, or 0.0 if nothing has been
 * set.
 */
PN_stdfloat NurbsVertex::
get_extended_vertex(int d) const {
  Extended::const_iterator ei;
  ei = _extended.find(d);
  if (ei == _extended.end()) {
    return 0.0f;
  }
  return (*ei).second;
}
