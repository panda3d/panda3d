/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file modifierButtons.h
 * @author drose
 * @date 2000-03-01
 */

#ifndef MODIFIERBUTTONS_H
#define MODIFIERBUTTONS_H

#include "pandabase.h"

#include "buttonHandle.h"
#include "pointerToArray.h"

/**
 * This class monitors the state of a number of individual buttons and tracks
 * whether each button is known to be down or up.
 */
class EXPCL_PANDA_PUTIL ModifierButtons {
PUBLISHED:
  ModifierButtons();
  ModifierButtons(const ModifierButtons &copy);
  ~ModifierButtons();
  INLINE void operator = (const ModifierButtons &copy);

  INLINE bool operator == (const ModifierButtons &other) const;
  INLINE bool operator != (const ModifierButtons &other) const;
  INLINE bool operator < (const ModifierButtons &other) const;

  INLINE ModifierButtons operator & (const ModifierButtons &other) const;
  INLINE ModifierButtons operator | (const ModifierButtons &other) const;

  void operator &= (const ModifierButtons &other);
  void operator |= (const ModifierButtons &other);

  void set_button_list(const ModifierButtons &other);

  bool matches(const ModifierButtons &other) const;

  bool add_button(ButtonHandle button);
  bool has_button(ButtonHandle button) const;
  bool remove_button(ButtonHandle button);

  INLINE int get_num_buttons() const;
  INLINE ButtonHandle get_button(int index) const;
  MAKE_SEQ(get_buttons, get_num_buttons, get_button);
  MAKE_SEQ_PROPERTY(buttons, get_num_buttons, get_button);

  bool button_down(ButtonHandle button);
  bool button_up(ButtonHandle button);
  INLINE void all_buttons_up();

  bool is_down(ButtonHandle button) const;
  INLINE bool is_down(int index) const;
  INLINE bool is_any_down() const;

  std::string get_prefix() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

private:
  void modify_button_list();

  PTA(ButtonHandle) _button_list;
  typedef unsigned long BitmaskType;
  BitmaskType _state;
};

INLINE std::ostream &operator << (std::ostream &out, const ModifierButtons &mb) {
  mb.output(out);
  return out;
}

#include "modifierButtons.I"

#endif
