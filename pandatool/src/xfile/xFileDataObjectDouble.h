/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectDouble.h
 * @author drose
 * @date 2004-10-07
 */

#ifndef XFILEDATAOBJECTDOUBLE_H
#define XFILEDATAOBJECTDOUBLE_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

/**
 * An double-valued data element.  This matches one double data member of a
 * template, or a single element of an double array.
 */
class XFileDataObjectDouble : public XFileDataObject {
public:
  XFileDataObjectDouble(const XFileDataDef *data_def, double value);

  virtual void output_data(std::ostream &out) const;
  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual void set_int_value(int int_value);
  virtual void set_double_value(double double_value);

  virtual int get_int_value() const;
  virtual double get_double_value() const;
  virtual std::string get_string_value() const;

private:
  double _value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataObjectDouble",
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObjectDouble.I"

#endif
