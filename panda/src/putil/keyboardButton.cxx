// Filename: keyboardButton.cxx
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

#include "keyboardButton.h"
#include "buttonRegistry.h"

#include <ctype.h>

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
//     Function: KeyboardButton::ascii_key
//       Access: Public, Static
//  Description: Returns the ButtonHandle associated with the
//               particular ASCII character (taken from the first
//               character of the indicated string), if there is one,
//               or ButtonHandle::none() if there is not.
////////////////////////////////////////////////////////////////////
ButtonHandle KeyboardButton::
ascii_key(const string &ascii_equivalent) {
  if (ascii_equivalent.empty()) {
    return ButtonHandle::none();
  } else {
    return ButtonRegistry::ptr()->find_ascii_button(ascii_equivalent[0]);
  }
}

#define DEFINE_KEYBD_BUTTON_HANDLE(KeyName)     \
                  static ButtonHandle _##KeyName; \
                  ButtonHandle KeyboardButton::KeyName() { return _##KeyName; }
                  

DEFINE_KEYBD_BUTTON_HANDLE(space)
DEFINE_KEYBD_BUTTON_HANDLE(backspace)
DEFINE_KEYBD_BUTTON_HANDLE(tab)
DEFINE_KEYBD_BUTTON_HANDLE(enter)
DEFINE_KEYBD_BUTTON_HANDLE(escape)
DEFINE_KEYBD_BUTTON_HANDLE(f1)
DEFINE_KEYBD_BUTTON_HANDLE(f2)
DEFINE_KEYBD_BUTTON_HANDLE(f3)
DEFINE_KEYBD_BUTTON_HANDLE(f4)
DEFINE_KEYBD_BUTTON_HANDLE(f5)
DEFINE_KEYBD_BUTTON_HANDLE(f6)
DEFINE_KEYBD_BUTTON_HANDLE(f7)
DEFINE_KEYBD_BUTTON_HANDLE(f8)
DEFINE_KEYBD_BUTTON_HANDLE(f9)
DEFINE_KEYBD_BUTTON_HANDLE(f10)
DEFINE_KEYBD_BUTTON_HANDLE(f11)
DEFINE_KEYBD_BUTTON_HANDLE(f12)
DEFINE_KEYBD_BUTTON_HANDLE(left)
DEFINE_KEYBD_BUTTON_HANDLE(right)
DEFINE_KEYBD_BUTTON_HANDLE(up)
DEFINE_KEYBD_BUTTON_HANDLE(down)
DEFINE_KEYBD_BUTTON_HANDLE(page_up)
DEFINE_KEYBD_BUTTON_HANDLE(page_down)
DEFINE_KEYBD_BUTTON_HANDLE(home)
DEFINE_KEYBD_BUTTON_HANDLE(end)
DEFINE_KEYBD_BUTTON_HANDLE(insert)
DEFINE_KEYBD_BUTTON_HANDLE(del)
DEFINE_KEYBD_BUTTON_HANDLE(meta)
DEFINE_KEYBD_BUTTON_HANDLE(caps_lock)
DEFINE_KEYBD_BUTTON_HANDLE(shift_lock)
DEFINE_KEYBD_BUTTON_HANDLE(scroll_lock)
DEFINE_KEYBD_BUTTON_HANDLE(num_lock)
DEFINE_KEYBD_BUTTON_HANDLE(print_screen)
DEFINE_KEYBD_BUTTON_HANDLE(shift)
DEFINE_KEYBD_BUTTON_HANDLE(control)
DEFINE_KEYBD_BUTTON_HANDLE(alt)


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

  ButtonRegistry::ptr()->register_button(_left, "arrow_left");
  ButtonRegistry::ptr()->register_button(_right, "arrow_right");
  ButtonRegistry::ptr()->register_button(_up, "arrow_up");  // cannot name this 'up' since it conflicts with key-release name 'up'
  ButtonRegistry::ptr()->register_button(_down, "arrow_down");  
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
  ButtonRegistry::ptr()->register_button(_num_lock, "num_lock");
  ButtonRegistry::ptr()->register_button(_scroll_lock, "scroll_lock");
  ButtonRegistry::ptr()->register_button(_print_screen, "print_screen");

  // Also register all of the visible ASCII characters.
  for (int i = 32; i < 127; i++) {
    if (isgraph(i)) {
      ButtonHandle key;
      ButtonRegistry::ptr()->register_button(key, string(1, (char)i), i);
    }
  }
}
