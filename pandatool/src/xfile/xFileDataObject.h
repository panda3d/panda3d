// Filename: xFileDataObject.h
// Created by:  drose (03Oct04)
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

#ifndef XFILEDATAOBJECT_H
#define XFILEDATAOBJECT_H

#include "pandatoolbase.h"
#include "xFileNode.h"
#include "xFileDataDef.h"
#include "pointerTo.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//       Class : XFileDataObject
// Description : The abstract base class for a number of different
//               types of data elements that may be stored in the X
//               file.
////////////////////////////////////////////////////////////////////
class XFileDataObject : public XFileNode {
public:
  INLINE XFileDataObject(XFile *x_file, const string &name);

  INLINE XFileDataDef *get_data_def() const;

  INLINE int i() const;
  INLINE double d() const;
  INLINE string s() const;
  INLINE operator int () const;
  INLINE operator double () const;
  INLINE operator string () const;

  INLINE const XFileDataObject &operator [] (int n) const;
  INLINE const XFileDataObject &operator [] (const string &name) const;

protected:
  virtual int as_integer_value() const;
  virtual double as_double_value() const;
  virtual string as_string_value() const;

  PT(XFileDataDef) _data_def;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileNode::init_type();
    register_type(_type_handle, "XFileDataObject",
                  XFileNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObject.I"

#endif
  


