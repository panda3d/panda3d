// Filename: buttonHandle.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BUTTONHANDLE_H
#define BUTTONHANDLE_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ButtonHandle
// Description : A ButtonHandle represents a single button from any
//               device, including keyboard buttons and mouse buttons
//               (but see KeyboardButton and MouseButton).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ButtonHandle {
public:
  INLINE ButtonHandle();
  INLINE ButtonHandle(const ButtonHandle &copy);
 
  INLINE bool operator == (const ButtonHandle &other) const;
  INLINE bool operator != (const ButtonHandle &other) const;
  INLINE bool operator < (const ButtonHandle &other) const;

  string get_name() const;
  INLINE bool has_ascii_equivalent() const;
  INLINE char get_ascii_equivalent() const;

  INLINE static ButtonHandle none();

private:
  int _index;
  static ButtonHandle _none;

friend class ButtonRegistry;
};

// It's handy to be able to output a ButtonHandle directly, and see the
// button name.
INLINE ostream &operator << (ostream &out, ButtonHandle button) {
  return out << button.get_name();
}

#include "buttonHandle.I"

#endif

