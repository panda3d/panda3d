// Filename: baseForce.h
// Created by:  charles (08Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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
PUBLISHED:
  virtual ~BaseForce();

  INLINE bool get_active() const;
  INLINE void set_active(bool active);
  virtual bool is_linear() const = 0;

  INLINE ForceNode *get_force_node() const;

  virtual LVector3f get_vector(const PhysicsObject *po) = 0;
  
  virtual void output(ostream &out, unsigned int indent=0) const;

protected:
  BaseForce(bool active = true);
  BaseForce(const BaseForce &copy);

private:
  ForceNode *_force_node;
  bool _active;

  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "BaseForce",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class ForceNode;
};

#include "baseForce.I"

#endif // BASEFORCE_H
