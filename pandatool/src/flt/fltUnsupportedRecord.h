// Filename: fltUnsupportedRecord.h
// Created by:  drose (24Aug00)
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

#ifndef FLTUNSUPPORTEDRECORD_H
#define FLTUNSUPPORTEDRECORD_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "datagram.h"

////////////////////////////////////////////////////////////////////
//       Class : FltUnsupportedRecord
// Description :
////////////////////////////////////////////////////////////////////
class FltUnsupportedRecord : public FltRecord {
public:
  FltUnsupportedRecord(FltHeader *header);

  virtual void output(ostream &out) const;

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


