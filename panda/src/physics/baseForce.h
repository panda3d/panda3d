// Filename: baseForce.h
// Created by:  charles (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BASEFORCE_H
#define BASEFORCE_H

#include <pandabase.h>
#include <typedReferenceCount.h>
#include <luse.h>

#include "physicsObject.h"

class ForceNode;

////////////////////////////////////////////////////////////////////
//        Class : BaseForce
//  Description : pure virtual base class for all forces that could
//                POSSIBLY exist.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseForce : public TypedReferenceCount {
private:
  ForceNode *_force_node;
  bool _active;

  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

protected:
  BaseForce(bool active = true);
  BaseForce(const BaseForce &copy);

PUBLISHED:
  virtual ~BaseForce(void);

  INLINE bool get_active(void) const;
  INLINE void set_active(bool active);
  virtual bool is_linear(void) const = 0;

  INLINE ForceNode *get_force_node(void) const;

  virtual LVector3f get_vector(const PhysicsObject *po) = 0;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BaseForce",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ForceNode;
};

#include "baseForce.I"

#endif // BASEFORCE_H
