// Filename: buttonEvent.cxx
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

#include "buttonEvent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "buttonRegistry.h"

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void ButtonEvent::
output(ostream &out) const {
  switch (_type) {
  case T_down:
    out << "button " << _button << " down";
    break;

  case T_resume_down:
    out << "button " << _button << " resume down";
    break;

  case T_up:
    out << "button " << _button << " up";
    break;

  case T_keystroke:
    out << "keystroke " << _keycode;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::write_datagram
//       Access: Public
//  Description: Writes the event into a datagram.
////////////////////////////////////////////////////////////////////
void ButtonEvent::
write_datagram(Datagram &dg) const {
  dg.add_uint8(_type);
  switch (_type) {
  case T_down:
  case T_resume_down:
  case T_up:
    // We write the button name.  This is not particularly compact, but
    // presumably we don't get thousands of button events per frame, and
    // it is robust as the button index may change between sessions but
    // the button name will not.
    dg.add_string(_button.get_name());
    break;

  case T_keystroke:
    dg.add_int16(_keycode);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ButtonEvent::read_datagram
//       Access: Public
//  Description: Restores the event from the datagram.
////////////////////////////////////////////////////////////////////
void ButtonEvent::
read_datagram(DatagramIterator &scan) {
  _type = (Type)scan.get_uint8();
  switch (_type) {
  case T_down:
  case T_resume_down:
  case T_up:
    _button = ButtonRegistry::ptr()->get_button(scan.get_string());
    break;

  case T_keystroke:
    _keycode = scan.get_int16();
    break;
  }
}
