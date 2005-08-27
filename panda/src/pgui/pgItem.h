// Filename: pgItem.h
// Created by:  drose (13Mar02)
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

#ifndef PGITEM_H
#define PGITEM_H

#include "pandabase.h"

#include "pgMouseWatcherRegion.h"
#include "pgFrameStyle.h"
#include "pgItemNotify.h"

#include "pandaNode.h"
#include "nodePath.h"
#include "luse.h"
#include "pointerTo.h"
#include "audioSound.h"
#include "textNode.h"
#include "plane.h"
#include "pmap.h"

class PGTop;
class MouseWatcherParameter;
class AudioSound;
class ClipPlaneAttrib;

////////////////////////////////////////////////////////////////////
//       Class : PGItem
// Description : This is the base class for all the various kinds of
//               gui widget objects.
//
//               It is a Node which corresponds to a rectangular
//               region on the screen, and it may have any number of
//               "state" subgraphs, one of which is rendered at any
//               given time according to its current state.
//
//               The PGItem node must be parented to the scene graph
//               somewhere beneath a PGTop node in order for this
//               behavior to work.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGItem : public PandaNode {
PUBLISHED:
  PGItem(const string &name);
  virtual ~PGItem();

protected:
  PGItem(const PGItem &copy);

  virtual PandaNode *make_copy() const;
  virtual void transform_changed();
  virtual void draw_mask_changed();

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual BoundingVolume *recompute_internal_bound();

public:
  virtual void xform(const LMatrix4f &mat);
  bool activate_region(const LMatrix4f &transform, int sort,
                       const ClipPlaneAttrib *cpa);
  INLINE PGMouseWatcherRegion *get_region() const;

  virtual void enter(const MouseWatcherParameter &param);
  virtual void exit(const MouseWatcherParameter &param);
  virtual void within(const MouseWatcherParameter &param);
  virtual void without(const MouseWatcherParameter &param);
  virtual void focus_in();
  virtual void focus_out();
  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void release(const MouseWatcherParameter &param, bool background);
  virtual void keystroke(const MouseWatcherParameter &param, bool background);
  virtual void candidate(const MouseWatcherParameter &param, bool background);
  virtual void move(const MouseWatcherParameter &param);

  static void background_press(const MouseWatcherParameter &param);
  static void background_release(const MouseWatcherParameter &param);
  static void background_keystroke(const MouseWatcherParameter &param);
  static void background_candidate(const MouseWatcherParameter &param);

  INLINE void set_notify(PGItemNotify *notify);
  INLINE bool has_notify() const;
  INLINE PGItemNotify *get_notify() const;

PUBLISHED:
  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE const LVecBase4f &get_frame() const;
  INLINE bool has_frame() const;
  INLINE void clear_frame();

  INLINE void set_state(int state);
  INLINE int get_state() const;

  virtual void set_active(bool active);
  INLINE bool get_active() const;

  virtual void set_focus(bool focus);
  INLINE bool get_focus() const;

  void set_background_focus(bool focus);
  INLINE bool get_background_focus() const;

  INLINE void set_suppress_flags(int suppress_flags);
  INLINE int get_suppress_flags() const;

  int get_num_state_defs() const;
  void clear_state_def(int state);
  bool has_state_def(int state) const;
  NodePath &get_state_def(int state);
  NodePath instance_to_state_def(int state, const NodePath &path);

  PGFrameStyle get_frame_style(int state);
  void set_frame_style(int state, const PGFrameStyle &style);

  INLINE const string &get_id() const;
  INLINE void set_id(const string &id);

  INLINE static string get_enter_prefix();
  INLINE static string get_exit_prefix();
  INLINE static string get_within_prefix();
  INLINE static string get_without_prefix();
  INLINE static string get_focus_in_prefix();
  INLINE static string get_focus_out_prefix();
  INLINE static string get_press_prefix();
  INLINE static string get_release_prefix();
  INLINE static string get_keystroke_prefix();

  INLINE string get_enter_event() const;
  INLINE string get_exit_event() const;
  INLINE string get_within_event() const;
  INLINE string get_without_event() const;
  INLINE string get_focus_in_event() const;
  INLINE string get_focus_out_event() const;
  INLINE string get_press_event(const ButtonHandle &button) const;
  INLINE string get_release_event(const ButtonHandle &button) const;
  INLINE string get_keystroke_event() const;

  INLINE LMatrix4f get_frame_inv_xform() const;

#ifdef HAVE_AUDIO
  void set_sound(const string &event, AudioSound *sound);
  void clear_sound(const string &event);
  AudioSound *get_sound(const string &event) const;
  bool has_sound(const string &event) const;
#endif

  static TextNode *get_text_node();
  INLINE static void set_text_node(TextNode *node);

  INLINE static PGItem *get_focus_item();

protected:
  void play_sound(const string &event);

  void reduce_region(LVecBase4f &clip, PGItem *obscurer) const;
  void reduce_region(LVecBase4f &frame, float px, float py) const;
  LVecBase4f get_relative_frame(PGItem *item) const;
  LPoint3f mouse_to_local(const LPoint2f &mouse_point) const;

  virtual void frame_changed();

private:
  void slot_state_def(int state);
  void update_frame(int state);
  void mark_frames_stale();

  INLINE static float compute_area(const LVecBase4f &frame);
  INLINE static void compare_largest(const LVecBase4f *&largest, 
                                     float &largest_area, 
                                     const LVecBase4f *new_frame);

  bool clip_frame(pvector<LPoint2f> &source_points, const Planef &plane) const;

private:
  PGItemNotify *_notify;

  bool _has_frame;
  LVecBase4f _frame;
  int _state;
  enum Flags {
    F_active             = 0x01,
    F_focus              = 0x02,
    F_background_focus   = 0x04,
  };
  int _flags;

  PT(PGMouseWatcherRegion) _region;

  LMatrix4f _frame_inv_xform;

  class StateDef {
  public:
    NodePath _root;
    PGFrameStyle _frame_style;
    NodePath _frame;
    bool _frame_stale;
  };
  typedef pvector<StateDef> StateDefs;
  StateDefs _state_defs;

#ifdef HAVE_AUDIO
  typedef pmap<string, PT(AudioSound) > Sounds;
  Sounds _sounds;
#endif

  static PT(TextNode) _text_node;
  static PGItem *_focus_item;

  typedef pset<PGItem *> BackgroundFocus;
  static BackgroundFocus _background_focus;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "PGItem",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgItem.I"

#endif
