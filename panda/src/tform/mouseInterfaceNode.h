/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseInterfaceNode.h
 * @author drose
 * @date 2004-06-11
 */

#ifndef MOUSEINTERFACENODE_H
#define MOUSEINTERFACENODE_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"

class ButtonEventList;

/**
 * This is the base class for some classes that monitor the mouse and keyboard
 * input and perform some action due to their state.
 *
 * It collects together some common interface; in particular, the
 * require_button() and related methods.
 */
class EXPCL_PANDA_TFORM MouseInterfaceNode : public DataNode {
public:
  explicit MouseInterfaceNode(const std::string &name);
  virtual ~MouseInterfaceNode();

PUBLISHED:
  void require_button(const ButtonHandle &button, bool is_down);
  void clear_button(const ButtonHandle &button);
  void clear_all_buttons();

protected:
  void watch_button(const ButtonHandle &button);
  const ButtonEventList *check_button_events(const DataNodeTransmit &input,
                                             bool &required_buttons_match);

  INLINE bool is_down(ButtonHandle button) const;

private:
  ModifierButtons _current_button_state;
  ModifierButtons _watched_buttons;
  ModifierButtons _required_buttons_mask;
  ModifierButtons _required_buttons_state;

private:
  int _button_events_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "MouseInterfaceNode",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "mouseInterfaceNode.I"

#endif
