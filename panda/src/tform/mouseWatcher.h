/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mouseWatcher.h
 * @author drose
 * @date 2002-03-12
 */

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
#include "pointerEvent.h"
#include "pointerEventList.h"
#include "linmath_events.h"
#include "bitArray.h"
#include "clockObject.h"
#include "pvector.h"
#include "displayRegion.h"

class MouseWatcherParameter;
class DisplayRegion;

/**
 * This TFormer maintains a list of rectangular regions on the screen that are
 * considered special mouse regions; typically these will be click buttons.
 * When the mouse passes in or out of one of these regions, or when a button
 * is clicked while the mouse is in one of these regions, an event is thrown.
 *
 * Mouse events may also be suppressed from the rest of the datagraph in these
 * special regions.
 *
 * This class can also implement a software mouse pointer by automatically
 * generating a transform to apply to a piece of geometry placed under the 2-d
 * scene graph.  It will move the geometry around according to the mouse's
 * known position.
 *
 * Finally, this class can keep a record of the mouse trail.  This is useful
 * if you want to know, not just where the mouse is, but the exact sequence of
 * movements it took to get there.  This information is mainly useful for
 * gesture-recognition code.  To use trail logging, you need to enable the
 * generation of pointer events in the GraphicsWindowInputDevice and set the
 * trail log duration in the MouseWatcher.  Otherwise, the trail log will be
 * empty.
 */
class EXPCL_PANDA_TFORM MouseWatcher : public DataNode, public MouseWatcherBase {
PUBLISHED:
  explicit MouseWatcher(const std::string &name = "");
  ~MouseWatcher();

  bool remove_region(MouseWatcherRegion *region);

  INLINE bool has_mouse() const;
  INLINE bool is_mouse_open() const;
  INLINE const LPoint2 &get_mouse() const;
  INLINE PN_stdfloat get_mouse_x() const;
  INLINE PN_stdfloat get_mouse_y() const;

  INLINE void set_frame(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  INLINE void set_frame(const LVecBase4 &frame);
  INLINE const LVecBase4 &get_frame() const;

  INLINE bool is_over_region() const;
  INLINE bool is_over_region(PN_stdfloat x, PN_stdfloat y) const;
  INLINE bool is_over_region(const LPoint2 &pos) const;

  INLINE MouseWatcherRegion *get_over_region() const;
  INLINE MouseWatcherRegion *get_over_region(PN_stdfloat x, PN_stdfloat y) const;
  MouseWatcherRegion *get_over_region(const LPoint2 &pos) const;

  INLINE bool is_button_down(ButtonHandle button) const;

  INLINE void set_button_down_pattern(const std::string &pattern);
  INLINE const std::string &get_button_down_pattern() const;

  INLINE void set_button_up_pattern(const std::string &pattern);
  INLINE const std::string &get_button_up_pattern() const;

  INLINE void set_button_repeat_pattern(const std::string &pattern);
  INLINE const std::string &get_button_repeat_pattern() const;

  INLINE void set_enter_pattern(const std::string &pattern);
  INLINE const std::string &get_enter_pattern() const;

  INLINE void set_leave_pattern(const std::string &pattern);
  INLINE const std::string &get_leave_pattern() const;

  INLINE void set_within_pattern(const std::string &pattern);
  INLINE const std::string &get_within_pattern() const;

  INLINE void set_without_pattern(const std::string &pattern);
  INLINE const std::string &get_without_pattern() const;

  INLINE void set_geometry(PandaNode *node);
  INLINE bool has_geometry() const;
  INLINE PandaNode *get_geometry() const;
  INLINE void clear_geometry();

  INLINE void set_extra_handler(EventHandler *eh);
  INLINE EventHandler *get_extra_handler() const;

  INLINE void set_modifier_buttons(const ModifierButtons &mods);
  INLINE ModifierButtons get_modifier_buttons() const;

  INLINE void set_display_region(DisplayRegion *dr);
  INLINE void clear_display_region();
  INLINE DisplayRegion *get_display_region() const;
  INLINE bool has_display_region() const;

  bool add_group(MouseWatcherGroup *group);
  bool remove_group(MouseWatcherGroup *group);
  bool replace_group(MouseWatcherGroup *old_group, MouseWatcherGroup *new_group);
  int get_num_groups() const;
  MouseWatcherGroup *get_group(int n) const;
  MAKE_SEQ(get_groups, get_num_groups, get_group);

  INLINE void set_inactivity_timeout(double timeout);
  INLINE bool has_inactivity_timeout() const;
  INLINE double get_inactivity_timeout() const;
  INLINE void clear_inactivity_timeout();

  INLINE void set_inactivity_timeout_event(const std::string &event);
  INLINE const std::string &get_inactivity_timeout_event() const;

  INLINE CPT(PointerEventList) get_trail_log() const;
  INLINE size_t num_trail_recent() const;
  void         set_trail_log_duration(double duration);
  PT(GeomNode) get_trail_node();
  void         clear_trail_node();
  INLINE void  clear_trail_log();

  void note_activity();

public:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  void get_over_regions(Regions &regions, const LPoint2 &pos) const;
  static MouseWatcherRegion *get_preferred_region(const Regions &regions);

  void set_current_regions(Regions &regions);
  void clear_current_regions();

#ifndef NDEBUG
  virtual void do_show_regions(const NodePath &render2d,
                               const std::string &bin_name, int draw_order);
  virtual void do_hide_regions();
#endif  // NDEBUG

  static void intersect_regions(Regions &only_a,
                                Regions &only_b,
                                Regions &both,
                                const Regions &regions_a,
                                const Regions &regions_b);
  static bool remove_region_from(Regions &regions,
                                 MouseWatcherRegion *region);
  static bool has_region_in(const Regions &regions,
                            MouseWatcherRegion *region);

  void throw_event_pattern(const std::string &pattern,
                           const MouseWatcherRegion *region,
                           const ButtonHandle &button);

  void move();
  void press(ButtonHandle button, bool keyrepeat);
  void release(ButtonHandle button);
  void keystroke(int keycode);
  void candidate(const std::wstring &candidate, size_t highlight_start,
                 size_t highlight_end, size_t cursor_pos);

  void global_keyboard_press(const MouseWatcherParameter &param);
  void global_keyboard_release(const MouseWatcherParameter &param);

  INLINE void within_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  INLINE void without_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  void enter_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);
  void exit_region(MouseWatcherRegion *region, const MouseWatcherParameter &param);

