/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rocketInputHandler.h
 * @author rdb
 * @date 2011-12-20
 */

#ifndef ROCKETINPUTHANDLER_H
#define ROCKETINPUTHANDLER_H

#include "config_rocket.h"
#include "dataNode.h"
#include "buttonHandle.h"

namespace Rocket {
  namespace Core {
    class Context;
  }
}

/**
 * DataNode that listens for keyboard and mouse events and passes them to
 * libRocket.
 */
class EXPCL_ROCKET RocketInputHandler : public DataNode {
PUBLISHED:
  RocketInputHandler(const std::string &name = std::string());
  virtual ~RocketInputHandler();

  static int get_rocket_key(const ButtonHandle handle);

public:
  void update_context(Rocket::Core::Context *context, int xoffs, int yoffs);

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  Mutex _lock;

  // inputs
  int _pixel_xy_input;
  int _button_events_input;

  LVecBase2 _mouse_xy;
  bool _mouse_xy_changed;
  int _modifiers;
  int _wheel_delta;
  typedef pmap<int, bool> ButtonActivityMap;
  ButtonActivityMap _mouse_buttons;
  ButtonActivityMap _keys;
  pvector<int> _repeated_keys;
  pvector<short> _text_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "RocketInputHandler",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
