// Filename: stencilProperty.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef STENCILPROPERTY_H
#define STENCILPROPERTY_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : StencilProperty
// Description : Defines how we can write to and stencil via the
//               stencil buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA StencilProperty {
public:
  enum Mode {
    M_none,             // No stencil test, and no write to stencil buffer.
    M_never,            // Never draw.
    M_less,		// incoming < stored
    M_equal,		// incoming == stored
    M_less_equal, 	// incoming <= stored
    M_greater, 		// incoming > stored
    M_not_equal, 	// incoming != stored
    M_greater_equal,	// incoming >= stored
    M_always            // Always draw.
  };

  enum Action {    // What to do to the stencil when the above test fails.
    A_keep,
    A_zero,
    A_replace,
    A_increment,
    A_decrement,
    A_invert
  };

public:
  INLINE StencilProperty(Mode mode, Action action);

  INLINE void set_mode(Mode mode);
  INLINE Mode get_mode() const;

  INLINE void set_action(Action action);
  INLINE Action get_action() const;

  INLINE int compare_to(const StencilProperty &other) const;
  void output(ostream &out) const;

private:
  Mode _mode;
  Action _action;
};

ostream &operator << (ostream &out, StencilProperty::Mode mode);
ostream &operator << (ostream &out, StencilProperty::Action action);

INLINE ostream &operator << (ostream &out, const StencilProperty &prop) {
  prop.output(out);
  return out;
}

#include "stencilProperty.I"

#endif
