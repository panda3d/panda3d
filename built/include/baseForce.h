/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file baseForce.h
 * @author charles
 * @date 2000-08-08
 */

#ifndef BASEFORCE_H
#define BASEFORCE_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "luse.h"
#include "nodePath.h"

#include "physicsObject.h"

class ForceNode;

/**
 * pure virtual base class for all forces that could POSSIBLY exist.
 */
class EXPCL_PANDA_PHYSICS BaseForce : public TypedReferenceCount {
PUBLISHED:
  virtual ~BaseForce();

  INLINE bool get_active() const;
  INLINE void set_active(bool active);
  virtual bool is_linear() const = 0;

  INLINE ForceNode *get_force_node() const;
  INLINE NodePath get_force_node_path() const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level=0) const;

protected:
  BaseForce(bool active = true);
  BaseForce(const BaseForce &copy);

private:
  ForceNode *_force_node;
  NodePath _force_node_path;
  bool _active;

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
