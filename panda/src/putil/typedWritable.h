// Filename: typedWritable.h
// Created by:  jason (08Jun00)
//

#ifndef __TYPED_WRITABLE_
#define __TYPED_WRITABLE_

#include "typeHandle.h"
#include "writable.h"
#include "vector_typedWritable.h"

class BamReader;

////////////////////////////////////////////////////////////////////
// 	 Class : TypedWritable
// Description : Convience class to not have to derive from TypedObject
//               and writable all the time
//
// Important   : Every class derived from TypedWritable that is
//               not an abstract class, MUST define a factory method
//               for creating and object of that type.  This method
//               must be static.  This method must be of the form
//               static <class type*> make_<appropriate name>(const FactoryParams &)
//               Also, in the config file for each package, make sure
//               to add in code to the ConfigureFN to register the
//               creation function with BamReader's factory.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA TypedWritable : public TypedObject, public Writable {
public:
  static TypedWritable* const Null;

  INLINE TypedWritable();
  INLINE TypedWritable(const TypedWritable &copy);
  INLINE void operator = (const TypedWritable &copy);

  virtual ~TypedWritable();

  //The essential virtual function interface to define
  //how any writable object, writes itself to a datagram
  virtual void write_datagram(BamWriter *, Datagram &) = 0; 

  //This function is the interface through which BamReader is
  //able to pass the completed object references into each
  //object that requested them.
  //In other words, if an object (when it is creating itself
  //from a datagram) requests BamReader to generate an object
  //that this object points to, this is the function that is
  //eventually called by BamReader to complete those references.
  //Return the number of pointers read.  This is useful for when
  //a parent reads in a variable number of pointers, so the child
  //knows where to start reading from.
  virtual int complete_pointers(vector_typedWritable &plist, 
				BamReader *manager);


protected:
  //This interface function is written here, as a suggestion
  //for a function to write in any class that will have children
  //that are also TypedWritable.  To encourage code re-use
  
  //virtual void fillin(TypedWritable*, DatagramIterator&, BamReader *);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    TypedObject::init_type();
    Writable::init_type();
    register_type(_type_handle, "TypedWritable", 
		  TypedObject::get_class_type(),
		  Writable::get_class_type());
    TypeRegistry::ptr()->record_alternate_name(_type_handle, "TypedWriteable");
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "typedWritable.I"

#endif


