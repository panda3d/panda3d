/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowsGuid.I
 * @author drose
 * @date 2004-10-03
 */

/**
 *
 */
INLINE WindowsGuid::
WindowsGuid() {
  memset(this, 0, sizeof(WindowsGuid));
}

/**
 *
 */
INLINE WindowsGuid::
WindowsGuid(unsigned long data1,
            unsigned short data2, unsigned short data3,
            unsigned char b1, unsigned char b2, unsigned char b3,
            unsigned char b4, unsigned char b5, unsigned char b6,
            unsigned char b7, unsigned char b8) :
  _data1(data1),
  _data2(data2),
  _data3(data3),
  _b1(b1),
  _b2(b2),
  _b3(b3),
  _b4(b4),
  _b5(b5),
  _b6(b6),
  _b7(b7),
  _b8(b8)
{
}

/**
 *
 */
INLINE WindowsGuid::
WindowsGuid(const WindowsGuid &copy) {
  (*this) = copy;
}

/**
 *
 */
INLINE void WindowsGuid::
operator = (const WindowsGuid &copy) {
  memcpy(this, &copy, sizeof(WindowsGuid));
}

/**
 *
 */
INLINE bool WindowsGuid::
operator == (const WindowsGuid &other) const {
  return compare_to(other) == 0;
}

/**
 *
 */
INLINE bool WindowsGuid::
operator != (const WindowsGuid &other) const {
  return compare_to(other) != 0;
}

/**
 *
 */
INLINE bool WindowsGuid::
operator < (const WindowsGuid &other) const {
  return compare_to(other) < 0;
}

/**
 * Returns a number less than zero if this WindowsGuid sorts before the other
 * one, greater than zero if it sorts after, or zero if they are equivalent.
 */
INLINE int WindowsGuid::
compare_to(const WindowsGuid &other) const {
  return memcmp(this, &other, sizeof(WindowsGuid));
}

INLINE std::ostream &
operator << (std::ostream &out, const WindowsGuid &guid) {
  guid.output(out);
  return out;
}
