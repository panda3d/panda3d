// Filename: mouseWatcher.h
// Created by:  drose (13Jul00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef MOUSEWATCHER_H
#define MOUSEWATCHER_H

#include <pandabase.h>

#include "mouseWatcherRegion.h"
#include "mouseWatcherGroup.h"

#include "dataNode.h"
#include "vec3DataTransition.h"
#include "luse.h"
#include "nodeRelation.h"
#include "pointerTo.h"
#include "eventHandler.h"
#include "pt_NodeRelation.h"
#include "modifierButtons.h"
#include "buttonHandle.h"

#include "pset.h"

class MouseWatcherParameter;

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcher
// Description : This TFormer maintains a list of rectangular regions
//               on the screen that are considered special mouse
//               regions; typically these will be click buttons.  When
//               the mouse passes in or out of one of these regions,
//               or when a button is clicked while the mouse is in one
//               of these regions, an event is thrown.
//
//               Mouse events may also be suppressed from the rest of
//               the datagraph in these special regions.
//
//               This class can also implement a software mouse
//               pointer by automatically generating a transform to
//               apply to a piece of geometry placed under the 2-d
//               scene graph.  It will move the geometry around
//               according to the mouse's known position.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MouseWatcher : public DataNode, public MouseWatcherGroup {
PUBLISHED:
  MouseWatcher(const string &name = "");
  ~MouseWatcher();

  bool remove_region(MouseWatcherRegion *region);

  INLINE bool has_mouse() const;
  INLINE bool is_mouse_open() const;
  INLINE const LPoint2f &get_mouse() const;
  INLINE float get_mouse_x() const;
  INLINE float get_mouse_y() const;

  INLINE bool is_over_region() const;
  INLINE bool is_over_region(float x, float y) const;
  INLINE bool is_over_region(const LPoint2f &pos) const;

  INLINE MouseWatcherRegion *get_over_region() const;
  INLINE MouseWatcherRegion *get_over_region(float x, float y) const;
  MouseWatcherRegion *get_over_region(const LPoint2f &pos) const;

  INLINE void set_button_down_pattern(const string &pattern);
  INLINE const string &get_button_down_pattern() const;

  INLINE void set_button_up_pattern(const string &pattern);
  INLINE const string &get_button_up_pattern() const;

  INLINE void set_enter_pattern(const string &pattern);
  INLINE const string &get_enter_pattern() const;

  INLINE void set_leave_pattern(const string &pattern);
  INLINE const string &get_leave_pattern() const;

  INLINE void set_geometry(NodeRelation *arc);
  INLINE bool has_geometry() const;
  INLINE NodeRelation *get_geometry() const;
  INLINE void clear_geometry();

  INLINE void set_extra_handler(EventHandler *eh);
  INLINE EventHandler *get_extra_handler(void) const;

  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  INLINE ModifierButtons get_modifier_buttons() const;

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  bool add_group(MouseWatcherGroup *group);
  bool remove_group(MouseWatcherGroup *group);

private:
  void set_current_region(MouseWatcherRegion *region);
  void throw_event_pattern(const string &pattern,
                           const MouseWatcherRegion *region,
                           const ButtonHandle &button);

  void press(ButtonHandle button);
  void release(ButtonHandle button);
  void global_keyboard_press(const MouseWatcherParameter &param);
  void global_keyboard_release(const MouseWatcherParameter &param);

  typedef pset< PT(MouseWatcherGroup) > Groups;
  Groups _groups;

  bool _has_mouse;
  int _suppress_flags;
  LPoint2f _mouse;

  PT(MouseWatcherRegion) _current_region;
  PT(MouseWatcherRegion) _button_down_region;
  bool _button_down;

  string _button_down_pattern;
  string _button_up_pattern;
  string _enter_pattern;
  string _leave_pattern;

  PT_NodeRelation _geometry;

  EventHandler* _eh;

  ModifierButtons _mods;

////////////////////////////////////////////////////////////////////
// From parent class DataNode
////////////////////////////////////////////////////////////////////
public:

  virtual void
  transmit_data(AllTransitionsWrapper &data);

  // inputs & outputs
  static TypeHandle _xyz_type;
  static TypeHandle _pixel_xyz_type;
  static TypeHandle _button_events_type;


public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#include "mouseWatcher.I"

#endif
