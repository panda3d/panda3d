// Filename: angularForce.h
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

#ifndef ANGULARFORCE_H
#define ANGULARFORCE_H

#include "baseForce.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularForce
// Description : pure virtual parent of all quat-based forces.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularForce : public BaseForce {
PUBLISHED:
  virtual ~AngularForce();

  virtual AngularForce *make_copy() const = 0;
  LVector3f get_vector(const PhysicsObject *po);
  virtual bool is_linear() const;
  
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, unsigned int indent=0) const;

protected:
  AngularForce();
  AngularForce(const AngularForce &copy);

private:
  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    BaseForce::init_type();
    register_type(_type_handle, "AngularForce",
                  BaseForce::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // ANGULARFORCE_H
