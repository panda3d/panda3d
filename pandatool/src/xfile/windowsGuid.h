/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file windowsGuid.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef WINDOWS_GUID_H
#define WINDOWS_GUID_H

#include "pandatoolbase.h"

#include <string.h>  // For memcpy, memcmp

/**
 * This is an implementation of the Windows GUID object, used everywhere as a
 * world-unique identifier for anything and everything.  In particular, it's
 * used in the X file format to identify standard templates.
 */
class WindowsGuid {
public:
  INLINE WindowsGuid();
  INLINE WindowsGuid(unsigned long data1,
                     unsigned short data2, unsigned short data3,
                     unsigned char b1, unsigned char b2, unsigned char b3,
                     unsigned char b4, unsigned char b5, unsigned char b6,
                     unsigned char b7, unsigned char b8);
  INLINE WindowsGuid(const WindowsGuid &copy);
  INLINE void operator = (const WindowsGuid &copy);

  INLINE bool operator == (const WindowsGuid &other) const;
  INLINE bool operator != (const WindowsGuid &other) const;
  INLINE bool operator < (const WindowsGuid &other) const;
  INLINE int compare_to(const WindowsGuid &other) const;

  bool parse_string(const std::string &str);
  std::string format_string() const;

  void output(std::ostream &out) const;

private:
  unsigned long _data1;
  unsigned short _data2;
  unsigned short _data3;
  unsigned char _b1, _b2, _b3, _b4, _b5, _b6, _b7, _b8;
};

INLINE std::ostream &operator << (std::ostream &out, const WindowsGuid &guid);

#include "windowsGuid.I"

#endif
