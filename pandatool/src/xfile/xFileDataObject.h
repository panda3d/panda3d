/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileDataObject.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILEDATAOBJECT_H
#define XFILEDATAOBJECT_H

#include "pandatoolbase.h"
#include "referenceCount.h"
#include "pointerTo.h"
#include "dcast.h"
#include "luse.h"

class XFile;
class XFileDataDef;

/**
 * The abstract base class for a number of different types of data elements
 * that may be stored in the X file.
 */
class XFileDataObject : virtual public ReferenceCount {
public:
  INLINE XFileDataObject(const XFileDataDef *data_def = nullptr);
  virtual ~XFileDataObject();

  INLINE const XFileDataDef *get_data_def() const;

  virtual bool is_complex_object() const;
  virtual std::string (get_type_name)() const;

  INLINE void operator = (int int_value);
  INLINE void operator = (double double_value);
  INLINE void operator = (const std::string &string_value);
  INLINE void operator = (const LVecBase2d &vec);
  INLINE void operator = (const LVecBase3d &vec);
  INLINE void operator = (const LVecBase4d &vec);
  INLINE void operator = (const LMatrix4d &mat);

  INLINE void set(int int_value);
  INLINE void set(double double_value);
  INLINE void set(const std::string &string_value);
  INLINE void set(const LVecBase2d &vec);
  INLINE void set(const LVecBase3d &vec);
  INLINE void set(const LVecBase4d &vec);
  INLINE void set(const LMatrix4d &mat);

  INLINE int i() const;
  INLINE double d() const;
  INLINE std::string s() const;
  INLINE LVecBase2d vec2() const;
  INLINE LVecBase3d vec3() const;
  INLINE LVecBase4d vec4() const;
  INLINE LMatrix4d mat4() const;

  INLINE int size() const;
  INLINE const XFileDataObject &operator [] (int n) const;
  INLINE const XFileDataObject &operator [] (const std::string &name) const;

  INLINE XFileDataObject &operator [] (int n);
  INLINE XFileDataObject &operator [] (const std::string &name);

  // The following methods can be used to add elements of a specific type to a
  // complex object, e.g.  an array or a template object.

  XFileDataObject &add_int(int int_value);
  XFileDataObject &add_double(double double_value);
  XFileDataObject &add_string(const std::string &string_value);

  // The following methods can be used to add elements of a specific type,
  // based on one of the standard templates.

  XFileDataObject &add_Vector(XFile *x_file, const LVecBase3d &vector);
  XFileDataObject &add_MeshFace(XFile *x_file);
  XFileDataObject &add_IndexedColor(XFile *x_file, int index,
                                    const LColor &color);
  XFileDataObject &add_Coords2d(XFile *x_file, const LVecBase2d &coords);

public:
  virtual bool add_element(XFileDataObject *element);

  virtual void output_data(std::ostream &out) const;
  virtual void write_data(std::ostream &out, int indent_level,
                          const char *separator) const;

protected:
  virtual void set_int_value(int int_value);
  virtual void set_double_value(double double_value);
  virtual void set_string_value(const std::string &string_value);
  void store_double_array(int num_elements, const double *values);

  virtual int get_int_value() const;
  virtual double get_double_value() const;
  virtual std::string get_string_value() const;
  void get_double_array(int num_elements, double *values) const;

  virtual int get_num_elements() const;
  virtual XFileDataObject *get_element(int n);
  virtual XFileDataObject *get_element(const std::string &name);

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

INLINE std::ostream &operator << (std::ostream &out, const XFileDataObject &data_object);

#include "xFileDataObject.I"

#endif
