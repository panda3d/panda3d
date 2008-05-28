// Filename: fltInstanceDefinition.h
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

#ifndef FLTINSTANCEDEFINITION_H
#define FLTINSTANCEDEFINITION_H

#include "pandatoolbase.h"

#include "fltBead.h"

////////////////////////////////////////////////////////////////////
//       Class : FltInstanceDefinition
// Description : This special kind of record marks the top node of an
//               instance subtree.  This subtree lives outside of the
//               normal hierarchy, and is MultiGen's way of supporting
//               instancing--each instance subtree has a unique index,
//               which may be referenced in a FltInstanceRef object to
//               make the instance appear in various places in the
//               hierarchy.
////////////////////////////////////////////////////////////////////
class FltInstanceDefinition : public FltBead {
public:
  FltInstanceDefinition(FltHeader *header);

  int _instance_index;

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
    FltBead::init_type();
    register_type(_type_handle, "FltInstanceDefinition",
                  FltBead::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltInstanceRef;
  friend class FltRecordWriter;
};

#endif


