// Filename: stencilTransition.I
// Created by:  drose (23Mar00)
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
//     Function: StencilTransition::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilTransition::
StencilTransition(StencilProperty::Mode mode, StencilProperty::Action pass_action, 
                  StencilProperty::Action fail_action, StencilProperty::Action zfail_action,
                  unsigned long refval, unsigned long funcmask, unsigned long writemask) :
  _value(mode,pass_action,fail_action,zfail_action,refval,funcmask,writemask)
{
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::set_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilTransition::
set_mode(StencilProperty::Mode mode) {
  _value.set_mode(mode);
  state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::get_mode
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Mode StencilTransition::
get_mode() const {
  return _value.get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::set_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void StencilTransition::
set_pass_action(StencilProperty::Action action) {
  _value.set_pass_action(action);
  state_changed();
}

INLINE void StencilTransition::
set_fail_action(StencilProperty::Action action) {
  _value.set_fail_action(action);
  state_changed();
}

INLINE void StencilTransition::
set_zfail_action(StencilProperty::Action action) {
  _value.set_zfail_action(action);
  state_changed();
}

INLINE void StencilTransition::
set_reference_value(unsigned long v) {
  _value.set_reference_value(v);
  state_changed();
}

INLINE void StencilTransition::
set_func_mask(unsigned long m) {
  _value.set_func_mask(m);
  state_changed();
}

INLINE void StencilTransition::
set_write_mask(unsigned long m) {
  _value.set_write_mask(m);
  state_changed();
}

INLINE void StencilTransition::
set_stencil_state(StencilProperty::Mode mode, StencilProperty::Action pass_action, 
                  StencilProperty::Action fail_action, StencilProperty::Action zfail_action,
                  unsigned long refval, unsigned long refmask, unsigned long writemask) {
 _value.set_stencil_state(mode,pass_action,fail_action,zfail_action,
                          refval,refmask,writemask);
 state_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: StencilTransition::get_action
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE StencilProperty::Action StencilTransition::
get_pass_action() const {
  return _value.get_pass_action();
}

INLINE StencilProperty::Action StencilTransition::
get_fail_action() const {
  return _value.get_fail_action();
}

INLINE StencilProperty::Action StencilTransition::
get_zfail_action() const {
  return _value.get_zfail_action();
}

INLINE unsigned long StencilTransition::
get_reference_value() const {
  return _value.get_reference_value();
}

INLINE unsigned long StencilTransition::
get_func_mask() const {
  return _value.get_func_mask();
}

INLINE unsigned long StencilTransition::
get_write_mask() const {
  return _value.get_write_mask();
}





