// Filename: stencilProperty.h
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

#ifndef STENCILPROPERTY_H
#define STENCILPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : StencilProperty
// Description : Defines how we can write to and stencil via the
//               stencil buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StencilProperty {
public:
  enum Mode {
    M_none,             // No stencil test, and no write to stencil buffer.
    M_never,            // Never draw.
    M_less,             // incoming < stored
    M_equal,            // incoming == stored
    M_less_equal,       // incoming <= stored
    M_greater,          // incoming > stored
    M_not_equal,        // incoming != stored
    M_greater_equal,    // incoming >= stored
    M_always            // Always draw.
  };

  enum Action {    
    A_keep,
    A_zero,
    A_replace,
    A_increment,        // incr maxval will wrap around to 0
    A_decrement,        // decr 0 will wrap around to all 1's
    A_increment_clamp,  // cap value to max stencil val
    A_decrement_clamp,  // cap value at zero
    A_invert
  };

public:
  INLINE StencilProperty(Mode mode,Action pass_action,Action fail_action,Action zfail_action,
                  unsigned long refval, unsigned long funcmask, unsigned long writemask);

  INLINE void set_mode(Mode mode) { _mode = mode; };
  INLINE Mode get_mode() const { return _mode; };

  INLINE void set_pass_action(Action action) { _pass_action = action; };
  INLINE Action get_pass_action() const { return _pass_action; };

  INLINE void set_fail_action(Action action) { _fail_action = action; };
  INLINE Action get_fail_action() const { return _fail_action; };

  INLINE void set_zfail_action(Action action) { _zfail_action = action; };
  INLINE Action get_zfail_action() const  { return _zfail_action; };

  INLINE void set_reference_value(unsigned long v) { _refval = v; };
  INLINE unsigned long get_reference_value() const { return _refval; };

  INLINE void set_func_mask(unsigned long m) { _funcmask = m; };
  INLINE unsigned long get_func_mask() const { return _funcmask; };

  INLINE void set_write_mask(unsigned long m) { _writemask = m; };
  INLINE unsigned long get_write_mask() const { return _writemask; };

  INLINE void set_stencil_state(Mode mode, Action pass_action, Action fail_action, Action zfail_action,
                                unsigned long refval, unsigned long funcmask, unsigned long writemask) {
        _mode = mode;
        _pass_action = pass_action;
        _fail_action = fail_action;
        _zfail_action = zfail_action;
        _refval = refval;
        _funcmask = funcmask;
        _writemask = writemask;
  };

  INLINE int compare_to(const StencilProperty &other) const;
  void output(ostream &out) const;

private:
  Mode _mode;
  Action _pass_action,_fail_action,_zfail_action;
  unsigned long _refval,_funcmask,_writemask;
};

ostream &operator << (ostream &out, StencilProperty::Mode mode);
ostream &operator << (ostream &out, StencilProperty::Action action);

INLINE ostream &operator << (ostream &out, const StencilProperty &prop) {
  prop.output(out);
  return out;
}

#include "stencilProperty.I"

#endif
