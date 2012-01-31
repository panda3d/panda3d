// Filename: collisionNode.h
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONNODE_H
#define COLLISIONNODE_H

#include "pandabase.h"

#include "collisionSolid.h"

#include "collideMask.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : CollisionNode
// Description : A node in the scene graph that can hold any number of
//               CollisionSolids.  This may either represent a bit of
//               static geometry in the scene that things will collide
//               with, or an animated object twirling around in the
//               world and running into things.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_COLLIDE CollisionNode : public PandaNode {
PUBLISHED:
  CollisionNode(const string &name);

protected:
  CollisionNode(const CollisionNode &copy);

public:
  virtual ~CollisionNode();
  virtual PandaNode *make_copy() const;
  virtual bool preserve_name() const;
  virtual void xform(const LMatrix4 &mat);
  virtual PandaNode *combine_with(PandaNode *other); 
  virtual CollideMask get_legal_collide_mask() const;

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;
  virtual bool is_collision_node() const;

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_collide_mask(CollideMask mask);
  void set_from_collide_mask(CollideMask mask);
  INLINE void set_into_collide_mask(CollideMask mask);
  INLINE CollideMask get_from_collide_mask() const;
  INLINE CollideMask get_into_collide_mask() const;

  INLINE void clear_solids();
  INLINE int get_num_solids() const;
  INLINE CPT(CollisionSolid) get_solid(int n) const;
  MAKE_SEQ(get_solids, get_num_solids, get_solid);
  INLINE PT(CollisionSolid) modify_solid(int n);
  INLINE void set_solid(int n, CollisionSolid *solid);
  INLINE void remove_solid(int n);
  INLINE int add_solid(const CollisionSolid *solid);

  INLINE int get_collider_sort() const;
  INLINE void set_collider_sort(int sort);

  INLINE static CollideMask get_default_collide_mask();

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

private:
  CPT(RenderState) get_last_pos_state();

  // This data is not cycled, for now.  We assume the collision
  // traversal will take place in App only.  Perhaps we will revisit
  // this later.
  CollideMask _from_collide_mask;
  int _collider_sort;

  typedef pvector< COWPT(CollisionSolid) > Solids;
  Solids _solids;
  
public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "CollisionNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionNode.I"

#endif
