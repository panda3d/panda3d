// Filename: writeableConfigurable.h
// Created by:  jason (19Jun00)
//

//
#ifndef WRITEABLECONFIGURABLE_H
#define WRITEABLECONFIGURABLE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "typedWriteable.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : WriteableConfigurable
// Description : Defined as a fix to allow creating Configurable and 
//               Writeable objects.  Otherwise the compilter gets
//               confused since both TypedWriteable and Configurable 
//               inherit from TypedObject  
//
//               An object that has data or parameters that are set
//		 less frequently (at least occasionally) than every
//		 frame.  We can cache the configuration info by
//		 by using the "dirty" flag. 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA WriteableConfigurable : public TypedWriteable {

public:
  WriteableConfigurable( void ) { make_dirty(); }
  virtual void config( void ) { _dirty = false; }
  INLINE void check_config() const {
    if (_dirty) {
      // This is a sneaky trick to allow check_config() to be called
      // from a const member function.  Even though we will be calling
      // config(), a non-const function that modifies the class
      // object, in some sense it's not really modifying the class
      // object--it's just updating a few internal settings for
      // consistency.
      ((WriteableConfigurable *)this)->config();
    }
  }

  INLINE bool is_dirty(void) const { return _dirty; }
  INLINE void make_dirty(void) { _dirty = true; }

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
    TypedWriteable::init_type();
    register_type(_type_handle, "WriteableConfigurable",
		  TypedWriteable::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;
};

#endif
