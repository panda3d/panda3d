/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file axisEventList.h
 * @author jb
 * @date 2023-11-21
 */

#ifndef AXISEVENTLIST_H
#define AXISEVENTLIST_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "paramValue.h"
#include "pvector.h"
#include "axisEvent.h"

class Datagram;
class DatagramIterator;

/**
 * Records a set of axis events that happened recently. This class is
 * usually used only in the data graph, to transmit the recent axis changes,
 * but it may be used anywhere a list of AxisEvents is desired.
 */
class EXPCL_PANDA_EVENT AxisEventList : public ParamValueBase {
PUBLISHED:
  INLINE AxisEventList();
  INLINE AxisEventList(const AxisEventList &copy);
  INLINE void operator = (const AxisEventList &copy);

  INLINE void add_event(AxisEvent event);
  INLINE int get_num_events() const;
  INLINE const AxisEvent &get_event(int n) const;
  INLINE void clear();

  void add_events(const AxisEventList &other);

  virtual void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

  MAKE_SEQ_PROPERTY(events, get_num_events, get_event);

private:
  typedef pvector<AxisEvent> Events;
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
    ParamValueBase::init_type();
    register_type(_type_handle, "AxisEventList",
                  ParamValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const AxisEventList &axislist) {
  axislist.output(out);
  return out;
}

#include "axisEventList.I"

#endif
