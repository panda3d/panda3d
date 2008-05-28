// Filename: fltInstanceRef.h
// Created by:  drose (30Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FLTINSTANCEREF_H
#define FLTINSTANCEREF_H

#include "pandatoolbase.h"

#include "fltBead.h"

class FltInstanceDefinition;

////////////////////////////////////////////////////////////////////
//       Class : FltInstanceRef
// Description : This bead appears in the hierarchy to refer to a
//               FltInstanceDefinition node defined elsewhere.  It
//               indicates that the subtree beginning at the
//               FltInstanceDefinition should be considered to be
//               instanced here.
////////////////////////////////////////////////////////////////////
class FltInstanceRef : public FltBead {
public:
  FltInstanceRef(FltHeader *header);

  int _instance_index;

  FltInstanceDefinition *get_instance() const;

  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual FltError write_record_and_children(FltRecordWriter &writer) const;
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
    FltBead::init_type();
    register_type(_type_handle, "FltInstanceRef",
                  FltBead::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif


