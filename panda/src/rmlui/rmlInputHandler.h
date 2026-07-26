/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rmlInputHandler.h
 * @author rdb
 * @date 2011-12-20
 */

#ifndef RML_INPUT_HANDLER_H
#define RML_INPUT_HANDLER_H

#include "config_rmlui.h"
#include "dataNode.h"
#include "buttonHandle.h"

namespace Rml {
  class Context;
}

/**
 * DataNode that listens for keyboard and mouse events and passes them to
 * RmlUi.  Attach to the scene graph below mouseWatcher.
 */
class EXPCL_PANDARMLUI RmlInputHandler : public DataNode {
PUBLISHED:
  RmlInputHandler(const std::string &name = std::string());
  virtual ~RmlInputHandler();

  static int get_rml_key(const ButtonHandle handle);

public:
  void update_context(Rml::Context *context, int xoffs, int yoffs);

protected:
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  Mutex _lock;

  int _pixel_xy_input;
  int _button_events_input;

  LVecBase2 _mouse_xy;
  bool _mouse_xy_changed = false;
  // Tracks whether the pointer is currently inside the window/region.  When it
  // leaves (pixel_xy stops being transmitted), we queue a single mouse-leave so
  // RmlUi clears hover/active state instead of leaving it latched.
  bool _mouse_in_window = false;
  bool _mouse_left = false;
  int _modifiers = 0;
  float _wheel_delta = 0.0f;

  // Ordered event queues — preserves press/release sequence within a frame.
  // Using pvector<pair> instead of a map so that a fast tap (T_down + T_up
  // in the same data-graph tick) delivers both ProcessKeyDown and ProcessKeyUp.
  pvector<std::pair<int, bool>> _key_events;
  pvector<std::pair<int, bool>> _mouse_button_events;
  pvector<int> _repeated_keys;
  pvector<int> _text_input;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() { init_type(); return get_class_type(); }

private:
  static TypeHandle _type_handle;
};

#endif
