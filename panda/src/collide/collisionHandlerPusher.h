// Filename: collisionHandlerPusher.h
// Created by:  drose (25Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONHANDLERPUSHER_H
#define COLLISIONHANDLERPUSHER_H

#include <pandabase.h>

#include "collisionHandlerPhysical.h"
#include "collisionNode.h"

///////////////////////////////////////////////////////////////////
//       Class : CollisionHandlerPusher
// Description : A specialized kind of CollisionHandler that simply
//               pushes back on things that attempt to move into solid
//               walls.  This is the simplest kind of "real-world"
//               collisions you can have.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandlerPusher : public CollisionHandlerPhysical {
PUBLISHED:
  CollisionHandlerPusher();
  virtual ~CollisionHandlerPusher();

  INLINE void set_horizontal(bool flag);
  INLINE bool get_horizontal() const;

protected:
  virtual void handle_entries();

private:
  bool _horizontal;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPhysical::init_type();
    register_type(_type_handle, "CollisionHandlerPusher",
                  CollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerPusher.I"

#endif
  
  

