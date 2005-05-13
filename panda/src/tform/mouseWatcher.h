// Filename: mouseWatcher.h
// Created by:  drose (12Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef MOUSEWATCHER_H
#define MOUSEWATCHER_H

#include "pandabase.h"

#include "mouseWatcherRegion.h"
#include "mouseWatcherGroup.h"
#include "dataNode.h"
#include "luse.h"
#include "pointerTo.h"
#include "eventHandler.h"
#include "modifierButtons.h"
#include "buttonHandle.h"
#include "buttonEventList.h"
#include "linmath_events.h"
#include "pvector.h"

class MouseWatcherParameter;
class DisplayRegion;

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

  INLINE void set_within_pattern(const string &pattern);
  INLINE const string &get_within_pattern() const;

  INLINE void set_without_pattern(const string &pattern);
  INLINE const string &get_without_pattern() const;

  INLINE void set_geometry(PandaNode *node);
  INLINE bool has_geometry() const;
  INLINE PandaNode *get_geometry() const;
  INLINE void clear_geometry();

  INLINE void set_extra_handler(EventHandler *eh);
  INLINE EventHandler *get_extra_handler(void) const;

  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  INLINE ModifierButtons get_modifier_buttons() const;

  INLINE void set_display_region(DisplayRegion *dr);
  INLINE void clear_display_region();
  INLINE DisplayRegion *get_display_region() const;
  INLINE bool has_display_region() const;

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  bool add_group(MouseWatcherGroup *group);
  bool remove_group(MouseWatcherGroup *group);

protected:
  typedef pvector< PT(MouseWatcherRegion) > VRegions;
  void get_over_regions(VRegions &regions, const LPoint2f &pos) const;
  static MouseWatcherRegion *get_preferred_region(const VRegions &regions);

  void set_current_regions(VRegions &regions);
  void clear_current_regions();
  static void intersect_regions(MouseWatcher::VRegions &result,
                                const MouseWatcher::VRegions &regions_a,
                                const MouseWatcher::VRegions &regions_b);
  static void remove_region_from(MouseWatcher::VRegions &regions,
                                 MouseWatcherRegion *region);
  static void remove_regions_from(MouseWatcher::VRegions &regions,
                                  MouseWatcherGroup *group);

    
  void throw_event_pattern(const string &pattern,
                           const MouseWatcherRegion *region,
                           const ButtonHandle &button);

  void move(ButtonHandle button);
  void press(ButtonHandle button);
  void release(ButtonHandle button);
  void keystroke(int keycode);
  void candidate(const wstring &candidate, size_t highlight_start, 
                 size_t highlight_end, size_t cursor_pos);
                 
  void global_keyboard_press(const MouseWatcherParameter &param);
  void global_keyboard_release(const MouseWatcherParameter &param);

  INLINE void within_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  INLINE void without_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  void enter_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  void exit_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);

  void set_no_mouse();
  void set_mouse(const LVecBase2f &xy, const LVecBase2f &pixel_xy);

  // This wants to be a set, but because you cannot export sets across
  // dlls in windows, we will make it a vector instead
  typedef pvector< PT(MouseWatcherGroup) > Groups;
  Groups _groups;

  bool _has_mouse;
  int _suppress_flags;
  LPoint2f _mouse;
  LPoint2f _mouse_pixel;

  VRegions _current_regions;
  PT(MouseWatcherRegion) _preferred_region;
  PT(MouseWatcherRegion) _preferred_button_down_region;
  bool _button_down;

  bool _enter_multiple;
  bool _implicit_click;

  string _button_down_pattern;
  string _button_up_pattern;
  string _enter_pattern;
  string _leave_pattern;
  string _within_pattern;
  string _without_pattern;

  PT(PandaNode) _geometry;

  EventHandler *_eh;
  ModifierButtons _mods;
  DisplayRegion *_display_region;

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _pixel_xy_input;
  int _pixel_size_input;
  int _xy_input;
  int _button_events_input;

  // outputs
  int _pixel_xy_output;
  int _pixel_size_output;
  int _xy_output;
  int _button_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _xy;
  PT(ButtonEventList) _button_events;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    DataNode::init_type();
    register_type(_type_handle, "MouseWatcher",
                  DataNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "mouseWatcher.I"

#endif
