// Filename: collisionHandler.h
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONHANDLER_H
#define COLLISIONHANDLER_H

#include <pandabase.h>

#include <typedReferenceCount.h>

class CollisionEntry;

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionHandler
// Description : The abstract interface to a number of classes that
//               decide what to do what a collision is detected.  One
//               of these must be assigned to the CollisionTraverser
//               that is processing collisions in order to specify how
//               to dispatch detected collisions.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandler : public TypedReferenceCount {
public:
  virtual void begin_group();
  virtual void add_entry(CollisionEntry *entry);
  virtual void end_group();


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CollisionHandler",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

  friend class CollisionTraverser;
};

#endif
  
  

