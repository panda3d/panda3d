/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buttonEventList.h
 * @author drose
 * @date 2002-03-12
 */

#ifndef BUTTONEVENTLIST_H
#define BUTTONEVENTLIST_H

#include "pandabase.h"

#include "buttonEvent.h"
#include "typedReferenceCount.h"
#include "eventParameter.h"
#include "pvector.h"

class ModifierButtons;
class Datagram;
class DatagramIterator;

/**
 * Records a set of button events that happened recently.  This class is
 * usually used only in the data graph, to transmit the recent button presses,
 * but it may be used anywhere a list of ButtonEvents is desired.
 */
class EXPCL_PANDA_EVENT ButtonEventList : public ParamValueBase {
PUBLISHED:
  INLINE ButtonEventList();
  INLINE ButtonEventList(const ButtonEventList &copy);
  INLINE void operator = (const ButtonEventList &copy);

  INLINE void add_event(ButtonEvent event);
  INLINE int get_num_events() const;
  INLINE const ButtonEvent &get_event(int n) const;
  INLINE void clear();

  void add_events(const ButtonEventList &other);
  void update_mods(ModifierButtons &mods) const;

  virtual void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef pvector<ButtonEvent> Events;
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
    register_type(_type_handle, "ButtonEventList",
                  ParamValueBase::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const ButtonEventList &buttonlist) {
  buttonlist.output(out);
  return out;
}

#include "buttonEventList.I"

#endif
