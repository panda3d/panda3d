/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgButton.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef PGBUTTON_H
#define PGBUTTON_H

#include "pandabase.h"

#include "pgItem.h"
#include "pgButtonNotify.h"
#include "nodePath.h"
#include "pset.h"

/**
 * This is a particular kind of PGItem that is specialized to behave like a
 * normal button object.  It keeps track of its own state, and handles mouse
 * events sensibly.
 */
class EXPCL_PANDA_PGUI PGButton : public PGItem {
PUBLISHED:
  explicit PGButton(const std::string &name);
  virtual ~PGButton();

protected:
  PGButton(const PGButton &copy);

public:
  virtual PandaNode *make_copy() const;

  virtual void enter_region(const MouseWatcherParameter &param);
  virtual void exit_region(const MouseWatcherParameter &param);
  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void release(const MouseWatcherParameter &param, bool background);

  virtual void click(const MouseWatcherParameter &param);

  INLINE void set_notify(PGButtonNotify *notify);
  INLINE PGButtonNotify *get_notify() const;

PUBLISHED:
  enum State {
    S_ready = 0,
    S_depressed,
    S_rollover,
    S_inactive
  };

  void setup(const std::string &label, PN_stdfloat bevel = 0.1f);
  INLINE void setup(const NodePath &ready);
  INLINE void setup(const NodePath &ready, const NodePath &depressed);
  INLINE void setup(const NodePath &ready, const NodePath &depressed,
                    const NodePath &rollover);
  void setup(const NodePath &ready, const NodePath &depressed,
             const NodePath &rollover, const NodePath &inactive);

  virtual void set_active(bool active);

  bool add_click_button(const ButtonHandle &button);
  bool remove_click_button(const ButtonHandle &button);
  bool has_click_button(const ButtonHandle &button);

  INLINE bool is_button_down();

  INLINE static std::string get_click_prefix();
  INLINE std::string get_click_event(const ButtonHandle &button) const;
  MAKE_PROPERTY(click_prefix, get_click_prefix);

private:
  typedef pset<ButtonHandle> Buttons;
  Buttons _click_buttons;

  bool _button_down;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGItem::init_type();
    register_type(_type_handle, "PGButton",
                  PGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "pgButton.I"

#endif
