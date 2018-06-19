/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseInterfaceNode.cxx
 * @author drose
 * @date 2004-06-11
 */

#include "trackball.h"
#include "buttonEvent.h"
#include "buttonEventList.h"
#include "dataNodeTransmit.h"
#include "mouseData.h"

TypeHandle MouseInterfaceNode::_type_handle;

/**
 *
 */
MouseInterfaceNode::
MouseInterfaceNode(const std::string &name) :
  DataNode(name)
{
  _button_events_input = define_input("button_events", ButtonEventList::get_class_type());
}

/**
 *
 */
MouseInterfaceNode::
~MouseInterfaceNode() {
}

/**
 * Indicates that the indicated button must be in the required state (either
 * up or down) in order for this particular MouseInterfaceNode to do anything.
 * For instance, this may be called to make a Trackball object respect mouse
 * input only when the control key is held down.
 */
void MouseInterfaceNode::
require_button(const ButtonHandle &button, bool is_down) {
  _required_buttons_mask.add_button(button);
  _required_buttons_state.set_button_list(_required_buttons_mask);
  _current_button_state.set_button_list(_required_buttons_mask);

  _required_buttons_mask.button_down(button);
  if (is_down) {
    _required_buttons_state.button_down(button);
  } else {
    _required_buttons_state.button_up(button);
  }
}

/**
 * Removes any requirement on the indicated button set by an earlier call to
 * require_button().
 */
void MouseInterfaceNode::
clear_button(const ButtonHandle &button) {
  _required_buttons_mask.button_up(button);
  _required_buttons_state.button_up(button);

  // The _required_buttons_mask and state must always keep the buttons that
  // are listed in _watched_buttons.

  if (!_watched_buttons.has_button(button)) {
    _required_buttons_mask.remove_button(button);
    _required_buttons_state.set_button_list(_required_buttons_mask);
    _current_button_state.set_button_list(_required_buttons_mask);
  }
}

/**
 * Removes all requirements on buttons set by an earlier call to
 * require_button().
 */
void MouseInterfaceNode::
clear_all_buttons() {
  _required_buttons_mask.all_buttons_up();
  _required_buttons_state.all_buttons_up();

  _required_buttons_mask.set_button_list(_watched_buttons);
  _required_buttons_state.set_button_list(_watched_buttons);
  _current_button_state.set_button_list(_watched_buttons);
}

/**
 * Indicates that the derived class would like to know the state of the given
 * button.
 */
void MouseInterfaceNode::
watch_button(const ButtonHandle &button) {
  _watched_buttons.add_button(button);

  // We also add the button to _required_buttons_mask and
  // _required_buttons_state, but it's left 'up' in these two.
  _required_buttons_mask.add_button(button);
  _required_buttons_state.set_button_list(_required_buttons_mask);
  _current_button_state.set_button_list(_required_buttons_mask);
}

/**
 * Gets the button events from the data graph and updates the ModifierButtons
 * objects appropriately.
 *
 * Sets required_buttons_match to true if the required combination of buttons
 * are being held down, or false otherwise.
 *
 * The return value is the list of button events processed this frame, or NULL
 * if there are no button events.
 */
const ButtonEventList *MouseInterfaceNode::
check_button_events(const DataNodeTransmit &input,
                    bool &required_buttons_match) {
  const ButtonEventList *button_events = nullptr;

  if (input.has_data(_button_events_input)) {
    DCAST_INTO_R(button_events, input.get_data(_button_events_input).get_ptr(), nullptr);
    button_events->update_mods(_current_button_state);
  }

  required_buttons_match =
    (_current_button_state & _required_buttons_mask) == _required_buttons_state;

  return button_events;
}
