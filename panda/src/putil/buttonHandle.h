/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonHandle.h
 * @author drose
 * @date 2000-03-01
 */

#ifndef BUTTONHANDLE_H
#define BUTTONHANDLE_H

#include "pandabase.h"
#include "typeHandle.h"
#include "register_type.h"

/**
 * A ButtonHandle represents a single button from any device, including
 * keyboard buttons and mouse buttons (but see KeyboardButton and
 * MouseButton).
 */
class EXPCL_PANDA_PUTIL ButtonHandle final {
PUBLISHED:
  // The default constructor must do nothing, because we can't guarantee
  // ordering of static initializers.  If the constructor tried to initialize
  // its value, it  might happen after the value had already been set
  // previously by another static initializer!
  INLINE ButtonHandle() = default;
  constexpr ButtonHandle(int index);
  ButtonHandle(const std::string &name);

PUBLISHED:
  INLINE bool operator == (const ButtonHandle &other) const;
  INLINE bool operator != (const ButtonHandle &other) const;
  INLINE bool operator < (const ButtonHandle &other) const;
  INLINE bool operator <= (const ButtonHandle &other) const;
  INLINE bool operator > (const ButtonHandle &other) const;
  INLINE bool operator >= (const ButtonHandle &other) const;
  INLINE int compare_to(const ButtonHandle &other) const;
  INLINE size_t get_hash() const;

  std::string get_name() const;
  INLINE bool has_ascii_equivalent() const;
  INLINE char get_ascii_equivalent() const;

  ButtonHandle get_alias() const;

  INLINE bool matches(const ButtonHandle &other) const;

  constexpr int get_index() const;
  INLINE void output(std::ostream &out) const;
  constexpr static ButtonHandle none() { return ButtonHandle(0); }

  INLINE operator bool () const;

  MAKE_PROPERTY(index, get_index);
  MAKE_PROPERTY(name, get_name);
  MAKE_PROPERTY2(ascii_equivalent, has_ascii_equivalent,
                                   get_ascii_equivalent);
  MAKE_PROPERTY(alias, get_alias);

private:
  int _index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "ButtonHandle");
  }

private:
  static TypeHandle _type_handle;

friend class ButtonRegistry;
};

// It's handy to be able to output a ButtonHandle directly, and see the button
// name.
INLINE std::ostream &operator << (std::ostream &out, ButtonHandle button) {
  button.output(out);
  return out;
}

#include "buttonHandle.I"

#endif
