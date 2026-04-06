/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltUnsupportedRecord.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTUNSUPPORTEDRECORD_H
#define FLTUNSUPPORTEDRECORD_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "datagram.h"

/**
 *
 */
class FltUnsupportedRecord : public FltRecord {
public:
  FltUnsupportedRecord(FltHeader *header);

  virtual void output(std::ostream &out) const;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

private:
  FltOpcode _opcode;
  Datagram _datagram;

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
    register_type(_type_handle, "FltUnsupportedRecord",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#endif
