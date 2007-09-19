// Filename: pointerEventList.h
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

#ifndef POINTEREVENTLIST_H
#define POINTEREVENTLIST_H

#include "pandabase.h"

#include "pointerEvent.h"
#include "typedReferenceCount.h"
#include "eventParameter.h"
#include "pvector.h"

class ModifierPointers;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : PointerEventList
// Description : Records a set of pointer events that happened
//               recently.  This class is usually used only in the
//               data graph, to transmit the recent pointer presses,
//               but it may be used anywhere a list of PointerEvents
//               is desired.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_EVENT PointerEventList : public EventStoreValueBase {
public:
  INLINE PointerEventList();
  INLINE PointerEventList(const PointerEventList &copy);
  INLINE void operator = (const PointerEventList &copy);

  INLINE void add_event(const PointerEvent &event);
  INLINE int get_num_events() const;
  INLINE const PointerEvent &get_event(int n) const;
  INLINE void clear();

  void add_events(const PointerEventList &other);
  
  virtual void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef pvector<PointerEvent> Events;
  Events _events;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);

public:
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EventStoreValueBase::init_type();
    register_type(_type_handle, "PointerEventList",
                  EventStoreValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const PointerEventList &pointerlist) {
  pointerlist.output(out);
  return out;
}

#include "pointerEventList.I"

#endif

