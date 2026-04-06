/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereLight.I
 * @author rdb
 * @date 2016-04-15
 */

/**
 *
 */
INLINE SphereLight::CData::
CData() :
  _radius(0.01f)
{
}

/**
 *
 */
INLINE SphereLight::CData::
CData(const SphereLight::CData &copy) :
  _radius(copy._radius)
{
}

/**
 * Returns the radius of the sphere.
 */
INLINE PN_stdfloat SphereLight::
get_radius() const {
  CDReader cdata(_cycler);
  return cdata->_radius;
}

/**
 * Sets the radius of the sphere.
 */
INLINE void SphereLight::
set_radius(PN_stdfloat radius) {
  CDWriter cdata(_cycler);
  cdata->_radius = radius;
}
