/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectArray.h
 * @author drose
 * @date 2004-10-07
 */

#ifndef XFILEDATAOBJECTARRAY_H
#define XFILEDATAOBJECTARRAY_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

/**
 * An array of nested data elements.
 */
class XFileDataObjectArray : public XFileDataObject {
public:
  INLINE XFileDataObjectArray(const XFileDataDef *data_def);

  virtual bool is_complex_object() const;

  virtual bool add_element(XFileDataObject *element);

  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual int get_num_elements() const;
  virtual XFileDataObject *get_element(int n);

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
