/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitMask_ext.I
 * @author rdb
 * @date 2020-03-22
 */

/**
 * Returns true if the value is not zero.
 */
template<class WType, int nbits>
INLINE bool Extension<BitMask<WType, nbits> >::
__bool__() const {
  return this->_this->get_word() != 0;
}

/**
 * Returns the value as an integer.
 */
template<class WType, int nbits>
INLINE PyObject *Extension<BitMask<WType, nbits> >::
__int__() const {
  return Dtool_WrapValue(this->_this->get_word());
}

/**
 * This special Python method is implemented to provide support for the pickle
 * module.
 */
template<class WType, int nbits>
INLINE PyObject *Extension<BitMask<WType, nbits> >::
__reduce__(PyObject *self) const {
  // We should return at least a 2-tuple, (Class, (args)): the necessary class
  // object whose constructor we should call (e.g.  this), and the arguments
  // necessary to reconstruct this object.
  return Py_BuildValue("(O(k))", Py_TYPE(self), this->_this->get_word());
}
