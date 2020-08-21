/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonThrower.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef BUTTONTHROWER_H
#define BUTTONTHROWER_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"
#include "buttonEventList.h"
#include "pvector.h"
#include "pmap.h"
#include "eventParameter.h"

/**
 * Throws Panda Events for button down/up events generated within the data
 * graph.
 *
 * This is a DataNode which is intended to be parented to the data graph below
 * a device which is generating a sequence of button events, like a
 * MouseAndKeyboard device.  It simply takes each button it finds and throws a
 * corresponding event based on the button name via the throw_event() call.
 */
class EXPCL_PANDA_TFORM ButtonThrower : public DataNode {
PUBLISHED:
  explicit ButtonThrower(const std::string &name);
  ~ButtonThrower();

  INLINE void set_button_down_event(const std::string &button_down_event);
  INLINE const std::string &get_button_down_event() const;
  INLINE void set_button_up_event(const std::string &button_up_event);
  INLINE const std::string &get_button_up_event() const;
  INLINE void set_button_repeat_event(const std::string &button_repeat_event);
  INLINE const std::string &get_button_repeat_event() const;
  INLINE void set_keystroke_event(const std::string &keystroke_event);
  INLINE const std::string &get_keystroke_event() const;
  INLINE void set_candidate_event(const std::string &candidate_event);
  INLINE const std::string &get_candidate_event() const;
  INLINE void set_move_event(const std::string &move_event);
  INLINE const std::string &get_move_event() const;
  INLINE void set_raw_button_down_event(const std::string &raw_button_down_event);
  INLINE const std::string &get_raw_button_down_event() const;
  INLINE void set_raw_button_up_event(const std::string &raw_button_up_event);
  INLINE const std::string &get_raw_button_up_event() const;
  MAKE_PROPERTY(button_down_event, get_button_down_event, set_button_down_event);
  MAKE_PROPERTY(button_up_event, get_button_up_event, set_button_up_event);
  MAKE_PROPERTY(button_repeat_event, get_button_repeat_event, set_button_repeat_event);
  MAKE_PROPERTY(keystroke_event, get_keystroke_event, set_keystroke_event);
  MAKE_PROPERTY(candidate_event, get_candidate_event, set_candidate_event);
  MAKE_PROPERTY(move_event, get_move_event, set_move_event);
  MAKE_PROPERTY(raw_button_down_event, get_raw_button_down_event, set_raw_button_down_event);
  MAKE_PROPERTY(raw_button_up_event, get_raw_button_up_event, set_raw_button_up_event);

  INLINE void set_prefix(const std::string &prefix);
  INLINE const std::string &get_prefix() const;
  INLINE void set_specific_flag(bool specific_flag);
  INLINE bool get_specific_flag() const;
  MAKE_PROPERTY(prefix, get_prefix, set_prefix);
  MAKE_PROPERTY(specific_flag, get_specific_flag, set_specific_flag);

  INLINE void set_time_flag(bool time_flag);
  INLINE bool get_time_flag() const;
  MAKE_PROPERTY(time_flag, get_time_flag, set_time_flag);

  void add_parameter(const EventParameter &obj);
  int get_num_parameters() const;
  EventParameter get_parameter(int n) const;
  MAKE_SEQ(get_parameters, get_num_parameters, get_parameter);
  MAKE_SEQ_PROPERTY(parameters, get_num_parameters, get_parameter);

  INLINE const ModifierButtons &get_modifier_buttons() const;
  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  MAKE_PROPERTY(modifier_buttons, get_modifier_buttons, set_modifier_buttons);

  INLINE void set_throw_buttons_active(bool flag);
  INLINE bool get_throw_buttons_active() const;
  MAKE_PROPERTY(throw_buttons_active, get_throw_buttons_active, set_throw_buttons_active);

  bool add_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool remove_throw_button(const ModifierButtons &mods, const ButtonHandle &button);
  bool has_throw_button(const ModifierButtons &mods, const ButtonHandle &button) const;
  bool has_throw_button(const ButtonHandle &button) const;
  void clear_throw_buttons();

public:
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  void do_specific_event(const std::string &event_name, double time);
  void do_general_event(const ButtonEvent &button_event,
                        const std::string &event_name);

private:
  std::string _button_down_event;
  std::string _button_up_event;
  std::string _button_repeat_event;
  std::string _keystroke_event;
  std::string _candidate_event;
  std::string _move_event;
  std::string _raw_button_up_event;
  std::string _raw_button_down_event;
  bool _specific_flag;
  std::string _prefix;
  bool _time_flag;
  ModifierButtons _mods;

  typedef pvector<EventParameter> ParameterList;
  ParameterList _parameters;

  typedef pvector<ModifierButtons> ThrowButtonDef;
  typedef pmap<ButtonHandle, ThrowButtonDef> ThrowButtons;
  ThrowButtons _throw_buttons;
  bool _throw_buttons_active;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _button_events_input;

  // outputs
  int _button_events_output;
  PT(ButtonEventList) _button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "ButtonThrower",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "buttonThrower.I"

#endif
