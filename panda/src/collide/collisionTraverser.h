// Filename: collisionTraverser.h
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

#ifndef COLLISIONTRAVERSER_H
#define COLLISIONTRAVERSER_H

#include "pandabase.h"

#include "collisionHandler.h"
#include "collisionLevelState.h"

#include "pointerTo.h"
#include "pStatCollector.h"

#include "pset.h"

class CollisionNode;
class CollisionRecorder;
class CollisionVisualizer;
class Geom;
class NodePath;
class CollisionEntry;

////////////////////////////////////////////////////////////////////
//       Class : CollisionTraverser
// Description : This class manages the traversal through the scene
//               graph to detect collisions.  It holds ownership of a
//               number of collider objects, each of which is a
//               CollisionNode and an associated CollisionHandler.
//
//               When traverse() is called, it begins at the indicated
//               root and detects all collisions with any of its
//               collider objects against nodes at or below the
//               indicated root, calling the appropriate
//               CollisionHandler for each detected collision.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CollisionTraverser {
PUBLISHED:
  CollisionTraverser();
  ~CollisionTraverser();

  INLINE void set_respect_prev_transform(bool flag);
  INLINE bool get_respect_prev_transform() const;

  void add_collider(const NodePath &collider, CollisionHandler *handler);
  bool remove_collider(const NodePath &collider);
  bool has_collider(const NodePath &collider) const;
  int get_num_colliders() const;
  NodePath get_collider(int n) const;
  CollisionHandler *get_handler(const NodePath &collider) const;
  void clear_colliders();

  // The following methods are deprecated and exist only as a temporary
  // transition to the above new NodePath-based methods.
  void add_collider(CollisionNode *node, CollisionHandler *handler);
  bool remove_collider(CollisionNode *node);

  void traverse(const NodePath &root);
  void reset_prev_transform(const NodePath &root);

#ifdef DO_COLLISION_RECORDING
  void set_recorder(CollisionRecorder *recorder);
  INLINE bool has_recorder() const;
  INLINE CollisionRecorder *get_recorder() const;
  INLINE void clear_recorder();

  CollisionVisualizer *show_collisions(const NodePath &root);
  void hide_collisions();
#endif  // DO_COLLISION_RECORDING

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

private:
  void prepare_colliders(CollisionLevelState &state, const NodePath &root);

  void r_traverse(CollisionLevelState &level_state);

  void compare_collider_to_node(CollisionEntry &entry,
                                const GeometricBoundingVolume *from_parent_gbv,
                                const GeometricBoundingVolume *from_node_gbv,
                                const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_geom_node(CollisionEntry &entry,
                                     const GeometricBoundingVolume *from_parent_gbv,
                                     const GeometricBoundingVolume *from_node_gbv,
                                     const GeometricBoundingVolume *into_node_gbv);
  void compare_collider_to_solid(CollisionEntry &entry,
                                 const GeometricBoundingVolume *from_node_gbv,
                                 const GeometricBoundingVolume *solid_gbv);
  void compare_collider_to_geom(CollisionEntry &entry, Geom *geom,
                                const GeometricBoundingVolume *from_node_gbv,
                                const GeometricBoundingVolume *solid_gbv);

  void r_reset_prev_transform(PandaNode *node);

private:
  PT(CollisionHandler) _default_handler;
  TypeHandle _graph_type;

  class OrderedColliderDef {
  public:
    NodePath _node_path;
    bool _in_graph;
  };

  typedef pmap<NodePath,  PT(CollisionHandler) > Colliders;
  Colliders _colliders;
  typedef pvector<OrderedColliderDef> OrderedColliders;
  OrderedColliders _ordered_colliders;

  typedef pmap<PT(CollisionHandler), int> Handlers;
  Handlers _handlers;

  Handlers::iterator remove_handler(Handlers::iterator hi);

  bool _respect_prev_transform;
#ifdef DO_COLLISION_RECORDING
  CollisionRecorder *_recorder;
  NodePath _collision_visualizer_np;
#endif  // DO_COLLISION_RECORDING

  // Statistics
  static PStatCollector _collisions_pcollector;
};

INLINE ostream &operator << (ostream &out, const CollisionTraverser &trav) {
  trav.output(out);
  return out;
}

#include "collisionTraverser.I"

#endif

