// Filename: pgItem.h
// Created by:  drose (02Jul01)
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

#ifndef PGITEM_H
#define PGITEM_H

#include "pandabase.h"

#include "pgMouseWatcherRegion.h"
#include "pgFrameStyle.h"

#include "namedNode.h"
#include "nodeRelation.h"
#include "luse.h"
#include "pointerTo.h"
#include "pt_Node.h"
#include "pt_NodeRelation.h"
#include "textNode.h"

class ArcChain;

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
class EXPCL_PANDA PGItem : public NamedNode {
PUBLISHED:
  PGItem(const string &name = "");
  virtual ~PGItem();

public:
  PGItem(const PGItem &copy);
  void operator = (const PGItem &copy);

  virtual Node *make_copy() const;

  void activate_region(const LMatrix4f &transform, int sort);
  INLINE PGMouseWatcherRegion *get_region() const;

  virtual void enter();
  virtual void exit();
  virtual void button_down(ButtonHandle button);
  virtual void button_up(ButtonHandle button, bool is_within);

PUBLISHED:
  INLINE void set_frame(float left, float right, float bottom, float top);
  INLINE void set_frame(const LVecBase4f &frame);
  INLINE const LVecBase4f &get_frame() const;
  INLINE bool has_frame() const;
  INLINE void clear_frame();

  INLINE void set_state(int state);
  INLINE int get_state() const;

  INLINE void set_active(bool active);
  INLINE bool get_active() const;

  int get_num_state_defs() const;
  void clear_state_def(int state);
  bool has_state_def(int state) const;
  Node *get_state_def(int state);
  void instance_to_state_def(int state, const ArcChain &chain);

  PGFrameStyle get_frame_style(int state);
  void set_frame_style(int state, const PGFrameStyle &style);

  INLINE const string &get_id() const;
  INLINE string get_enter_event() const;
  INLINE string get_exit_event() const;
  INLINE string get_button_down_event() const;
  INLINE string get_button_up_event() const;

  static TextNode *get_text_node();
  INLINE static void set_text_node(TextNode *node);

private:
  void slot_state_def(int state);
  void update_frame(int state);
  void mark_frames_stale();

  bool _has_frame;
  LVecBase4f _frame;
  int _state;
  bool _active;

  PT(PGMouseWatcherRegion) _region;

  class StateDef {
  public:
    PT_Node _node;
    PGFrameStyle _frame_style;
    PT_NodeRelation _frame_arc;
    bool _frame_stale;
  };
  typedef pvector<StateDef> StateDefs;
  StateDefs _state_defs;

  static PT(TextNode) _text_node;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    register_type(_type_handle, "PGItem",
                  NamedNode::get_class_type());
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
