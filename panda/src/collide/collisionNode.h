// Filename: collisionNode.h
// Created by:  drose (24Apr00)
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

#include <pandabase.h>

#include "collisionSolid.h"

#include <collideMask.h>
#include <namedNode.h>

////////////////////////////////////////////////////////////////////
//       Class : CollisionNode
// Description : A node in the scene graph that can hold any number of
//               CollisionSolids.  This may either represent a bit of
//               static geometry in the scene that things will collide
//               with, or an animated object twirling around in the
//               world and running into things.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionNode : public NamedNode {
PUBLISHED:
  CollisionNode(const string &name = "");

public:
  CollisionNode(const CollisionNode &copy);
  void operator = (const CollisionNode &copy);
  virtual ~CollisionNode();
  virtual Node *make_copy() const;
  virtual void xform(const LMatrix4f &mat);
  virtual Node *combine_with(Node *other); 
  virtual bool preserve_name() const;

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

public:
  virtual void draw_traverse(const ArcChain &chain);
  virtual void output(ostream &out) const;

protected:
  virtual BoundingVolume *recompute_bound();

private:
  CollideMask _from_collide_mask;
  CollideMask _into_collide_mask;
  bool _collide_geom;

  typedef pvector< PT(CollisionSolid) > Solids;
  Solids _solids;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

  static TypedWritable *make_CollisionNode(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    NamedNode::init_type();
    register_type(_type_handle, "CollisionNode",
                  NamedNode::get_class_type());
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
