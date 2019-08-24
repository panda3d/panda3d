/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file touchTrackball.h
 * @author D. Lawrence
 * @date 2019-08-16
 */

#ifndef TOUCH_TRACKBALL_H
#define TOUCH_TRACKBALL_H

#include "pandabase.h"

#include "mouseInterfaceNode.h"
#include "nodePath.h"
#include "modifierButtons.h"
#include "luse.h"
#include "transformState.h"

BEGIN_PUBLISH
// At the time of writing this is the only class in all of Panda that utilizes
// multitouch gestures. We'll want to come up with a more generalized way of
// detecting them that applications can utilize.
enum class GestureState {
  none,
  one_drag,
  two_drag,
  pinch
};
END_PUBLISH

/**
 * This functions similarly to Trackball, but uses touch controls instead.
 */
class EXPCL_PANDA_TFORM TouchTrackball : public Trackball {
PUBLISHED:
  explicit TouchTrackball(const std::string &name);

  INLINE GestureState get_gesture_state();

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

  GestureState determine_gesture_state(const PointerEventList *event_list);
  
private:
  int _pointer_input;

  GestureState _gesture_state;

  const double _gesture_detect_time = 0.5;
  const double _pinch_touch_thresh = 100.0;

  std::pair<PT(PointerEvent), PT(PointerEvent)> _current_touches, _prev_touches, _initial_touches;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MouseInterfaceNode::init_type();
    register_type(_type_handle, "TouchTrackball",
                  MouseInterfaceNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "touchTrackball.I"

#endif
