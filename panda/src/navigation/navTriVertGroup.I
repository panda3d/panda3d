/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navTriVertGroup.I
 * @author Maxwell175
 * @date 2022-07-14
 */

INLINE NavTriVertGroup::
NavTriVertGroup(LVector3 _a, LVector3 _b, LVector3 _c) :
  a(_a), b(_b), c(_c)
{}

/**
 * Sets the "a" vertex.
 */
INLINE void NavTriVertGroup::
set_a(LVector3 _a) {
  a = _a;
}

/**
 * Returns the "a" vertex.
 */
INLINE LVector3 NavTriVertGroup::
get_a() const {
  return a;
}

/**
 * Sets the "b" vertex.
 */
INLINE void NavTriVertGroup::
set_b(LVector3 _b) {
  b = _b;
}

/**
 * Returns the "b" vertex.
 */
INLINE LVector3 NavTriVertGroup::
get_b() const {
  return b;
}

/**
 * Sets the "c" vertex.
 */
INLINE void NavTriVertGroup::
set_c(LVector3 _c) {
  c = _c;
}

/**
 * Returns the "c" vertex.
 */
INLINE LVector3 NavTriVertGroup::
get_c() const {
  return c;
}

INLINE bool NavTriVertGroup::
operator==(const NavTriVertGroup &other) const {
  return std::tie(a, b, c) ==
         std::tie(other.a, other.b, other.c);
}

INLINE bool NavTriVertGroup::
operator<(const NavTriVertGroup &other) const {
  return std::tie(a, b, c) <
         std::tie(other.a, other.b, other.c);
}
