/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggOptcharUserData.I
 * @author drose
 * @date 2003-07-18
 */

/**
 *
 */
INLINE EggOptcharUserData::
EggOptcharUserData() {
  _flags = 0;
  _static_mat = LMatrix4d::ident_mat();
  _static_value = 0.0;
}


/**
 *
 */
INLINE EggOptcharUserData::
EggOptcharUserData(const EggOptcharUserData &copy) :
  EggUserData(copy),
  _flags(copy._flags),
  _static_mat(copy._static_mat),
  _static_value(copy._static_value)
{
}


/**
 *
 */
INLINE void EggOptcharUserData::
operator = (const EggOptcharUserData &copy) {
  EggUserData::operator = (copy);
  _flags = copy._flags;
  _static_mat = copy._static_mat;
  _static_value = copy._static_value;
}

/**
 *
 */
INLINE bool EggOptcharUserData::
is_static() const {
  return (_flags & F_static) != 0;
}

/**
 *
 */
INLINE bool EggOptcharUserData::
is_identity() const {
  return (_flags & F_identity) != 0;
}

/**
 *
 */
INLINE bool EggOptcharUserData::
is_empty() const {
  return (_flags & F_empty) != 0;
}

/**
 *
 */
INLINE bool EggOptcharUserData::
is_top() const {
  return (_flags & F_top) != 0;
}
