// Filename: pgEntry.h
// Created by:  drose (10Jul01)
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

#ifndef PGENTRY_H
#define PGENTRY_H

#include "pandabase.h"

#include "pgItem.h"

#include "textNode.h"
#include "pointerTo.h"
#include "pvector.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//       Class : PGEntry
// Description : This is a particular kind of PGItem that handles
//               simple one-line text entries, of the sort where the
//               user can type any string.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGEntry : public PGItem {
PUBLISHED:
  PGEntry(const string &name = "");
  virtual ~PGEntry();

public:
  PGEntry(const PGEntry &copy);
  void operator = (const PGEntry &copy);

  virtual Node *make_copy() const;

  virtual void draw_item(PGTop *top, GraphicsStateGuardian *gsg, 
                         const AllTransitionsWrapper &trans);

  virtual void press(const MouseWatcherParameter &param, bool background);

  virtual void accept(const MouseWatcherParameter &param);
  virtual void overflow(const MouseWatcherParameter &param);
  virtual void type(const MouseWatcherParameter &param);
  virtual void erase(const MouseWatcherParameter &param);

PUBLISHED:
  enum State {
    S_focus = 0,
    S_no_focus,
    S_inactive
  };

  void setup(float width, int num_lines);

  INLINE void set_text(const string &text);
  INLINE const string &get_text() const;

  INLINE void set_cursor_position(int position);
  INLINE int get_cursor_position() const;

  INLINE void set_max_chars(int max_chars);
  INLINE int get_max_chars() const;
  INLINE void set_max_width(float max_width);
  INLINE float get_max_width() const;
  INLINE void set_num_lines(int num_lines);
  INLINE int get_num_lines() const;

  INLINE void set_blink_rate(float blink_rate);
  INLINE float get_blink_rate() const;

  INLINE Node *get_cursor_def();
  INLINE void clear_cursor_def();

  INLINE void set_cursor_keys_active(bool flag);
  INLINE bool get_cursor_keys_active() const;

  void set_text_def(int state, TextNode *node);
  TextNode *get_text_def(int state) const;

  virtual void set_active(bool active);
  virtual void set_focus(bool focus);

  INLINE static string get_accept_prefix();
  INLINE static string get_overflow_prefix();
  INLINE static string get_type_prefix();
  INLINE static string get_erase_prefix();

  INLINE string get_accept_event(const ButtonHandle &button) const;
  INLINE string get_overflow_event() const;
  INLINE string get_type_event() const;
  INLINE string get_erase_event() const;

private:
  void slot_text_def(int state);
  void update_text();
  void update_cursor();
  void show_hide_cursor(bool visible);
  void update_state();

  string _text;
  int _cursor_position;
  bool _cursor_stale;
  bool _cursor_visible;

  int _max_chars;
  float _max_width;
  int _num_lines;

  typedef pvector< PT(TextNode) > TextDefs;
  TextDefs _text_defs;

  // This node is the root of the subgraph that renders both the text
  // and the cursor.
  PT_Node _text_render_root;

  // This is the arc that is attached to the above node when the text
  // is generated.
  NodeRelation *_current_text_arc;
  TextNode *_last_text_def;
  bool _text_geom_stale;

  // This is a list of each row of text in the entry, after it has
  // been wordwrapped by update_text().  It's used by update_cursor()
  // to compute the correct cursor position.
  class WWLine {
  public:
    string _str;
    float _left;
  };
  typedef pvector<WWLine> WWLines;
  WWLines _ww_lines;

  // This is the arc above the node that represents the cursor
  // geometry.  It is also attached to the above node, and is
  // transformed around and/or hidden according to the cursor's
  // position and blink state.
  NodeRelation *_cursor_def;

  double _blink_start;
  double _blink_rate;

  bool _cursor_keys_active;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGItem::init_type();
    register_type(_type_handle, "PGEntry",
                  PGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgEntry.I"

#endif
