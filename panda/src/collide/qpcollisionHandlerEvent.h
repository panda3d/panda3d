// Filename: qpcollisionHandlerEvent.h
// Created by:  drose (16Mar02)
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

#ifndef qpCOLLISIONHANDLEREVENT_H
#define qpCOLLISIONHANDLEREVENT_H

#include "pandabase.h"

#include "qpcollisionHandler.h"
#include "qpcollisionNode.h"
#include "qpcollisionEntry.h"

#include "pointerTo.h"

///////////////////////////////////////////////////////////////////
//       Class : qpCollisionHandlerEvent
// Description : A specialized kind of qpCollisionHandler that throws an
//               event for each collision detected.  The event thrown
//               may be based on the name of the moving object or the
//               struck object, or both.  The first parameter of the
//               event will be a pointer to the qpCollisionEntry that
//               triggered it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpCollisionHandlerEvent : public qpCollisionHandler {
PUBLISHED:
  qpCollisionHandlerEvent();

public:
  virtual void begin_group();
  virtual void add_entry(qpCollisionEntry *entry);
  virtual bool end_group();

PUBLISHED:
  INLINE void set_in_pattern(const string &pattern);
  INLINE string get_in_pattern() const;
  INLINE void set_again_pattern(const string &pattern);
  INLINE string get_again_pattern() const;
  INLINE void set_out_pattern(const string &pattern);
  INLINE string get_out_pattern() const;

  void clear();

private:
  void throw_event_pattern(const string &pattern, qpCollisionEntry *entry);

  string _in_pattern;
  string _again_pattern;
  string _out_pattern;

  int _index;

  class SortEntries {
  public:
    INLINE bool
    operator () (const PT(qpCollisionEntry) &a,
                 const PT(qpCollisionEntry) &b) const;
    INLINE void operator = (const SortEntries &other);
  };

  typedef pset<PT(qpCollisionEntry), SortEntries> Colliding;
  Colliding _current_colliding;
  Colliding _last_colliding;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpCollisionHandler::init_type();
    register_type(_type_handle, "qpCollisionHandlerEvent",
                  qpCollisionHandler::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "qpcollisionHandlerEvent.I"

#endif



