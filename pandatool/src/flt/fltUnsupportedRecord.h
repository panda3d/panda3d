// Filename: fltUnsupportedRecord.h
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTUNSUPPORTEDRECORD_H
#define FLTUNSUPPORTEDRECORD_H

#include <pandatoolbase.h>

#include "fltRecord.h"

#include <datagram.h>

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


