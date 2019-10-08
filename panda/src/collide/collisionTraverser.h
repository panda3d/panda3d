/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionTraverser.h
 * @author drose
 * @date 2002-03-16
 */

#ifndef COLLISIONTRAVERSER_H
#define COLLISIONTRAVERSER_H

#include "pandabase.h"

#include "collisionHandler.h"
#include "collisionLevelState.h"

#include "pointerTo.h"
#include "pStatCollector.h"

#include "pset.h"
#include "register_type.h"

class CollisionNode;
class CollisionRecorder;
class CollisionVisualizer;
class Geom;
class NodePath;
class CollisionEntry;

/**
 * This class manages the traversal through the scene graph to detect
 * collisions.  It holds ownership of a number of collider objects, each of
 * which is a CollisionNode and an associated CollisionHandler.
 *
 * When traverse() is called, it begins at the indicated root and detects all
 * collisions with any of its collider objects against nodes at or below the
 * indicated root, calling the appropriate CollisionHandler for each detected
 * collision.
 */
class EXPCL_PANDA_COLLIDE CollisionTraverser : public Namable {
PUBLISHED:
  explicit CollisionTraverser(const std::string &name = "ctrav");
  ~CollisionTraverser();

  INLINE void set_respect_prev_transform(bool flag);
  INLINE bool get_respect_prev_transform() const;
  MAKE_PROPERTY(respect_prev_transform, get_respect_prev_transform,
                                        set_respect_prev_transform);

  void add_collider(const NodePath &collider, CollisionHandler *handler);
  bool remove_collider(const NodePath &collider);
  bool has_collider(const NodePath &collider) const;
  int get_num_colliders() const;
  NodePath get_collider(int n) const;
  MAKE_SEQ(get_colliders, get_num_colliders, get_collider);
  CollisionHandler *get_handler(const NodePath &collider) const;
  void clear_colliders();
  MAKE_SEQ_PROPERTY(colliders, get_num_colliders, get_collider);

  void traverse(const NodePath &root);

#if defined(DO_COLLISION_RECORDING) || !defined(CPPPARSER)
  void set_recorder(CollisionRecorder *recorder);
  INLINE bool has_recorder() const;
  INLINE CollisionRecorder *get_recorder() const;
  INLINE void clear_recorder();
  MAKE_PROPERTY2(recorder, has_recorder, get_recorder,
                           set_recorder, clear_recorder);

  CollisionVisualizer *show_collisions(const NodePath &root);
  void hide_collisions();
#endif  // DO_COLLISION_RECORDING

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

private:
  typedef pvector<CollisionLevelStateSingle> LevelStatesSingle;
  void prepare_colliders_single(LevelStatesSingle &level_states, const NodePath &root);
  void r_traverse_single(CollisionLevelStateSingle &level_state, size_t pass);

  typedef pvector<CollisionLevelStateDouble> LevelStatesDouble;
  void prepare_colliders_double(LevelStatesDouble &level_states, const NodePath &root);
  void r_traverse_double(CollisionLevelStateDouble &level_state, size_t pass);

  typedef pvector<CollisionLevelStateQuad> LevelStatesQuad;
  void prepare_colliders_quad(LevelStatesQuad &level_states, const NodePath &root);
  void r_traverse_quad(CollisionLevelStateQuad &level_state, size_t pass);

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
  void compare_collider_to_geom(CollisionEntry &entry, const Geom *geom,
                                const GeometricBoundingVolume *from_node_gbv,
                                const GeometricBoundingVolume *solid_gbv);

  PStatCollector &get_pass_collector(int pass);

private:
  PT(CollisionHandler) _default_handler;

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
#else
  CollisionRecorder *_recorder_disabled = nullptr;
  NodePath _collision_visualizer_np_disabled;
#endif  // DO_COLLISION_RECORDING

  // Statistics
  static PStatCollector _collisions_pcollector;

  static PStatCollector _cnode_volume_pcollector;
  static PStatCollector _gnode_volume_pcollector;
  static PStatCollector _geom_volume_pcollector;

  PStatCollector _this_pcollector;
  typedef pvector<PStatCollector> PassCollectors;
  PassCollectors _pass_collectors;
  // pstats category for actual collision detection (vs.  bounding heirarchy
  // collision detection)
  typedef pvector<PStatCollector> SolidCollideCollectors;
  SolidCollideCollectors _solid_collide_collectors;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "CollisionTraverser");
  }

private:
  static TypeHandle _type_handle;

  friend class SortByColliderSort;
};

INLINE std::ostream &operator << (std::ostream &out, const CollisionTraverser &trav) {
  trav.output(out);
  return out;
}

#include "collisionTraverser.I"

#endif
