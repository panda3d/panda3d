// Filename: collisionSolid.h
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONSOLID_H
#define COLLISIONSOLID_H

#include <pandabase.h>

#include <typedWritableReferenceCount.h>
#include <boundedObject.h>
#include <luse.h>
#include <nodeRelation.h>
#include <pointerTo.h>
#include <node.h>
#include <vector_PT_NodeRelation.h>

class CollisionHandler;
class CollisionEntry;
class CollisionSphere;
class Node;
class GeomNode;
class CollisionNode;

///////////////////////////////////////////////////////////////////
// 	 Class : CollisionSolid
// Description : The abstract base class for all things that can
//               collide with other things in the world, and all the
//               things they can collide with (except geometry).
//
//               This class and its derivatives really work very
//               similarly to the way BoundingVolume and all of its
//               derivatives work.  There's a different subclass for
//               each basic shape of solid, and double-dispatch
//               function calls handle the subset of the N*N
//               intersection tests that we care about.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionSolid : 
  public TypedWritableReferenceCount, public BoundedObject {
public:
  CollisionSolid();
  CollisionSolid(const CollisionSolid &copy);
  virtual ~CollisionSolid();

  virtual CollisionSolid *make_copy()=0;
  virtual LPoint3f get_collision_origin() const=0;

PUBLISHED:
  INLINE void set_tangible(bool tangible);
  INLINE bool is_tangible() const;

public:
  virtual int
  test_intersection(CollisionHandler *record,
		    const CollisionEntry &entry,
		    const CollisionSolid *into) const=0;

  virtual void xform(const LMatrix4f &mat)=0;

  void update_viz(Node *parent);

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int
  test_intersection_from_sphere(CollisionHandler *record,
				const CollisionEntry &entry) const;
  virtual int
  test_intersection_from_ray(CollisionHandler *record,
			     const CollisionEntry &entry) const;
  virtual int
  test_intersection_from_segment(CollisionHandler *record,
				 const CollisionEntry &entry) const;

  static void
  report_undefined_intersection_test(TypeHandle from_type,
				     TypeHandle into_type);

  INLINE void mark_viz_stale();
  void clear_viz_arcs();
  void add_solid_viz(Node *parent, GeomNode *viz);
  void add_wireframe_viz(Node *parent, GeomNode *viz);
  void add_other_viz(Node *parent, GeomNode *viz);

  virtual void recompute_viz(Node *parent)=0;
  
  typedef vector_PT_NodeRelation VizArcs;
  VizArcs _solid_viz_arcs;
  VizArcs _wireframe_viz_arcs;
  VizArcs _other_viz_arcs;
  bool _viz_stale;
  bool _tangible;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "CollisionSolid",
		  TypedWritableReferenceCount::get_class_type(),
		  BoundedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CollisionSphere;
  friend class CollisionRay;
  friend class CollisionSegment;
};

INLINE ostream &operator << (ostream &out, const CollisionSolid &cs) {
  cs.output(out);
  return out;
}

#include "collisionSolid.I"

#endif

