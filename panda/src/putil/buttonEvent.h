// Filename: buttonEvent.h
// Created by:  drose (01Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include "pandabase.h"

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonEvent
// Description : Records a button event of some kind.  This is either
//               a keyboard or mouse button (or some other kind of
//               button) changing state from up to down, or
//               vice-versa, or it is a single "keystroke".
//
//               A keystroke is different than a button event in that
//               (a) it does not necessarily correspond to a physical
//               button on a keyboard, but might be the result of a
//               combination of buttons (e.g. "A" is the result of
//               shift + "a"); and (b) it does not manage separate
//               "up" and "down" events, but is itself an
//               instantaneous event.  
//
//               Normal up/down button events can be used to track the
//               state of a particular button on the keyboard, while
//               keystroke events are best used to monitor what a user
//               is attempting to type.
//
//               Button up/down events are defined across all the
//               physical keys on the keyboard (and other buttons for
//               which there is a corresponding ButtonHandle object),
//               while keystroke events are defined across the entire
//               Unicode character set.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEvent {
public:
  enum Type {
    T_down,
    T_up,
    T_keystroke
  };

  INLINE ButtonEvent();
  INLINE ButtonEvent(ButtonHandle button, Type type);
  INLINE ButtonEvent(short keycode);
  INLINE ButtonEvent(const ButtonEvent &copy);
  INLINE void operator = (const ButtonEvent &copy);

  INLINE bool operator == (const ButtonEvent &other) const;
  INLINE bool operator != (const ButtonEvent &other) const;
  INLINE bool operator < (const ButtonEvent &other) const;

  void output(ostream &out) const;

  // _button will be filled in if type is T_down or T_up.
  ButtonHandle _button;

  // _keycode will be filled in if type is T_keystroke.  It will be
  // the Unicode character that was typed.
  short _keycode;

  Type _type;
};

INLINE ostream &operator << (ostream &out, const ButtonEvent &be) {
  be.output(out);
  return out;
}

#include "buttonEvent.I"

#endif

