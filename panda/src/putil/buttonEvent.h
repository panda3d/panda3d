// Filename: buttonEvent.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include <pandabase.h>

#include "buttonHandle.h"
#include "modifierButtons.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ButtonEvent
// Description : Records a transition of one button from up to down or
//               vice-versa, as well as an optional recording of some
//               ModifierButtons that were being tracked at the time.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEvent {
public:
  INLINE ButtonEvent();
  INLINE ButtonEvent(ButtonHandle button, bool down);
  INLINE ButtonEvent(ButtonHandle button, bool down, ModifierButtons mods);
  INLINE ButtonEvent(const ButtonEvent &copy);
  INLINE void operator = (const ButtonEvent &copy);

  INLINE bool operator == (const ButtonEvent &other) const;
  INLINE bool operator != (const ButtonEvent &other) const;
  INLINE bool operator < (const ButtonEvent &other) const;

  void output(ostream &out) const;

  ButtonHandle _button;
  bool _down;
  const ModifierButtons _mods;
};

INLINE ostream &operator << (ostream &out, const ButtonEvent &be) {
  be.output(out);
  return out;
}

#include "buttonEvent.I"

#endif

