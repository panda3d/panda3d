// Filename: dataNodeTransmit.h
// Created by:  drose (11Mar02)
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

#ifndef DATANODETRANSMIT_H
#define DATANODETRANSMIT_H

#include "pandabase.h"
#include "eventParameter.h"
#include "typedWritable.h"

class Datagram;
class DatagramIterator;
class BamReader;
class BamWriter;

////////////////////////////////////////////////////////////////////
//       Class : DataNodeTransmit
// Description : Encapsulates the data generated from (or sent into)
//               any particular DataNode.  This is basically just an
//               array of EventParameters, one for each registered
//               input or output wire.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DataNodeTransmit : public TypedWritable {
public:
  INLINE DataNodeTransmit();
  INLINE DataNodeTransmit(const DataNodeTransmit &copy);
  INLINE void operator = (const DataNodeTransmit &copy);
  virtual ~DataNodeTransmit();

  INLINE void reserve(int num_wires);

  INLINE const EventParameter &get_data(int index) const;
  INLINE bool has_data(int index) const;
  INLINE void set_data(int index, const EventParameter &data);

private:
  void slot_data(int index);

  typedef pvector<EventParameter> Data;
  Data _data;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "DataNodeTransmit",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dataNodeTransmit.I"

#endif
