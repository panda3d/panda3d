// Filename: physicalNode.h
// Created by:  charles (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PHYSICALNODE_H
#define PHYSICALNODE_H

#include <pandabase.h>
#include <namedNode.h>

#include <vector>

#include "physical.h"
#include "config_physics.h"

////////////////////////////////////////////////////////////////////
//        Class : PhysicalNode
//  Description : Graph node that encapsulated a series of physical
//                objects
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PhysicalNode : public NamedNode {
private:
  vector< PT(Physical) > _physicals;

PUBLISHED:
  PhysicalNode(const string &name = "");
  PhysicalNode(const PhysicalNode &copy);
  virtual ~PhysicalNode(void);

  PhysicalNode &operator =(const PhysicalNode &copy);

  virtual bool safe_to_flatten(void) const { return false; }
  virtual Node *make_copy(void) const;

  INLINE void clear(void);
  INLINE Physical *get_physical(int index) const;
  INLINE int get_num_physicals(void) const;
  INLINE void add_physical(Physical *physical);

  void add_physicals_from(const PhysicalNode &other);
  void remove_physical(Physical *physical);
  void remove_physical(int index);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    NamedNode::init_type();
    register_type(_type_handle, "PhysicalNode",
                  NamedNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "physicalNode.I"

#endif // PHYSICALNODE_H
