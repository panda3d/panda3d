// Filename: xFileDataObjectArray.h
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

#ifndef XFILEDATAOBJECTARRAY_H
#define XFILEDATAOBJECTARRAY_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataObjectArray
// Description : An array of nested data elements.
////////////////////////////////////////////////////////////////////
class XFileDataObjectArray : public XFileDataObject {
public:
  INLINE XFileDataObjectArray(const XFileDataDef *data_def);

  virtual void write_data(ostream &out, int indent_level,
                          const char *separator) const;

  virtual bool add_element(XFileDataObject *element);

protected:
  virtual int get_num_elements() const;
  virtual const XFileDataObject *get_element(int n) const;

private:
  typedef pvector< PT(XFileDataObject) > NestedElements;
  NestedElements _nested_elements;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataObjectArray",
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObjectArray.I"

#endif
