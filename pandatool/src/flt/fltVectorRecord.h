/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltVectorRecord.h
 * @author drose
 * @date 2002-08-30
 */

#ifndef FLTVECTORRECORD_H
#define FLTVECTORRECORD_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

/**
 * This is an ancillary record of the old (pre-15.4) face node.  Its only use
 * is to provide the direction vector for unidirectional and bidirectional
 * light point faces.
 */
class FltVectorRecord : public FltRecord {
public:
  FltVectorRecord(FltHeader *header);

  const LVector3 &get_vector() const;

protected:
  LVector3 _vector;

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
