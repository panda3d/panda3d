// Filename: buttonHandle.h
// Created by:  drose (01Mar00)
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

#ifndef BUTTONHANDLE_H
#define BUTTONHANDLE_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonHandle
// Description : A ButtonHandle represents a single button from any
//               device, including keyboard buttons and mouse buttons
//               (but see KeyboardButton and MouseButton).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonHandle {
PUBLISHED:
  INLINE ButtonHandle();

public:
  INLINE ButtonHandle(const ButtonHandle &copy);

  INLINE bool operator == (const ButtonHandle &other) const;
  INLINE bool operator != (const ButtonHandle &other) const;
  INLINE bool operator < (const ButtonHandle &other) const;

PUBLISHED:
  string get_name() const;
  INLINE bool has_ascii_equivalent() const;
  INLINE char get_ascii_equivalent() const;

  INLINE int get_index() const;
  INLINE void output(ostream &out) const;
  INLINE static ButtonHandle none();

private:
  int _index;
  static ButtonHandle _none;

friend class ButtonRegistry;
};

// It's handy to be able to output a ButtonHandle directly, and see the
// button name.
INLINE ostream &operator << (ostream &out, ButtonHandle button) {
  button.output(out);
  return out;
}

#include "buttonHandle.I"

#endif

