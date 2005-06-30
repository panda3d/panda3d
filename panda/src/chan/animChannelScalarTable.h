// Filename: animChannelScalarTable.h
// Created by:  drose (22Feb99)
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

#ifndef ANIMCHANNELSCALARTABLE_H
#define ANIMCHANNELSCALARTABLE_H

#include "pandabase.h"

#include "animChannel.h"

#include "pointerToArray.h"
#include "pta_float.h"

////////////////////////////////////////////////////////////////////
//       Class : AnimChannelScalarTable
// Description : An animation channel that issues a scalar each frame,
//               read from a table such as might have been read from
//               an egg file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AnimChannelScalarTable : public AnimChannelScalar {
public:
  AnimChannelScalarTable(AnimGroup *parent, const string &name);

  virtual bool has_changed(int last_frame, int this_frame);
  virtual void get_value(int frame, float &value);

  void set_table(const CPTA_float &table);

PUBLISHED:
  INLINE bool has_table() const;
  INLINE void clear_table();

public:
  virtual void write(ostream &out, int indent_level) const;

protected:
  AnimChannelScalarTable(void);
  CPTA_float _table;

public:
  static void register_with_read_factory(void);
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
