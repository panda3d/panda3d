// Filename: modifierButtons.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MODIFIERBUTTONS_H
#define MODIFIERBUTTONS_H

#include <pandabase.h>

#include "buttonHandle.h"
#include "pointerToArray.h"
#include "buttonEvent.h"

////////////////////////////////////////////////////////////////////
//       Class : ModifierButtons
// Description : This class monitors the state of a number of
//               individual buttons and tracks whether each button is
//               known to be down or up.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModifierButtons {
PUBLISHED:
  ModifierButtons();
  ModifierButtons(const ModifierButtons &copy);
  ~ModifierButtons();
  INLINE void operator = (const ModifierButtons &copy);

  INLINE bool operator == (const ModifierButtons &other) const;
  INLINE bool operator != (const ModifierButtons &other) const;
  INLINE bool operator < (const ModifierButtons &other) const;

  bool add_button(ButtonHandle button);
  bool has_button(ButtonHandle button) const;
  bool remove_button(ButtonHandle button);

  INLINE int get_num_buttons() const;
  INLINE ButtonHandle get_button(int index) const;

  bool button_down(ButtonHandle button);
  bool button_up(ButtonHandle button);
  INLINE bool add_event(const ButtonEvent &event);
  INLINE void all_buttons_up();

  bool is_down(ButtonHandle button) const;
  INLINE bool is_down(int index) const;
  INLINE bool is_any_down() const;

  void output(ostream &out) const;
  void write(ostream &out) const;

private:
  void modify_button_list();

  PTA(ButtonHandle) _button_list;
  typedef unsigned long BitmaskType;
  BitmaskType _state;
};

INLINE ostream &operator << (ostream &out, const ModifierButtons &mb) {
  mb.output(out);
  return out;
}

#include "modifierButtons.I"

#endif

