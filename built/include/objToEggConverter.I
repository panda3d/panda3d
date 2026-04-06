/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file objToEggConverter.I
 * @author drose
 * @date 2013-01-03
 */

/**
 * Provides a unique but arbitrary ordering for VertexEntry objects in a map.
 */
INLINE bool ObjToEggConverter::VertexEntry::
operator < (const VertexEntry &other) const {
  if (_vi != other._vi) {
    return _vi < other._vi;
  }
  if (_vti != other._vti) {
    return _vti < other._vti;
  }

  // It's important that these two tests are made last, so we can find the
  // first vertex that has any normal but also matches the above properties.
  if (_vni != other._vni) {
    return _vni < other._vni;
  }
  if (_synth_vni != other._synth_vni) {
    return _synth_vni < other._synth_vni;
  }
  return false;
}

/**
 *
 */
INLINE bool ObjToEggConverter::VertexEntry::
operator == (const VertexEntry &other) const {
  return (_vi == other._vi && _vti == other._vti &&
          _vni == other._vni && _synth_vni == other._synth_vni);
}

/**
 * Returns true if all the properties except _vni and _synth_vni are
 * equivalent.
 */
INLINE bool ObjToEggConverter::VertexEntry::
matches_except_normal(const VertexEntry &other) const {
  return (_vi == other._vi && _vti == other._vti);
}
