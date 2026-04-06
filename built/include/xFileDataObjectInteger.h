/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectInteger.h
 * @author drose
 * @date 2004-10-07
 */

#ifndef XFILEDATAOBJECTINTEGER_H
#define XFILEDATAOBJECTINTEGER_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

/**
 * An integer-valued data element.  This matches one integer data member of a
 * template, or a single element of an integer array.
 */
class XFileDataObjectInteger : public XFileDataObject {
public:
  XFileDataObjectInteger(const XFileDataDef *data_def, int value);

  virtual void output_data(std::ostream &out) const;
  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual void set_int_value(int int_value);

  virtual int get_int_value() const;
  virtual double get_double_value() const;
  virtual std::string get_string_value() const;

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
