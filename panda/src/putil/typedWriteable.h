// Filename: typedWriteable.h
// Created by:  jason (08Jun00)
//

#ifndef __TYPED_WRITEABLE_
#define __TYPED_WRITEABLE_

#include "typeHandle.h"
#include "writeable.h"
#include "vector_typedWriteable.h"

class BamReader;

////////////////////////////////////////////////////////////////////
// 	 Class : TypedWriteable
// Description : Convience class to not have to derive from TypedObject
//               and writeable all the time
//
// Important   : Every class derived from TypedWriteable that is
//               not an abstract class, MUST define a factory method
//               for creating and object of that type.  This method
//               must be static.  This method must be of the form
//               static <class type*> make_<appropriate name>(const FactoryParams &)
//               Also, in the config file for each package, make sure
//               to add in code to the ConfigureFN to register the
//               creation function with BamReader's factory.
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA TypedWriteable : public TypedObject, public Writeable {
public:
  static TypedWriteable* const Null;

  INLINE TypedWriteable();
  INLINE TypedWriteable(const TypedWriteable &copy);
  INLINE void operator = (const TypedWriteable &copy);

  virtual ~TypedWriteable();

  //The essential virtual function interface to define
  //how any writeable object, writes itself to a datagram
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
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);


protected:
  //This interface function is written here, as a suggestion
  //for a function to write in any class that will have children
  //that are also TypedWriteable.  To encourage code re-use
  
  //virtual void fillin(TypedWriteable*, DatagramIterator&, BamReader *);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    Writeable::init_type();
    register_type(_type_handle, "TypedWriteable", 
    TypedObject::get_class_type(),
    Writeable::get_class_type());
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

#include "typedWriteable.I"

#endif


