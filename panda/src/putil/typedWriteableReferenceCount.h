// Filename: typedWriteableReferenceCount.h
// Created by:  jason (08Jun00)
//

#ifndef TYPEDWRITEABLEREFERENCECOUNT_H
#define TYPEDWRITEABLEREFERENCECOUNT_H

#include <pandabase.h>

#include "typedWriteable.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
// 	 Class : TypedWriteableReferenceCount
// Description : A base class for things which need to inherit from
//               both TypedWriteable and from ReferenceCount.  It's
//               convenient to define this intermediate base class
//               instead of multiply inheriting from the two classes
//               each time they are needed, so that we can sensibly
//               pass around pointers to things which are both
//               TypedWriteables and ReferenceCounters.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TypedWriteableReferenceCount : public TypedWriteable, public ReferenceCount {
public:
  INLINE TypedWriteableReferenceCount();
  INLINE TypedWriteableReferenceCount(const TypedWriteableReferenceCount &copy);
  INLINE void operator = (const TypedWriteableReferenceCount &copy);

public:
  virtual void write_datagram(BamWriter *, Datagram &) = 0; 

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWriteable::init_type();
    ReferenceCount::init_type();
    register_type(_type_handle, "TypedWriteableReferenceCount",
		  TypedWriteable::get_class_type(),
		  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "typedWriteableReferenceCount.I"

#endif
