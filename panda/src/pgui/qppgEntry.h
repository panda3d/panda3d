// Filename: qppgEntry.h
// Created by:  drose (13Mar02)
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

#ifndef qpPGENTRY_H
#define qpPGENTRY_H

#include "pandabase.h"

#include "qppgItem.h"

#include "qptextNode.h"
#include "pointerTo.h"
#include "pvector.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//       Class : qpPGEntry
// Description : This is a particular kind of PGItem that handles
//               simple one-line text entries, of the sort where the
//               user can type any string.
//
//               A qpPGEntry does all of its internal manipulation on a
//               wide string, so it can store the full Unicode
//               character set.  The interface can support either the
//               wide string getters and setters, or the normal 8-bit
//               string getters and setters, which use whatever
//               encoding method is specified by the associated
//               TextNode.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPGEntry : public qpPGItem {
PUBLISHED:
  qpPGEntry(const string &name);
  virtual ~qpPGEntry();

protected:
  qpPGEntry(const qpPGEntry &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(qpCullTraverser *trav, CullTraverserData &data);

  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void keystroke(const MouseWatcherParameter &param, bool background);

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
  INLINE string get_text() const;

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

  INLINE const qpNodePath &get_cursor_def();
  INLINE void clear_cursor_def();

  INLINE void set_cursor_keys_active(bool flag);
  INLINE bool get_cursor_keys_active() const;

  INLINE void set_obscure_mode(bool flag);
  INLINE bool get_obscure_mode() const;

  void set_text_def(int state, qpTextNode *node);
  qpTextNode *get_text_def(int state) const;

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

public:
  INLINE void set_wtext(const wstring &wtext);
  INLINE const wstring &get_wtext() const;


private:
  const wstring &get_display_wtext();
  void slot_text_def(int state);
  void update_text();
  void update_cursor();
  void show_hide_cursor(bool visible);
  void update_state();

  wstring _wtext;
  wstring _obscured_wtext;
  int _cursor_position;
  bool _cursor_stale;
  bool _cursor_visible;

  int _max_chars;
  float _max_width;
  int _num_lines;

  typedef pvector< PT(qpTextNode) > TextDefs;
  TextDefs _text_defs;

  // This is the subgraph that renders both the text and the cursor.
  qpNodePath _text_render_root;

  // This is the node for rendering the actual text that is parented
  // to the above node when the text is generated.
  qpNodePath _current_text;
  qpTextNode *_last_text_def;
  bool _text_geom_stale;

  // This is a list of each row of text in the entry, after it has
  // been wordwrapped by update_text().  It's used by update_cursor()
  // to compute the correct cursor position.
  class WWLine {
  public:
    wstring _str;
    float _left;
  };
  typedef pvector<WWLine> WWLines;
  WWLines _ww_lines;

  // This is the node that represents the cursor geometry.  It is also
  // attached to the above node, and is transformed around and/or
  // hidden according to the cursor's position and blink state.
  qpNodePath _cursor_def;

  double _blink_start;
  double _blink_rate;

  bool _cursor_keys_active;
  bool _obscure_mode;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpPGItem::init_type();
    register_type(_type_handle, "qpPGEntry",
                  qpPGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qppgEntry.I"

#endif
