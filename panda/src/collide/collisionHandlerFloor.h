// Filename: collisionHandlerFloor.h
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONHANDLERFLOOR_H
#define COLLISIONHANDLERFLOOR_H

#include <pandabase.h>

#include "collisionHandlerPhysical.h"
#include "collisionNode.h"

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionHandlerFloor
// Description : A specialized kind of CollisionHandler that sets the
//               Z height of the collider to a fixed linear offset
//               from the highest detected collision point each frame.
//               It's intended to implement walking around on a floor
//               of varying height by casting a ray down from the
//               avatar's head.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionHandlerFloor : public CollisionHandlerPhysical {
public:
  CollisionHandlerFloor();
  virtual ~CollisionHandlerFloor();

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;

protected:
  virtual void handle_entries();

private:
  float _offset;


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CollisionHandlerPhysical::init_type();
    register_type(_type_handle, "CollisionHandlerFloor",
		  CollisionHandlerPhysical::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "collisionHandlerFloor.I"

#endif
  
  

