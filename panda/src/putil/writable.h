// Filename: writable.h
// Created by:  jason (08Jun00)
//

#ifndef __WRITABLE_
#define __WRITABLE_

#include "vector_writable.h"
#include "typeHandle.h"

class BamWriter;
class Datagram;
class DatagramIterator;
class FactoryParams;

////////////////////////////////////////////////////////////////////
// 	 Class : Writable
// Description : This is an abstract class that all classes which
//               need to write themselves in binary form to some
//               media should inherit from
////////////////////////////////////////////////////////////////////

class EXPCL_PANDA Writable {
public:
  static Writable* const Null;

  INLINE Writable();
  INLINE Writable(const Writable &copy);
  INLINE void operator = (const Writable &copy);

  virtual ~Writable();

  //The essential virtual function interface to define
  //how any writable object, writes itself to a datagram
  virtual void write_datagram(BamWriter *manager, Datagram &me) = 0; 

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
  //virtual int complete_pointers(vector_typedWritable &plist, 
  //                              BamReader *manager) {}


  //When everything else is done, this is the function called
  //by BamReader to perform any final actions needed for setting
  //up the object
  virtual void finalize(void) {}

protected:
  //This interface function is written here, as a suggestion
  //for a function to write in any class that will have children
  //that are also Writable.  To encourage code re-use
  
  //virtual void fillin(DatagramIterator& scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "Writable");
    TypeRegistry::ptr()->record_alternate_name(_type_handle, "Writeable");
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

#include "writable.I"

#endif
