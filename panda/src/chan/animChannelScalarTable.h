/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelScalarTable.h
 * @author drose
 * @date 1999-02-22
 */

#ifndef ANIMCHANNELSCALARTABLE_H
#define ANIMCHANNELSCALARTABLE_H

#include "pandabase.h"

#include "animChannel.h"

#include "pointerToArray.h"
#include "pta_stdfloat.h"

/**
 * An animation channel that issues a scalar each frame, read from a table
 * such as might have been read from an egg file.
 */
class EXPCL_PANDA_CHAN AnimChannelScalarTable : public AnimChannelScalar {
protected:
  AnimChannelScalarTable();
  AnimChannelScalarTable(AnimGroup *parent, const AnimChannelScalarTable &copy);

public:
  AnimChannelScalarTable(AnimGroup *parent, const std::string &name);

  virtual bool has_changed(int last_frame, double last_frac,
                           int this_frame, double this_frac);
  virtual void get_value(int frame, PN_stdfloat &value);

PUBLISHED:
  void set_table(const CPTA_stdfloat &table);
  INLINE CPTA_stdfloat get_table() const;

  INLINE bool has_table() const;
  INLINE void clear_table();

  MAKE_PROPERTY2(table, has_table, get_table, set_table, clear_table);

public:
  virtual void write(std::ostream &out, int indent_level) const;

protected:
  virtual AnimGroup *make_copy(AnimGroup *parent) const;

protected:
  CPTA_stdfloat _table;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_AnimChannelScalarTable(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    AnimChannelScalar::init_type();
    register_type(_type_handle, "AnimChannelScalarTable",
                  AnimChannelScalar::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "animChannelScalarTable.I"

#endif
