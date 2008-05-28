// Filename: writableConfigurable.h
// Created by:  jason (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

//
#ifndef WRITABLECONFIGURABLE_H
#define WRITABLECONFIGURABLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#include "typedWritable.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : WritableConfigurable
// Description : Defined as a fix to allow creating Configurable and
//               Writable objects.  Otherwise the compiler gets
//               confused since both TypedWritable and Configurable
//               inherit from TypedObject.
//
//               An object that has data or parameters that are set
//               less frequently (at least occasionally) than every
//               frame.  We can cache the configuration info by
//               by using the "dirty" flag.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL WritableConfigurable : public TypedWritable {

public:
  WritableConfigurable( void ) { make_dirty(); }
  virtual void config( void ) { _dirty = false; }
  INLINE void check_config() const {
    if (_dirty) {
      // This is a sneaky trick to allow check_config() to be called
      // from a const member function.  Even though we will be calling
      // config(), a non-const function that modifies the class
      // object, in some sense it's not really modifying the class
      // object--it's just updating a few internal settings for
      // consistency.
      ((WritableConfigurable *)this)->config();
    }
  }

  INLINE bool is_dirty() const { return _dirty; }
  INLINE void make_dirty() { _dirty = true; }

private:
  bool _dirty;

public:
  virtual void write_datagram(BamWriter*, Datagram&) = 0;

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedWritable::init_type();
    register_type(_type_handle, "WritableConfigurable",
                  TypedWritable::get_class_type());
    TypeRegistry::ptr()->record_alternate_name(_type_handle,
                                               "WriteableConfigurable");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;
};

#endif
