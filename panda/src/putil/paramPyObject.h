/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paramPyObject.h
 * @author rdb
 * @date 2021-03-01
 */

#ifndef PARAMPYOBJECT_H
#define PARAMPYOBJECT_H

#include "pandabase.h"
#include "paramValue.h"

#ifdef HAVE_PYTHON

#include "py_panda.h"

/**
 * A class object for storing an arbitrary Python object.
 */
class ParamPyObject final : public ParamValueBase {
public:
  INLINE ParamPyObject(PyObject *value);
  virtual ~ParamPyObject();

  INLINE PyObject *get_value() const;

  void output(std::ostream &out) const override;

public:
  PyObject *_value;

public:
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ParamValueBase::init_type();
    register_type(_type_handle, "ParamPyObject",
                  ParamValueBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "paramPyObject.I"

#endif  // HAVE_PYTHON

#endif
