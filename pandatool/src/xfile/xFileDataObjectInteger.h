// Filename: xFileDataObjectInteger.h
// Created by:  drose (07Oct04)
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

#ifndef XFILEDATAOBJECTINTEGER_H
#define XFILEDATAOBJECTINTEGER_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataObjectInteger
// Description : An integer-valued data element.  This matches one
//               integer data member of a template, or a single
//               element of an integer array.
////////////////////////////////////////////////////////////////////
class XFileDataObjectInteger : public XFileDataObject {
public:
  XFileDataObjectInteger(const XFileDataDef *data_def, int value);

  virtual void output_data(ostream &out) const;
  virtual void write_data(ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual int as_integer_value() const;
  virtual double as_double_value() const;
  virtual string as_string_value() const;

private:
  int _value;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataObjectInteger",
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObjectInteger.I"

#endif
