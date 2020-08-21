/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffId.I
 * @author drose
 * @date 2001-04-23
 */

/**
 *
 */
INLINE IffId::
IffId() {
  _id._c[0] = 0;
  _id._c[1] = 0;
  _id._c[2] = 0;
  _id._c[3] = 0;
}

/**
 *
 */
INLINE IffId::
IffId(const char id[4]) {
  _id._c[0] = id[0];
  _id._c[1] = id[1];
  _id._c[2] = id[2];
  _id._c[3] = id[3];
}

/**
 *
 */
INLINE IffId::
IffId(const IffId &copy) {
  _id._n = copy._id._n;
}

/**
 *
 */
INLINE void IffId::
operator = (const IffId &copy) {
  _id._n = copy._id._n;
}

/**
 *
 */
INLINE bool IffId::
operator == (const IffId &other) const {
  return (_id._n == other._id._n);
}

/**
 *
 */
INLINE bool IffId::
operator != (const IffId &other) const {
  return (_id._n != other._id._n);
}

/**
 * The ordering is arbitrary, and may not even be consistent between different
 * architectures (e.g.  big-endian and little-endian).  It is useful mainly
 * for putting IffId's into a sorted container, like sets and maps.
 */
INLINE bool IffId::
operator < (const IffId &other) const {
  return (_id._n < other._id._n);
}

/**
 * Returns the four-character name of the Id, for outputting.
 */
INLINE std::string IffId::
get_name() const {
  return std::string(_id._c, 4);
}
