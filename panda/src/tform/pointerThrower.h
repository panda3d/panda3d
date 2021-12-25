/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pointerThrower.h
 * @author rdb
 * @date 2021-12-22
 */

#ifndef POINTERTHROWER_H
#define POINTERTHROWER_H

#include "pandabase.h"

#include "dataNode.h"
#include "modifierButtons.h"
#include "buttonEventList.h"
#include "pvector.h"
#include "pmap.h"
#include "eventParameter.h"

/**
 * Throws pointer events.
 *
 * This is a DataNode which is intended to be parented to the data graph below
 * a device which is generating a sequence of pointer events, like a
 * MouseAndKeyboard device.
 */
class EXPCL_PANDA_TFORM PointerThrower : public DataNode {
PUBLISHED:
  explicit PointerThrower(const std::string &name,
                          const std::string &prefix = std::string());
  ~PointerThrower();

public:
  INLINE void set_down_event(const std::string &pointer_down_event);
  INLINE const std::string &get_down_event() const;
  INLINE void set_up_event(const std::string &pointer_up_event);
  INLINE const std::string &get_up_event() const;
  INLINE void set_move_event(const std::string &pointer_move_event);
  INLINE const std::string &get_move_event() const;

  INLINE void set_prefix(const std::string &prefix);
  INLINE const std::string &get_prefix() const;

  void add_parameter(const EventParameter &obj);
  int get_num_parameters() const;
  EventParameter get_parameter(size_t n) const;

PUBLISHED:
  MAKE_PROPERTY(down_event, get_down_event, set_down_event);
  MAKE_PROPERTY(up_event, get_up_event, set_up_event);
  MAKE_PROPERTY(move_event, get_move_event, set_move_event);

  MAKE_PROPERTY(prefix, get_prefix, set_prefix);
  MAKE_SEQ_PROPERTY(parameters, get_num_parameters, get_parameter);

public:
  virtual void write(std::ostream &out, int indent_level = 0) const;

private:
  void do_general_event(const PointerEvent &pointer_event);

private:
  std::string _down_event;
  std::string _up_event;
  std::string _move_event;
  std::string _prefix;

  typedef pvector<EventParameter> ParameterList;
  ParameterList _parameters;

  pset<int> _active_ids;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _pointer_events_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "PointerThrower",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pointerThrower.I"

#endif
