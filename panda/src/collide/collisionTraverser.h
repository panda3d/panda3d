// Filename: collisionTraverser.h
// Created by:  drose (24Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLLISIONTRAVERSER_H
#define COLLISIONTRAVERSER_H

#include <pandabase.h>

#include "collisionHandler.h"
#include "collisionLevelState.h"

#include <traverserVisitor.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>
#include <pointerTo.h>
#include <renderRelation.h>
#include <pointerTo.h>
#include <pStatCollector.h>

#include <set>

class CollisionNode;
class Geom;
class NodePath;

////////////////////////////////////////////////////////////////////
// 	 Class : CollisionTraverser
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionTraverser : 
  public TraverserVisitor<NullTransitionWrapper, CollisionLevelState> {
PUBLISHED:
  CollisionTraverser(TypeHandle graph_type = RenderRelation::get_class_type());
  ~CollisionTraverser();

  void add_collider(CollisionNode *node, CollisionHandler *handler);
  bool remove_collider(CollisionNode *node);
  bool has_collider(CollisionNode *node) const;
  int get_num_colliders() const;
  CollisionNode *get_collider(int n) const;
  CollisionHandler *get_handler(CollisionNode *node) const;
  void clear_colliders();

  void traverse(Node *root);
  void traverse(const NodePath &root);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  void prepare_colliders(CollisionLevelState &state);

public:  
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the CollisionTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool reached_node(Node *node, NullAttributeWrapper &render_state,
		    CollisionLevelState &level_state);

  bool forward_arc(NodeRelation *arc, NullTransitionWrapper &trans,
		   NullAttributeWrapper &pre, NullAttributeWrapper &post,
		   CollisionLevelState &level_state);

private:
  void compare_collider_to_node(CollisionEntry &entry,
				const GeometricBoundingVolume *from_node_gbv,
				const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_geom_node(CollisionEntry &entry,
				     const GeometricBoundingVolume *from_node_gbv,
				     const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_solid(CollisionEntry &entry,
				 const GeometricBoundingVolume *from_node_gbv,
				 const GeometricBoundingVolume *solid_gbv);
  void compare_collider_to_geom(CollisionEntry &entry, Geom *geom,
				const GeometricBoundingVolume *from_node_gbv,
				const GeometricBoundingVolume *solid_gbv);

private:
  PT(CollisionHandler) _default_handler;
  TypeHandle _graph_type;

  typedef map<PT(CollisionNode),  PT(CollisionHandler) > Colliders;
  Colliders _colliders;
  typedef vector<CollisionNode *> OrderedColliders;
  OrderedColliders _ordered_colliders;

  typedef map<PT(CollisionHandler), int> Handlers;
  Handlers _handlers;

  // Statistics
  static PStatCollector _collisions_pcollector;
};

INLINE ostream &operator << (ostream &out, const CollisionTraverser &trav) {
  trav.output(out);
  return out;
}

#include "collisionTraverser.I"

#endif

