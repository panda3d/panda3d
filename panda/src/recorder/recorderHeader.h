// Filename: recorderHeader.h
// Created by:  drose (29Jan04)
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

#ifndef RECORDERHEADER_H
#define RECORDERHEADER_H

#include "pandabase.h"
#include "recorderBase.h"
#include "typedWritable.h"

#include <time.h>

class BamWriter;
class BamReader;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : RecorderHeader
// Description : This object contains the header information written
//               out at the beginning of a recorded session file.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RecorderHeader : public TypedWritable {
public:
  INLINE RecorderHeader();
  INLINE RecorderHeader(const RecorderHeader &copy);
  INLINE void operator = (const RecorderHeader &copy);
  INLINE ~RecorderHeader();

  time_t _start_time;
  int _random_seed;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  
protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "RecorderHeader",
                  TypedWritable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "recorderHeader.I"

#endif

