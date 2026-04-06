/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgEntry.h
 * @author drose
 * @date 2002-03-13
 */

#ifndef PGENTRY_H
#define PGENTRY_H

#include "pandabase.h"

#include "pgItem.h"

#include "textNode.h"
#include "pointerTo.h"
#include "small_vector.h"
#include "clockObject.h"
#include "textAssembler.h"
#include "pipeline.h"

/**
 * This is a particular kind of PGItem that handles simple one-line or short
 * multi-line text entries, of the sort where the user can type any string.
 *
 * A PGEntry does all of its internal manipulation on a wide string, so it can
 * store the full Unicode character set.  The interface can support either the
 * wide string getters and setters, or the normal 8-bit string getters and
 * setters, which use whatever encoding method is specified by the associated
 * TextNode.
 */
class EXPCL_PANDA_PGUI PGEntry : public PGItem {
PUBLISHED:
  explicit PGEntry(const std::string &name);
  virtual ~PGEntry();

protected:
  PGEntry(const PGEntry &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void press(const MouseWatcherParameter &param, bool background);
  virtual void keystroke(const MouseWatcherParameter &param, bool background);
  virtual void candidate(const MouseWatcherParameter &param, bool background);

  virtual void accept(const MouseWatcherParameter &param);
  virtual void accept_failed(const MouseWatcherParameter &param);
  virtual void overflow(const MouseWatcherParameter &param);
  virtual void type(const MouseWatcherParameter &param);
  virtual void erase(const MouseWatcherParameter &param);
  virtual void cursormove();

PUBLISHED:
  enum State {
    S_focus = 0,
    S_no_focus,
    S_inactive
  };

  void setup(PN_stdfloat width, int num_lines);
  void setup_minimal(PN_stdfloat width, int num_lines);

  INLINE bool set_text(const std::string &text);
  INLINE std::string get_plain_text() const;
  INLINE std::string get_text() const;

  INLINE int get_num_characters() const;
  INLINE wchar_t get_character(int n) const;
  INLINE const TextGraphic *get_graphic(int n) const;
  INLINE const TextProperties &get_properties(int n) const;

  INLINE void set_cursor_position(int position);
  INLINE int get_cursor_position() const;
  MAKE_PROPERTY(cursor_position, get_cursor_position, set_cursor_position);

  INLINE PN_stdfloat get_cursor_X() const;
  INLINE PN_stdfloat get_cursor_Y() const;

  INLINE void set_max_chars(int max_chars);
  INLINE int get_max_chars() const;
  INLINE void set_max_width(PN_stdfloat max_width);
  INLINE PN_stdfloat get_max_width() const;
  INLINE void set_num_lines(int num_lines);
  INLINE int get_num_lines() const;
  MAKE_PROPERTY(max_chars, get_max_chars, set_max_chars);
  MAKE_PROPERTY(max_width, get_max_width, set_max_width);
  MAKE_PROPERTY(num_lines, get_num_lines, set_num_lines);

  INLINE void set_blink_rate(PN_stdfloat blink_rate);
  INLINE PN_stdfloat get_blink_rate() const;
  MAKE_PROPERTY(blink_rate, get_blink_rate, set_blink_rate);

  INLINE NodePath get_cursor_def();
  INLINE void clear_cursor_def();
  MAKE_PROPERTY(cursor_def, get_cursor_def);

  INLINE void set_cursor_keys_active(bool flag);
  INLINE bool get_cursor_keys_active() const;
  MAKE_PROPERTY(cursor_keys_active, get_cursor_keys_active, set_cursor_keys_active);

  INLINE void set_obscure_mode(bool flag);
  INLINE bool get_obscure_mode() const;
  MAKE_PROPERTY(obscure_mode, get_obscure_mode, set_obscure_mode);

  INLINE void set_overflow_mode(bool flag);
  INLINE bool get_overflow_mode() const;
  MAKE_PROPERTY(overflow_mode, get_overflow_mode, set_overflow_mode);

  INLINE void set_candidate_active(const std::string &candidate_active);
  INLINE const std::string &get_candidate_active() const;
  MAKE_PROPERTY(candidate_active, get_candidate_active, set_candidate_active);

  INLINE void set_candidate_inactive(const std::string &candidate_inactive);
  INLINE const std::string &get_candidate_inactive() const;
  MAKE_PROPERTY(candidate_inactive, get_candidate_inactive, set_candidate_inactive);

  void set_text_def(int state, TextNode *node);
  TextNode *get_text_def(int state) const;

  virtual void set_active(bool active) final;
  virtual void set_focus(bool focus);
  MAKE_PROPERTY(active, get_active, set_active);
  MAKE_PROPERTY(focus, get_focus, set_focus);

  INLINE static std::string get_accept_prefix();
  INLINE static std::string get_accept_failed_prefix();
  INLINE static std::string get_overflow_prefix();
  INLINE static std::string get_type_prefix();
  INLINE static std::string get_erase_prefix();
  INLINE static std::string get_cursormove_prefix();

  INLINE std::string get_accept_event(const ButtonHandle &button) const;
  INLINE std::string get_accept_failed_event(const ButtonHandle &button) const;
  INLINE std::string get_overflow_event() const;
  INLINE std::string get_type_event() const;
  INLINE std::string get_erase_event() const;
  INLINE std::string get_cursormove_event() const;

  MAKE_PROPERTY(accept_prefix, get_accept_prefix);
  MAKE_PROPERTY(accept_failed_prefix, get_accept_failed_prefix);
  MAKE_PROPERTY(overflow_prefix, get_overflow_prefix);
  MAKE_PROPERTY(type_prefix, get_type_prefix);
  MAKE_PROPERTY(erase_prefix, get_erase_prefix);
  MAKE_PROPERTY(cursormove_prefix, get_cursormove_prefix);

  INLINE bool set_wtext(const std::wstring &wtext);
  INLINE std::wstring get_plain_wtext() const;
  INLINE std::wstring get_wtext() const;
  INLINE void set_accept_enabled(bool enabled);
  bool is_wtext() const;


private:
  void slot_text_def(int state);
  void update_text();
  void update_cursor();
  void show_hide_cursor(bool visible);
  void update_state();

  TextAssembler _text;
  TextAssembler _obscure_text;
  TextAssembler _candidate_text;
  int _cursor_position;
  bool _cursor_stale;
  bool _cursor_visible;

  std::wstring _candidate_wtext;
  size_t _candidate_highlight_start;
  size_t _candidate_highlight_end;
  size_t _candidate_cursor_pos;

  int _max_chars;
  PN_stdfloat _max_width;
  int _num_lines;

  bool _accept_enabled;

  std::string _candidate_active;
  std::string _candidate_inactive;

  // Most entries have 3 states.
  typedef small_vector<PT(TextNode), 3> TextDefs;
  TextDefs _text_defs;

  // This is the subgraph that renders both the text and the cursor.
  NodePath _text_render_root;

  // This is the node for rendering the actual text that is parented to the
  // above node when the text is generated.
  NodePath _current_text;
  TextNode *_last_text_def;
  bool _text_geom_stale;
  bool _text_geom_flattened;

  // This is the node that represents the cursor geometry.  It is also
  // attached to the above node, and is transformed around andor hidden
  // according to the cursor's position and blink state.
  NodePath _cursor_scale;
  NodePath _cursor_def;

  double _blink_start;
  double _blink_rate;

  bool _cursor_keys_active;
  bool _obscure_mode;
  bool _overflow_mode;

  PN_stdfloat _current_padding;

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
