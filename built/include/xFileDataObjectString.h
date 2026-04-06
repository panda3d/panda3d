/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObjectString.h
 * @author drose
 * @date 2004-10-08
 */

#ifndef XFILEDATAOBJECTSTRING_H
#define XFILEDATAOBJECTSTRING_H

#include "pandatoolbase.h"
#include "xFileDataObject.h"

/**
 * An string-valued data element.  This matches one string data member of a
 * template, or a single element of an string array.
 */
class XFileDataObjectString : public XFileDataObject {
public:
  XFileDataObjectString(const XFileDataDef *data_def, const std::string &value);

  virtual void output_data(std::ostream &out) const;
  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual void set_string_value(const std::string &string_value);
  virtual std::string get_string_value() const;

private:
  void enquote_string(std::ostream &out) const;

  std::string _value;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    XFileDataObject::init_type();
    register_type(_type_handle, "XFileDataObjectString",
                  XFileDataObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "xFileDataObjectString.I"

#endif
