// Filename: colorTransition.I
// Created by:  drose (22Mar00)
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

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorTransition::
ColorTransition() {
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorTransition::
ColorTransition(const Colorf &color) :
  OnOffTransition(TD_on),
  _value(color)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE ColorTransition::
ColorTransition(float r, float g, float b, float a) :
  OnOffTransition(TD_on),
  _value(Colorf(r, g, b, a))
{
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::uncolor
//       Access: Public
//  Description: This named constructor returns a ColorTransition that
//               turns on the "uncolor", i.e. turns off all sending of
//               color commands down the pipe.
////////////////////////////////////////////////////////////////////
INLINE ColorTransition ColorTransition::
uncolor() {
  ColorTransition result;
  result.set_uncolor();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::off
//       Access: Public
//  Description: This named constructor returns a ColorTransition that
//               turns off scene graph color, thus allowing the
//               geometry's natural polygon or vertex color to show
//               through.
////////////////////////////////////////////////////////////////////
INLINE ColorTransition ColorTransition::
off() {
  ColorTransition result;
  result.set_off();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::set_on
//       Access: Public
//  Description: Changes the ColorTransition to turn on the particular
//               color.
////////////////////////////////////////////////////////////////////
INLINE void ColorTransition::
set_on(const Colorf &color) {
  _value = ColorProperty(color);
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::set_on
//       Access: Public
//  Description: Changes the ColorTransition to turn on the particular
//               color.
////////////////////////////////////////////////////////////////////
INLINE void ColorTransition::
set_on(float r, float g, float b, float a) {
  set_on(Colorf(r, g, b, a));
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::set_uncolor
//       Access: Public
//  Description: Changes the ColorTransition to turn on uncolor.
////////////////////////////////////////////////////////////////////
INLINE void ColorTransition::
set_uncolor() {
  _value = ColorProperty();
  OnOffTransition::set_on();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::is_real
//       Access: Public
//  Description: Returns true if the ColorTransition represents a real
//               color, false if it represents the uncolor.  It is
//               only valid to call this if is_on() has returned true.
////////////////////////////////////////////////////////////////////
INLINE bool ColorTransition::
is_real() const {
  nassertr(is_on(), false);
  return _value.is_real();
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::get_color
//       Access: Public
//  Description: Returns the color that the ColorTransition
//               represents.  It is only valid to call this if is_on()
//               and is_real() have both returned true.
////////////////////////////////////////////////////////////////////
INLINE Colorf ColorTransition::
get_color() const {
  nassertr(is_on() && is_real(), Colorf(0.0, 0.0, 0.0, 0.0));
  return _value.get_color();
}
