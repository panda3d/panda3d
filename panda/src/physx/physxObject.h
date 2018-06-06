/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxObject.h
 * @author enn0x
 * @date 2009-09-11
 */

#ifndef PHYSXOBJECT_H
#define PHYSXOBJECT_H

#include "pandabase.h"
#include "typedReferenceCount.h"

#ifdef HAVE_PYTHON
#undef _POSIX_C_SOURCE
#include <Python.h>
#endif // HAVE_PYTHON

/**
 *
 */
class EXPCL_PANDAPHYSX PhysxObject : public TypedReferenceCount {

#ifdef HAVE_PYTHON
PUBLISHED:
  INLINE void set_python_tag(const std::string &key, PyObject *value);
  INLINE PyObject *get_python_tag(const std::string &key) const;
  INLINE bool has_python_tag(const std::string &key) const;
  INLINE void clear_python_tag(const std::string &key);
  INLINE bool has_python_tags() const;
#endif // HAVE_PYTHON

PUBLISHED:
  virtual void ls() const = 0;
  virtual void ls(std::ostream &out, int indent_level=0) const = 0;

protected:
  INLINE PhysxObject();
  INLINE ~PhysxObject();

  enum ErrorType {
    ET_empty,
    ET_ok,
    ET_released,
    ET_fail,
  };

  ErrorType _error_type;

#ifdef HAVE_PYTHON
private:
  typedef phash_map<std::string, PyObject *, string_hash> PythonTagData;
  PythonTagData _python_tag_data;
#endif // HAVE_PYTHON

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "PhysxObject",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxObject.I"

#endif // PHYSXOBJECT_H
