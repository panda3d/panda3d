/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file perspectiveLens.I
 * @author drose
 * @date 1999-02-18
 */

/**
 *
 */
INLINE PerspectiveLens::
PerspectiveLens() {
}

/**
 *
 */
INLINE PerspectiveLens::
PerspectiveLens(PN_stdfloat hfov, PN_stdfloat vfov) {
  Lens::CDWriter lens_cdata(Lens::_cycler, true);
  lens_cdata->_fov.set(hfov, vfov);
}

/**
 *
 */
INLINE PerspectiveLens::
PerspectiveLens(const PerspectiveLens &copy) : Lens(copy) {
}

/**
 *
 */
INLINE void PerspectiveLens::
operator = (const PerspectiveLens &copy) {
  Lens::operator = (copy);
}
