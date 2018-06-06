/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionHandlerEvent.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONHANDLEREVENT_H
#define COLLISIONHANDLEREVENT_H

#include "pandabase.h"

#include "collisionHandler.h"
#include "collisionNode.h"
#include "collisionEntry.h"

#include "vector_string.h"
#include "pointerTo.h"

/**
 * A specialized kind of CollisionHandler that throws an event for each
 * collision detected.  The event thrown may be based on the name of the
 * moving object or the struck object, or both.  The first parameter of the
 * event will be a pointer to the CollisionEntry that triggered it.
 */
class EXPCL_PANDA_COLLIDE CollisionHandlerEvent : public CollisionHandler {
PUBLISHED:
  CollisionHandlerEvent();

public:
  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual bool end_group();

PUBLISHED:
  INLINE void clear_in_patterns();
  INLINE void add_in_pattern(const std::string &in_pattern);
  INLINE void set_in_pattern(const std::string &in_pattern);
  INLINE int get_num_in_patterns() const;
  INLINE std::string get_in_pattern(int n) const;
  MAKE_SEQ(get_in_patterns, get_num_in_patterns, get_in_pattern);

  INLINE void clear_again_patterns();
  INLINE void add_again_pattern(const std::string &again_pattern);
  INLINE void set_again_pattern(const std::string &again_pattern);
  INLINE int get_num_again_patterns() const;
  INLINE std::string get_again_pattern(int n) const;
  MAKE_SEQ(get_again_patterns, get_num_again_patterns, get_again_pattern);

  INLINE void clear_out_patterns();
  INLINE void add_out_pattern(const std::string &out_pattern);
  INLINE void set_out_pattern(const std::string &out_pattern);
  INLINE int get_num_out_patterns() const;
  INLINE std::string get_out_pattern(int n) const;
  MAKE_SEQ(get_out_patterns, get_num_out_patterns, get_out_pattern);

  MAKE_SEQ_PROPERTY(in_patterns, get_num_in_patterns, get_in_pattern);
  MAKE_SEQ_PROPERTY(again_patterns, get_num_again_patterns, get_out_pattern);
  MAKE_SEQ_PROPERTY(out_patterns, get_num_out_patterns, get_out_pattern);

  void clear();
  void flush();

protected:
  void throw_event_for(const vector_string &patterns, CollisionEntry *entry);
  void throw_event_pattern(const std::string &pattern, CollisionEntry *entry);

  vector_string _in_patterns;
  vector_string _again_patterns;
  vector_string _out_patterns;

  int _index;

  class SortEntries {
  public:
    INLINE bool
    operator () (const PT(CollisionEntry) &a,
                 const PT(CollisionEntry) &b) const;
    INLINE void operator = (const SortEntries &other);
  };

  typedef pset<PT(CollisionEntry), SortEntries> Colliding;
  Colliding _current_colliding;
  Colliding _last_colliding;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandler::init_type();
    register_type(_type_handle, "CollisionHandlerEvent",
                  CollisionHandler::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerEvent.I"

#endif
