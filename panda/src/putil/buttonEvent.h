// Filename: buttonEvent.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include <pandabase.h>

#include "buttonHandle.h"

////////////////////////////////////////////////////////////////////
//       Class : ButtonEvent
// Description : Records a transition of one button from up to down or
//               vice-versa.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonEvent {
public:
  INLINE ButtonEvent();
  INLINE ButtonEvent(ButtonHandle button, bool down);
  INLINE ButtonEvent(const ButtonEvent &copy);
  INLINE void operator = (const ButtonEvent &copy);

  INLINE bool operator == (const ButtonEvent &other) const;
  INLINE bool operator != (const ButtonEvent &other) const;
  INLINE bool operator < (const ButtonEvent &other) const;

  void output(ostream &out) const;

  ButtonHandle _button;
  bool _down;
};

INLINE ostream &operator << (ostream &out, const ButtonEvent &be) {
  be.output(out);
  return out;
}

#include "buttonEvent.I"

#endif

