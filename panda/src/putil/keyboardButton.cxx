// Filename: keyboardButton.cxx
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

#include "keyboardButton.h"
#include "buttonRegistry.h"

#include <ctype.h>

static ButtonHandle _space;
static ButtonHandle _backspace;
static ButtonHandle _tab;
static ButtonHandle _enter;
static ButtonHandle _escape;

static ButtonHandle _f1;
static ButtonHandle _f2;
static ButtonHandle _f3;
static ButtonHandle _f4;
static ButtonHandle _f5;
static ButtonHandle _f6;
static ButtonHandle _f7;
static ButtonHandle _f8;
static ButtonHandle _f9;
static ButtonHandle _f10;
static ButtonHandle _f11;
static ButtonHandle _f12;

static ButtonHandle _left;
static ButtonHandle _right;
static ButtonHandle _up;
static ButtonHandle _down;
static ButtonHandle _page_up;
static ButtonHandle _page_down;
static ButtonHandle _home;
static ButtonHandle _end;
static ButtonHandle _insert;
static ButtonHandle _del;

static ButtonHandle _shift;
static ButtonHandle _control;
static ButtonHandle _alt;
static ButtonHandle _meta;
static ButtonHandle _caps_lock;
static ButtonHandle _shift_lock;


////////////////////////////////////////////////////////////////////
//     Function: KeyboardButton::ascii_key
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               particular ASCII character, if there is one, or
//               ButtonHandle::none() if there is not.
////////////////////////////////////////////////////////////////////
ButtonHandle KeyboardButton::
ascii_key(char ascii_equivalent) {
  return ButtonRegistry::ptr()->find_ascii_button(ascii_equivalent);
}

////////////////////////////////////////////////////////////////////
//     Function: KeyboardButton::space
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               Space bar.
////////////////////////////////////////////////////////////////////
ButtonHandle KeyboardButton::
space() {
  return _space;
}

////////////////////////////////////////////////////////////////////
//     Function: KeyboardButton::backspace
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               Backspace key.  Most of the remaining methods in this
//               file are similar.
////////////////////////////////////////////////////////////////////
ButtonHandle KeyboardButton::
backspace() {
  return _backspace;
}

ButtonHandle KeyboardButton::
tab() {
  return _tab;
}

ButtonHandle KeyboardButton::
enter() {
  return _enter;
}

ButtonHandle KeyboardButton::
escape() {
  return _escape;
}

ButtonHandle KeyboardButton::
f1() {
  return _f1;
}

ButtonHandle KeyboardButton::
f2() {
  return _f2;
}

ButtonHandle KeyboardButton::
f3() {
  return _f3;
}

ButtonHandle KeyboardButton::
f4() {
  return _f4;
}

ButtonHandle KeyboardButton::
f5() {
  return _f5;
}

ButtonHandle KeyboardButton::
f6() {
  return _f6;
}

ButtonHandle KeyboardButton::
f7() {
  return _f7;
}

ButtonHandle KeyboardButton::
f8() {
  return _f8;
}

ButtonHandle KeyboardButton::
f9() {
  return _f9;
}

ButtonHandle KeyboardButton::
f10() {
  return _f10;
}

ButtonHandle KeyboardButton::
f11() {
  return _f11;
}

ButtonHandle KeyboardButton::
f12() {
  return _f12;
}

ButtonHandle KeyboardButton::
left() {
  return _left;
}

ButtonHandle KeyboardButton::
right() {
  return _right;
}

ButtonHandle KeyboardButton::
up() {
  return _up;
}

ButtonHandle KeyboardButton::
down() {
  return _down;
}

ButtonHandle KeyboardButton::
page_up() {
  return _page_up;
}

ButtonHandle KeyboardButton::
page_down() {
  return _page_down;
}

ButtonHandle KeyboardButton::
home() {
  return _home;
}

ButtonHandle KeyboardButton::
end() {
  return _end;
}

ButtonHandle KeyboardButton::
insert() {
  return _insert;
}

ButtonHandle KeyboardButton::
del() {
  return _del;
}

ButtonHandle KeyboardButton::
shift() {
  return _shift;
}

ButtonHandle KeyboardButton::
control() {
  return _control;
}

ButtonHandle KeyboardButton::
alt() {
  return _alt;
}

ButtonHandle KeyboardButton::
meta() {
  return _meta;
}

ButtonHandle KeyboardButton::
caps_lock() {
  return _caps_lock;
}

ButtonHandle KeyboardButton::
shift_lock() {
  return _shift_lock;
}

////////////////////////////////////////////////////////////////////
//     Function: KeyboardButton::init_keyboard_buttons
//       Access: Public, Static
//  Description: This is intended to be called only once, by the
//               static initialization performed in config_util.cxx.
////////////////////////////////////////////////////////////////////
void KeyboardButton::
init_keyboard_buttons() {
  ButtonRegistry::ptr()->register_button(_space, "space", ' ');
  ButtonRegistry::ptr()->register_button(_backspace, "backspace", '\x08');
  ButtonRegistry::ptr()->register_button(_tab, "tab", '\x09');
  ButtonRegistry::ptr()->register_button(_enter, "enter", '\x0d');
  ButtonRegistry::ptr()->register_button(_escape, "escape", '\x1b');
  ButtonRegistry::ptr()->register_button(_del, "delete", '\x7f');

  ButtonRegistry::ptr()->register_button(_f1, "f1");
  ButtonRegistry::ptr()->register_button(_f2, "f2");
  ButtonRegistry::ptr()->register_button(_f3, "f3");
  ButtonRegistry::ptr()->register_button(_f4, "f4");
  ButtonRegistry::ptr()->register_button(_f5, "f5");
  ButtonRegistry::ptr()->register_button(_f6, "f6");
  ButtonRegistry::ptr()->register_button(_f7, "f7");
  ButtonRegistry::ptr()->register_button(_f8, "f8");
  ButtonRegistry::ptr()->register_button(_f9, "f9");
  ButtonRegistry::ptr()->register_button(_f10, "f10");
  ButtonRegistry::ptr()->register_button(_f11, "f11");
  ButtonRegistry::ptr()->register_button(_f12, "f12");

  ButtonRegistry::ptr()->register_button(_left, "left");
  ButtonRegistry::ptr()->register_button(_right, "right");
  ButtonRegistry::ptr()->register_button(_up, "up");
  ButtonRegistry::ptr()->register_button(_down, "down");
  ButtonRegistry::ptr()->register_button(_page_up, "page_up");
  ButtonRegistry::ptr()->register_button(_page_down, "page_down");
  ButtonRegistry::ptr()->register_button(_home, "home");
  ButtonRegistry::ptr()->register_button(_end, "end");
  ButtonRegistry::ptr()->register_button(_insert, "insert");

  ButtonRegistry::ptr()->register_button(_shift, "shift");
  ButtonRegistry::ptr()->register_button(_control, "control");
  ButtonRegistry::ptr()->register_button(_alt, "alt");
  ButtonRegistry::ptr()->register_button(_meta, "meta");
  ButtonRegistry::ptr()->register_button(_caps_lock, "caps_lock");
  ButtonRegistry::ptr()->register_button(_shift_lock, "shift_lock");

  // Also register all of the visible ASCII characters.
  for (int i = 32; i < 127; i++) {
    if (isgraph(i)) {
      ButtonHandle key;
      ButtonRegistry::ptr()->register_button(key, string(1, (char)i), i);
    }
  }
}
