// Filename: fltVectorRecord.h
// Created by:  drose (30Aug02)
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

#ifndef FLTVECTORRECORD_H
#define FLTVECTORRECORD_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : FltVectorRecord
// Description : This is an ancillary record of the old (pre-15.4)
//               face node.  Its only use is to provide the direction
//               vector for unidirectional and bidirectional light
//               point faces.
////////////////////////////////////////////////////////////////////
class FltVectorRecord : public FltRecord {
public:
  FltVectorRecord(FltHeader *header);

  const LVector3f &get_vector() const;

protected:
  LVector3f _vector;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltVectorRecord",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltBead;
};

#endif


