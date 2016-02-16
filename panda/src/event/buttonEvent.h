/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonEvent.h
 * @author drose
 * @date 2000-03-01
 */

#ifndef BUTTONEVENT_H
#define BUTTONEVENT_H

#include "pandabase.h"

#include "buttonHandle.h"
#include "clockObject.h"
#include "modifierButtons.h"

class Datagram;
class DatagramIterator;

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
class EXPCL_PANDA_EVENT ButtonEvent {
public:
  enum Type {
    // T_down and T_up represent a button changing state
    // correspondingly.  T_resume_down is a special event that is only
    // thrown when focus is returned to a window and a button is
    // detected as being held down at that point; it indicates that
    // the button should be considered down now (if it wasn't
    // already), but it didn't just get pressed down at this moment,
    // it was depressed some time ago.  It's mainly used for correct
    // tracking of modifier keys like shift and control, and can be
    // ignored for other keys.
    T_down,
    T_resume_down,
    T_up,

    // T_repeat is sent for each a keyrepeat event generated by the
    // system, for a button that is continually held down.  If you
    // want to respect keyrepeat, treat T_down and T_repeat
    // equivalently.
    T_repeat,

    // T_keystroke is a special keystroke event, and is sent along
    // with a Unicode keycode value, not a ButtonHandle.
    T_keystroke,

    // T_candidate is used to indicate that the user is using the IME
    // and has in the process of selecting some possible text to type
    // from a menu.
    T_candidate,

    // T_move is used to indicate that the mouse has moved within the
    // current region. Button drag mode needs this, others may ignore
    // this event
    T_move,

    // T_raw_down is usually sent together with T_down, except that
    // this is the original, untransformed scan key sent by the keyboard.
    // It is not altered by modifier keys and acts as if the user is
    // using the US (qwerty) keyboard layout.
    T_raw_down,
    T_raw_up,
  };

  INLINE ButtonEvent();
  INLINE ButtonEvent(ButtonHandle button, Type type, double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE ButtonEvent(short keycode, double time = ClockObject::get_global_clock()->get_frame_time());
  INLINE ButtonEvent(const wstring &candidate_string, size_t highlight_start, 
                     size_t highlight_end, size_t cursor_pos);
  INLINE ButtonEvent(const ButtonEvent &copy);
  INLINE void operator = (const ButtonEvent &copy);

  INLINE bool operator == (const ButtonEvent &other) const;
  INLINE bool operator != (const ButtonEvent &other) const;
  INLINE bool operator < (const ButtonEvent &other) const;

  INLINE bool update_mods(ModifierButtons &mods) const;

  void output(ostream &out) const;

  void write_datagram(Datagram &dg) const;
  void read_datagram(DatagramIterator &scan);

public:
  // _button will be filled in if type is T_down, T_resume_down, or
  // T_up.
  ButtonHandle _button;

  // _keycode will be filled in if type is T_keystroke.  It will be
  // the Unicode character that was typed.
  short _keycode;

  // _candidate_string will be filled in if type is T_candidate.
  wstring _candidate_string;
  size_t _highlight_start;
  size_t _highlight_end;
  size_t _cursor_pos;

  // This is the type of the button event (see above).
  Type _type;

  // This is the time the event occurred, as recorded from the OS if
  // that information is available.  It is in seconds elapsed from an
  // arbitrary epoch, and it matches the time reported by
  // ClockObject::get_global_clock().
  double _time;
};

INLINE ostream &operator << (ostream &out, const ButtonEvent &be) {
  be.output(out);
  return out;
}

#include "buttonEvent.I"

#endif

