// Filename: collisionNode.h
// Created by:  drose (16Mar02)
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
class EXPCL_PANDA CollisionNode : public PandaNode {
PUBLISHED:
  CollisionNode(const string &name);

protected:
  CollisionNode(const CollisionNode &copy);

public:
  virtual ~CollisionNode();
  virtual PandaNode *make_copy() const;
  virtual bool preserve_name() const;
  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *combine_with(PandaNode *other); 

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_collide_mask(CollideMask mask);
  INLINE void set_from_collide_mask(CollideMask mask);
  INLINE void set_into_collide_mask(CollideMask mask);
  INLINE CollideMask get_from_collide_mask() const;
  INLINE CollideMask get_into_collide_mask() const;

  INLINE void set_collide_geom(bool flag);
  INLINE bool get_collide_geom() const;

  INLINE int get_num_solids() const;
  INLINE CollisionSolid *get_solid(int n) const;
  INLINE void remove_solid(int n);
  INLINE int add_solid(CollisionSolid *solid);

protected:
  virtual BoundingVolume *recompute_bound();
  virtual BoundingVolume *recompute_internal_bound();

private:
  CPT(RenderState) get_last_pos_state();

  // This data is not cycled, for now.  We assume the collision
  // traversal will take place in App only.  Perhaps we will revisit
  // this later.
  CollideMask _from_collide_mask;
  CollideMask _into_collide_mask;

  enum Flags {
    F_collide_geom = 0x0001,
    // Presently only 8 bits are written to the bam file.
  };
  int _flags;

  typedef pvector< PT(CollisionSolid) > Solids;
  Solids _solids;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    PandaNode::init_type();
    register_type(_type_handle, "CollisionNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "collisionNode.I"

#endif
