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
#include "referenceCount.h"
#include "pointerTo.h"
#include "dcast.h"
#include "luse.h"

class XFile;
class XFileDataDef;

////////////////////////////////////////////////////////////////////
//       Class : XFileDataObject
// Description : The abstract base class for a number of different
//               types of data elements that may be stored in the X
//               file.
////////////////////////////////////////////////////////////////////
class XFileDataObject : virtual public ReferenceCount {
public:
  INLINE XFileDataObject(const XFileDataDef *data_def = NULL);
  virtual ~XFileDataObject();

  INLINE const XFileDataDef *get_data_def() const;

  virtual bool is_complex_object() const;

  INLINE void operator = (int int_value);
  INLINE void operator = (double double_value);
  INLINE void operator = (const string &string_value);

  INLINE int i() const;
  INLINE double d() const;
  INLINE string s() const;

  INLINE int size() const;
  INLINE const XFileDataObject &operator [] (int n) const;
  INLINE const XFileDataObject &operator [] (const string &name) const;

  INLINE XFileDataObject &operator [] (int n);
  INLINE XFileDataObject &operator [] (const string &name);

  // The following methods can be used to add elements of a specific
  // type to a complex object, e.g. an array or a template object.

  XFileDataObject &add_int(int int_value);
  XFileDataObject &add_double(double double_value);
  XFileDataObject &add_string(const string &string_value);

  // The following methods can be used to add elements of a specific
  // type, based on one of the standard templates.

  XFileDataObject &add_Vector(XFile *x_file, const LVecBase3d &vector);
  XFileDataObject &add_MeshFace(XFile *x_file);
  XFileDataObject &add_IndexedColor(XFile *x_file, int index, 
                                    const Colorf &color);
  XFileDataObject &add_Coords2d(XFile *x_file, const LVecBase2d &coords);

public:
  virtual bool add_element(XFileDataObject *element);

  virtual void output_data(ostream &out) const;
  virtual void write_data(ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual void set_int_value(int int_value);
  virtual void set_double_value(double double_value);
  virtual void set_string_value(const string &string_value);

  virtual int get_int_value() const;
  virtual double get_double_value() const;
  virtual string get_string_value() const;

  virtual int get_num_elements() const;
  virtual XFileDataObject *get_element(int n);
  virtual XFileDataObject *get_element(const string &name);

  const XFileDataDef *_data_def;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "XFileDataObject",
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const XFileDataObject &data_object);

#include "xFileDataObject.I"

#endif
  


