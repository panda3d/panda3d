// Filename: windowsGuid.h
// Created by:  drose (03Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef WINDOWS_GUID_H
#define WINDOWS_GUID_H

#include "pandatoolbase.h"

#include <string.h>  // For memcpy, memcmp

////////////////////////////////////////////////////////////////////
//       Class : WindowsGuid
// Description : This is an implementation of the Windows GUID object,
//               used everywhere as a world-unique identifier for
//               anything and everything.  In particular, it's used in
//               the X file format to identify standard templates.
////////////////////////////////////////////////////////////////////
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

  bool parse_string(const string &str);
  string format_string() const;

  void output(ostream &out) const;

private:
  unsigned long _data1;
  unsigned short _data2;
  unsigned short _data3;
  unsigned char _b1, _b2, _b3, _b4, _b5, _b6, _b7, _b8;
};

INLINE ostream &operator << (ostream &out, const WindowsGuid &guid);

#include "windowsGuid.I"

#endif

