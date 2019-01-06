/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexAux.I
 * @author jenes
 * @date 2011-11-15
 */

/**
 *
 */
INLINE void EggVertexAux::
set_name(const std::string &name) {
  Namable::set_name(name);
}

/**
 * Returns the auxiliary data quadruple.
 */
INLINE const LVecBase4d &EggVertexAux::
get_aux() const {
  return _aux;
}

/**
 * Sets the auxiliary data quadruple.
 */
INLINE void EggVertexAux::
set_aux(const LVecBase4d &aux) {
  _aux = aux;
}
