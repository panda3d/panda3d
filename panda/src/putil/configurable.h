// Filename: configurable.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef CONFIGURABLE_H
#define CONFIGURABLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "typedObject.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : Configurable
// Description : An object that has data or parameters that are set
//               less frequently (at least occasionally) than every
//               frame.  We can cache the configuration info by
//               by using the "dirty" flag.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Configurable : public TypedObject {
public:

  Configurable( void ) { make_dirty(); }
  virtual void config( void ) { _dirty = false; }
  INLINE void check_config() const {
    if (_dirty) {
      // This is a sneaky trick to allow check_config() to be called
      // from a const member function.  Even though we will be calling
      // config(), a non-const function that modifies the class
      // object, in some sense it's not really modifying the class
      // object--it's just updating a few internal settings for
      // consistency.
      ((Configurable *)this)->config();
    }
  }

  INLINE bool is_dirty(void) const { return _dirty; }
  INLINE void make_dirty(void) { _dirty = true; }

private:

  bool _dirty;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "Configurable",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;
};

#endif
