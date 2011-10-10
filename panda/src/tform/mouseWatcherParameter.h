// Filename: mouseWatcherParameter.h
// Created by:  drose (06Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef MOUSEWATCHERPARAMETER_H
#define MOUSEWATCHERPARAMETER_H

#include "pandabase.h"

#include "buttonHandle.h"
#include "modifierButtons.h"
#include "textEncoder.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherParameter
// Description : This is sent along as a parameter to most events
//               generated for a region to indicate the mouse and
//               button state for the event.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TFORM MouseWatcherParameter {
public:
  INLINE MouseWatcherParameter();
  INLINE MouseWatcherParameter(const MouseWatcherParameter &other);
  INLINE void operator = (const MouseWatcherParameter &other);
  INLINE ~MouseWatcherParameter();

  INLINE void set_button(const ButtonHandle &button);
  INLINE void set_keyrepeat(bool flag);
  INLINE void set_keycode(int keycode);
  INLINE void set_candidate(const wstring &candidate_string,
                            size_t highlight_start, 
                            size_t higlight_end,
                            size_t cursor_pos);
  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  INLINE void set_mouse(const LPoint2 &mouse);
  INLINE void set_outside(bool flag);

PUBLISHED:
  INLINE bool has_button() const;
  INLINE ButtonHandle get_button() const;
  INLINE bool is_keyrepeat() const;

  INLINE bool has_keycode() const;
  INLINE int get_keycode() const;

  INLINE bool has_candidate() const;

public:
  INLINE const wstring &get_candidate_string() const;

PUBLISHED:
  INLINE string get_candidate_string_encoded() const;
  INLINE string get_candidate_string_encoded(TextEncoder::Encoding encoding) const;
  INLINE size_t get_highlight_start() const;
  INLINE size_t get_highlight_end() const;
  INLINE size_t get_cursor_pos() const;

  INLINE const ModifierButtons &get_modifier_buttons() const;

  INLINE bool has_mouse() const;
  INLINE const LPoint2 &get_mouse() const;

  INLINE bool is_outside() const;

  void output(ostream &out) const;

public:
  ButtonHandle _button;
  short _keycode;
  wstring _candidate_string;
  size_t _highlight_start;
  size_t _highlight_end;
  size_t _cursor_pos;
  ModifierButtons _mods;
  LPoint2 _mouse;

  enum Flags {
    F_has_button    = 0x001,
    F_has_mouse     = 0x002,
    F_is_outside    = 0x004,
    F_has_keycode   = 0x008,
    F_has_candidate = 0x010,
    F_is_keyrepeat  = 0x020,
  };
  int _flags;
};

INLINE ostream &operator << (ostream &out, const MouseWatcherParameter &parm);

#include "mouseWatcherParameter.I"

#endif
