// Filename: collisionNode.h
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONNODE_H
#define COLLISIONNODE_H

#include <pandabase.h>

#include "collisionSolid.h"
#include "collideMask.h"

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
public:
  CollisionNode(const string &name = "");
  CollisionNode(const CollisionNode &copy);
  void operator = (const CollisionNode &copy);
  virtual ~CollisionNode();

  virtual Node *make_copy() const;
  virtual void xform(const LMatrix4f &mat);

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

  virtual void draw_traverse();
  virtual void output(ostream &out) const;

protected:
  virtual void recompute_bound();

private:
  CollideMask _from_collide_mask;
  CollideMask _into_collide_mask;
  bool _collide_geom;

  typedef vector<PT(CollisionSolid)> Solids;
  Solids _solids;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  
  virtual int complete_pointers(vector_typedWriteable &plist, 
				BamReader *manager);

  static TypedWriteable *make_CollisionNode(const FactoryParams &params);

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