  void set_no_mouse();
  void set_mouse(const LVecBase2 &xy, const LVecBase2 &pixel_xy);

private:
  void consider_keyboard_suppress(const MouseWatcherRegion *region);
  void discard_excess_trail_log();
  void update_trail_node();

  bool constrain_display_region(DisplayRegion *display_region,
                                LVecBase2 &f, LVecBase2 &p,
                                Thread *current_thread);

private:
  // This wants to be a set, but because you cannot export sets across dlls in
  // windows, we will make it a vector instead
  typedef pvector< PT(MouseWatcherGroup) > Groups;
  Groups _groups;

  bool _has_mouse;
  int _internal_suppress;
  int _external_suppress;
  LPoint2 _mouse;
  LPoint2 _mouse_pixel;
  BitArray _current_buttons_down;

  LVecBase4 _frame;

  PT(PointerEventList) _trail_log;
  size_t _num_trail_recent;
  double _trail_log_duration;
  PT(GeomNode) _trail_node;

  Regions _current_regions;
  PT(MouseWatcherRegion) _preferred_region;
  PT(MouseWatcherRegion) _preferred_button_down_region;
  bool _button_down;

  bool _enter_multiple;
  bool _implicit_click;

  std::string _button_down_pattern;
  std::string _button_up_pattern;
  std::string _button_repeat_pattern;
  std::string _enter_pattern;
  std::string _leave_pattern;
  std::string _within_pattern;
  std::string _without_pattern;

  PT(PandaNode) _geometry;

  EventHandler *_eh;
  ModifierButtons _mods;
  DisplayRegion *_display_region;
  DisplayRegion *_button_down_display_region;

  bool _has_inactivity_timeout;
  double _inactivity_timeout;
  std::string _inactivity_timeout_event;
  double _last_activity;

  enum InactivityState {
    IS_active,
    IS_inactive,
    IS_active_to_inactive,
    IS_inactive_to_active,
  };
  InactivityState _inactivity_state;

#ifndef NDEBUG
  NodePath _show_regions_render2d;
  std::string _show_regions_bin_name;
  int _show_regions_draw_order;
#endif

protected:
  // Inherited from DataNode
  virtual void do_transmit_data(DataGraphTraverser *trav,
                                const DataNodeTransmit &input,
                                DataNodeTransmit &output);

private:
  // inputs
  int _pixel_xy_input;
  int _pixel_size_input;
  int _xy_input;
  int _button_events_input;
  int _pointer_events_input;

  // outputs
  int _pixel_xy_output;
  int _pixel_size_output;
  int _xy_output;
  int _button_events_output;

  PT(EventStoreVec2) _pixel_xy;
  PT(EventStoreVec2) _xy;
  PT(EventStoreVec2) _pixel_size;
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
