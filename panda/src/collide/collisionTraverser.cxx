/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionTraverser.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionTraverser.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "collisionGeom.h"
#include "collisionRecorder.h"
#include "collisionVisualizer.h"
#include "collisionSphere.h"
#include "collisionBox.h"
#include "collisionCapsule.h"
#include "collisionPolygon.h"
#include "collisionPlane.h"
#include "config_collide.h"
#include "boundingSphere.h"
#include "transformState.h"
#include "geomNode.h"
#include "geom.h"
#include "geom.h"
#include "geomTriangles.h"
#include "geomVertexReader.h"
#include "lodNode.h"
#include "nodePath.h"
#include "pStatTimer.h"
#include "indent.h"

#include <algorithm>

using std::min;

PStatCollector CollisionTraverser::_collisions_pcollector("App:Collisions");

PStatCollector CollisionTraverser::_cnode_volume_pcollector("Collision Volumes:CollisionNode");
PStatCollector CollisionTraverser::_gnode_volume_pcollector("Collision Volumes:GeomNode");
PStatCollector CollisionTraverser::_geom_volume_pcollector("Collision Volumes:Geom");

TypeHandle CollisionTraverser::_type_handle;

// This function object class is used in prepare_colliders(), below.
class SortByColliderSort {
public:
  SortByColliderSort(const CollisionTraverser &trav) :
    _trav(trav)
  {
  }

  inline bool operator () (int a, int b) const {
    const CollisionTraverser::OrderedColliderDef &ocd_a = _trav._ordered_colliders[a];
    const CollisionTraverser::OrderedColliderDef &ocd_b = _trav._ordered_colliders[b];
    return ((const CollisionNode *)ocd_a._node_path.node())->get_collider_sort() < ((const CollisionNode *)ocd_b._node_path.node())->get_collider_sort();
  }

  const CollisionTraverser &_trav;
};

/**
 *
 */
CollisionTraverser::
CollisionTraverser(const std::string &name) :
  Namable(name),
  _this_pcollector(_collisions_pcollector, name)
{
  _respect_prev_transform = respect_prev_transform;
  #ifdef DO_COLLISION_RECORDING
  _recorder = nullptr;
  #endif
}

/**
 *
 */
CollisionTraverser::
~CollisionTraverser() {
  #ifdef DO_COLLISION_RECORDING
  clear_recorder();
  #endif
}

/**
 * Adds a new CollisionNode, representing an object that will be tested for
 * collisions into other objects, along with the handler that will serve each
 * detected collision.  Each CollisionNode may be served by only one handler
 * at a time, but a given handler may serve many CollisionNodes.
 *
 * The handler that serves a particular node may be changed from time to time
 * by calling add_collider() again on the same node.
 */
void CollisionTraverser::
add_collider(const NodePath &collider, CollisionHandler *handler) {
  nassertv(_ordered_colliders.size() == _colliders.size());
  nassertv(!collider.is_empty() && collider.node()->is_collision_node());
  nassertv(handler != nullptr);

  Colliders::iterator ci = _colliders.find(collider);
  if (ci != _colliders.end()) {
    // We already knew about this collider.
    if ((*ci).second != handler) {
      // Change the handler.
      PT(CollisionHandler) old_handler = (*ci).second;
      (*ci).second = handler;

      // Now update our own reference counts within our handler set.
      Handlers::iterator hi = _handlers.find(old_handler);
      nassertv(hi != _handlers.end());
      (*hi).second--;
      nassertv((*hi).second >= 0);
      if ((*hi).second == 0) {
        _handlers.erase(hi);
      }

      hi = _handlers.find(handler);
      if (hi == _handlers.end()) {
        _handlers.insert(Handlers::value_type(handler, 1));
      } else {
        ++(*hi).second;
      }
    }

  } else {
    // We hadn't already known about this collider.
    _colliders.insert(Colliders::value_type(collider, handler));
    OrderedColliderDef ocdef;
    ocdef._node_path = collider;
    ocdef._in_graph = true;
    _ordered_colliders.push_back(ocdef);

    Handlers::iterator hi = _handlers.find(handler);
    if (hi == _handlers.end()) {
      _handlers.insert(Handlers::value_type(handler, 1));
    } else {
      ++(*hi).second;
    }
  }

  nassertv(_ordered_colliders.size() == _colliders.size());
}

/**
 * Removes the collider (and its associated handler) from the set of
 * CollisionNodes that will be tested each frame for collisions into other
 * objects.  Returns true if the definition was found and removed, false if it
 * wasn't present to begin with.
 */
bool CollisionTraverser::
remove_collider(const NodePath &collider) {
  nassertr(_ordered_colliders.size() == _colliders.size(), false);

  Colliders::iterator ci = _colliders.find(collider);
  if (ci == _colliders.end()) {
    // We didn't know about this node.
    return false;
  }

  CollisionHandler *handler = (*ci).second;

  // Update the set of handlers.
  Handlers::iterator hi = _handlers.find(handler);
  nassertr(hi != _handlers.end(), false);
  (*hi).second--;
  nassertr((*hi).second >= 0, false);
  if ((*hi).second == 0) {
    _handlers.erase(hi);
  }

  _colliders.erase(ci);

  OrderedColliders::iterator oci;
  oci = _ordered_colliders.begin();
  while (oci != _ordered_colliders.end() &&
         (*oci)._node_path != collider) {
    ++oci;
  }

  nassertr(oci != _ordered_colliders.end(), false);
  _ordered_colliders.erase(oci);

  nassertr(_ordered_colliders.size() == _colliders.size(), false);
  return true;
}

/**
 * Returns true if the indicated node is current in the set of nodes that will
 * be tested each frame for collisions into other objects.
 */
bool CollisionTraverser::
has_collider(const NodePath &collider) const {
  Colliders::const_iterator ci = _colliders.find(collider);
  return (ci != _colliders.end());
}

/**
 * Returns the number of CollisionNodes that have been added to the traverser
 * via add_collider().
 */
int CollisionTraverser::
get_num_colliders() const {
  nassertr(_ordered_colliders.size() == _colliders.size(), 0);
  return _ordered_colliders.size();
}

/**
 * Returns the nth CollisionNode that has been added to the traverser via
 * add_collider().
 */
NodePath CollisionTraverser::
get_collider(int n) const {
  nassertr(_ordered_colliders.size() == _colliders.size(), NodePath());
  nassertr(n >= 0 && n < (int)_ordered_colliders.size(), NodePath());
  return _ordered_colliders[n]._node_path;
}

/**
 * Returns the handler that is currently assigned to serve the indicated
 * collision node, or NULL if the node is not on the traverser's set of active
 * nodes.
 */
CollisionHandler *CollisionTraverser::
get_handler(const NodePath &collider) const {
  Colliders::const_iterator ci = _colliders.find(collider);
  if (ci != _colliders.end()) {
    return (*ci).second;
  }
  return nullptr;
}

/**
 * Completely empties the set of collision nodes and their associated
 * handlers.
 */
void CollisionTraverser::
clear_colliders() {
  _colliders.clear();
  _ordered_colliders.clear();
  _handlers.clear();
}

/**
 *
 */
void CollisionTraverser::
traverse(const NodePath &root) {
  PStatTimer timer(_this_pcollector);

  #ifdef DO_COLLISION_RECORDING
  if (has_recorder()) {
    get_recorder()->begin_traversal();
  }
  #endif  // DO_COLLISION_RECORDING

  Handlers::iterator hi;
  for (hi = _handlers.begin(); hi != _handlers.end(); ++hi) {
    if ((*hi).first->wants_all_potential_collidees()) {
      (*hi).first->set_root(root);
    }
    (*hi).first->begin_group();
  }

  bool traversal_done = false;
  if ((int)_colliders.size() <= CollisionLevelStateSingle::get_max_colliders() ||
      !allow_collider_multiple) {
    // Use the single-word-at-a-time traverser, which might need to make lots
    // of passes.
    LevelStatesSingle level_states;
    prepare_colliders_single(level_states, root);

    if (level_states.size() == 1 || !allow_collider_multiple) {
      traversal_done = true;

      // Make a number of passes, one for each group of 32 Colliders (or
      // whatever number of bits we have available in CurrentMask).
      for (size_t pass = 0; pass < level_states.size(); ++pass) {
#ifdef DO_PSTATS
        PStatTimer pass_timer(get_pass_collector(pass));
#endif
        r_traverse_single(level_states[pass], pass);
      }
    }
  }

  if (!traversal_done &&
      (int)_colliders.size() <= CollisionLevelStateDouble::get_max_colliders()) {
    // Try the double-word-at-a-time traverser.
    LevelStatesDouble level_states;
    prepare_colliders_double(level_states, root);

    if (level_states.size() == 1) {
      traversal_done = true;

      for (size_t pass = 0; pass < level_states.size(); ++pass) {
#ifdef DO_PSTATS
        PStatTimer pass_timer(get_pass_collector(pass));
#endif
        r_traverse_double(level_states[pass], pass);
      }
    }
  }

  if (!traversal_done) {
    // OK, do the quad-word-at-a-time traverser.
    LevelStatesQuad level_states;
    prepare_colliders_quad(level_states, root);

    traversal_done = true;

    for (size_t pass = 0; pass < level_states.size(); ++pass) {
#ifdef DO_PSTATS
      PStatTimer pass_timer(get_pass_collector(pass));
#endif
      r_traverse_quad(level_states[pass], pass);
    }
  }

  hi = _handlers.begin();
  while (hi != _handlers.end()) {
    if (!(*hi).first->end_group()) {
      // This handler wants to remove itself from the traversal list.
      hi = remove_handler(hi);
    } else {
      ++hi;
    }
  }

  #ifdef DO_COLLISION_RECORDING
  if (has_recorder()) {
    get_recorder()->end_traversal();
  }
  #endif  // DO_COLLISION_RECORDING

  CollisionLevelStateBase::_node_volume_pcollector.flush_level();
  _cnode_volume_pcollector.flush_level();
  _gnode_volume_pcollector.flush_level();
  _geom_volume_pcollector.flush_level();

  CollisionSphere::flush_level();
  CollisionCapsule::flush_level();
  CollisionPolygon::flush_level();
  CollisionPlane::flush_level();
  CollisionBox::flush_level();
}

#if defined(DO_COLLISION_RECORDING) || !defined(CPPPARSER)
/**
 * Uses the indicated CollisionRecorder object to start recording the
 * intersection tests made by each subsequent call to traverse() on this
 * object.  A particular CollisionRecorder object can only record one
 * traverser at a time; if this object has already been assigned to another
 * traverser, that assignment is broken.
 *
 * This is intended to be used in a debugging mode to try to determine what
 * work is being performed by the collision traversal.  Usually, attaching a
 * recorder will impose significant runtime overhead.
 *
 * This does not transfer ownership of the CollisionRecorder pointer;
 * maintenance of that remains the caller's responsibility.  If the
 * CollisionRecorder is destructed, it will cleanly remove itself from the
 * traverser.
 */
void CollisionTraverser::
set_recorder(CollisionRecorder *recorder) {
#ifdef DO_COLLISION_RECORDING
  if (recorder != _recorder) {
    // Remove the old recorder, if any.
    if (_recorder != nullptr) {
      nassertv(_recorder->_trav == this);
      _recorder->_trav = nullptr;
    }

    _recorder = recorder;

    // Tell the new recorder about his new owner.
    if (_recorder != nullptr) {
      nassertv(_recorder->_trav != this);
      if (_recorder->_trav != nullptr) {
        _recorder->_trav->clear_recorder();
      }
      nassertv(_recorder->_trav == nullptr);
      _recorder->_trav = this;
    }
  }
#endif
}

/**
 * This is a high-level function to create a CollisionVisualizer object to
 * render the collision tests performed by this traverser.  The supplied root
 * should be any node in the scene graph; typically, the top node (e.g.
 * render).  The CollisionVisualizer will be attached to this node.
 */
CollisionVisualizer *CollisionTraverser::
show_collisions(const NodePath &root) {
#ifdef DO_COLLISION_RECORDING
  hide_collisions();
  CollisionVisualizer *viz = new CollisionVisualizer("show_collisions");
  _collision_visualizer_np = root.attach_new_node(viz);
  set_recorder(viz);
  return viz;
#else
  return nullptr;
#endif
}

/**
 * Undoes the effect of a previous call to show_collisions().
 */
void CollisionTraverser::
hide_collisions() {
#ifdef DO_COLLISION_RECORDING
  if (!_collision_visualizer_np.is_empty()) {
    _collision_visualizer_np.remove_node();
  }
  clear_recorder();
#endif
}

#endif  // DO_COLLISION_RECORDING

/**
 *
 */
void CollisionTraverser::
output(std::ostream &out) const {
  out << "CollisionTraverser, " << _colliders.size()
      << " colliders and " << _handlers.size() << " handlers.\n";
}

/**
 *
 */
void CollisionTraverser::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "CollisionTraverser, " << _colliders.size()
    << " colliders and " << _handlers.size() << " handlers:\n";

  OrderedColliders::const_iterator oci;
  for (oci = _ordered_colliders.begin();
       oci != _ordered_colliders.end();
       ++oci) {
    NodePath cnode_path = (*oci)._node_path;
    bool in_graph = (*oci)._in_graph;

    Colliders::const_iterator ci;
    ci = _colliders.find(cnode_path);
    nassertv(ci != _colliders.end());

    CollisionHandler *handler = (*ci).second;
    nassertv(handler != nullptr);

    indent(out, indent_level + 2)
      << cnode_path;
    if (in_graph) {
      out << " handled by " << handler->get_type() << "\n";
    } else {
      out << " ignored\n";
    }

    if (!cnode_path.is_empty() && cnode_path.node()->is_collision_node()) {
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());

      int num_solids = cnode->get_num_solids();
      for (int i = 0; i < num_solids; ++i) {
        cnode->get_solid(i)->write(out, indent_level + 4);
      }
    }
  }
}

/**
 * Fills up the set of LevelStates corresponding to the active colliders in
 * use.
 *
 * This flavor uses a CollisionLevelStateSingle, which is limited to a certain
 * number of colliders per pass (typically 32).
 */
void CollisionTraverser::
prepare_colliders_single(CollisionTraverser::LevelStatesSingle &level_states,
                         const NodePath &root) {
  int num_colliders = _colliders.size();
  int max_colliders = CollisionLevelStateSingle::get_max_colliders();

  CollisionLevelStateSingle level_state(root);
  // This reserve() call is only correct if there is exactly one solid per
  // collider added to the traverser, which is the normal case.  If there is
  // more than one solid in any of the colliders, this reserve() call won't
  // reserve enough, but the code is otherwise correct.
  level_state.reserve(min(num_colliders, max_colliders));

  // Create an indirect index array to walk through the colliders in sorted
  // order, without affect the actual collider order.
  int *indirect = (int *)alloca(sizeof(int) * num_colliders);
  int i;
  for (i = 0; i < num_colliders; ++i) {
    indirect[i] = i;
  }
  std::sort(indirect, indirect + num_colliders, SortByColliderSort(*this));

  int num_remaining_colliders = num_colliders;
  for (i = 0; i < num_colliders; ++i) {
    OrderedColliderDef &ocd = _ordered_colliders[indirect[i]];
    NodePath cnode_path = ocd._node_path;

    if (!cnode_path.is_same_graph(root)) {
      if (ocd._in_graph) {
        // Only report this warning once.
        collide_cat.info()
          << "Collider " << cnode_path
          << " is not in scene graph.  Ignoring.\n";
        ocd._in_graph = false;
      }

    } else {
      ocd._in_graph = true;
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());

      CollisionLevelStateSingle::ColliderDef def;
      def._node = cnode;
      def._node_path = cnode_path;

      int num_solids = cnode->get_num_solids();
      for (int s = 0; s < num_solids; ++s) {
        CPT(CollisionSolid) collider = cnode->get_solid(s);
        def._collider = collider;
        level_state.prepare_collider(def, root);

        if (level_state.get_num_colliders() == max_colliders) {
          // That's the limit.  Save off this level state and make a new one.
          level_states.push_back(level_state);
          level_state.clear();
          level_state.reserve(min(num_remaining_colliders, max_colliders));
        }
      }
    }

    --num_remaining_colliders;
    nassertv(num_remaining_colliders >= 0);
  }

  if (level_state.get_num_colliders() != 0) {
    level_states.push_back(level_state);
  }
  nassertv(num_remaining_colliders == 0);
}

/**
 *
 */
void CollisionTraverser::
r_traverse_single(CollisionLevelStateSingle &level_state, size_t pass) {
  if (!level_state.any_in_bounds()) {
    return;
  }
  if (!level_state.apply_transform()) {
    return;
  }

  PandaNode *node = level_state.node();
  if (node->is_collision_node()) {
    CollisionNode *cnode;
    DCAST_INTO_V(cnode, node);
    CPT(BoundingVolume) node_bv = cnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = cnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             cnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }

  } else if (node->is_geom_node()) {
    #ifndef NDEBUG
    if (collide_cat.is_spam()) {
      collide_cat.spam()
        << "Reached " << *node << "\n";
    }
    #endif

    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);
    CPT(BoundingVolume) node_bv = gnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = gnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             gnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_geom_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }
  }

  if (node->has_single_child_visibility()) {
    // If it's a switch node or sequence node, visit just the one visible
    // child.
    int index = node->get_visible_child();
    if (index >= 0 && index < node->get_num_children()) {
      CollisionLevelStateSingle next_state(level_state, node->get_child(index));
      r_traverse_single(next_state, pass);
    }

  } else if (node->is_lod_node()) {
    // If it's an LODNode, visit the lowest level of detail with all bits,
    // allowing collision with geometry under the lowest level of default; and
    // visit all other levels without GeomNode::get_default_collide_mask(),
    // allowing only collision with CollisionNodes and special geometry under
    // higher levels of detail.
    int index = DCAST(LODNode, node)->get_lowest_switch();
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateSingle next_state(level_state, children.get_child(i));
      if (i != index) {
        next_state.set_include_mask(next_state.get_include_mask() &
          ~GeomNode::get_default_collide_mask());
      }
      r_traverse_single(next_state, pass);
    }

  } else {
    // Otherwise, visit all the children.
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateSingle next_state(level_state, children.get_child(i));
      r_traverse_single(next_state, pass);
    }
  }
}

/**
 * Fills up the set of LevelStates corresponding to the active colliders in
 * use.
 *
 * This flavor uses a CollisionLevelStateDouble, which is limited to a certain
 * number of colliders per pass (typically 32).
 */
void CollisionTraverser::
prepare_colliders_double(CollisionTraverser::LevelStatesDouble &level_states,
                         const NodePath &root) {
  int num_colliders = _colliders.size();
  int max_colliders = CollisionLevelStateDouble::get_max_colliders();

  CollisionLevelStateDouble level_state(root);
  // This reserve() call is only correct if there is exactly one solid per
  // collider added to the traverser, which is the normal case.  If there is
  // more than one solid in any of the colliders, this reserve() call won't
  // reserve enough, but the code is otherwise correct.
  level_state.reserve(min(num_colliders, max_colliders));

  // Create an indirect index array to walk through the colliders in sorted
  // order, without affect the actual collider order.
  int *indirect = (int *)alloca(sizeof(int) * num_colliders);
  int i;
  for (i = 0; i < num_colliders; ++i) {
    indirect[i] = i;
  }
  std::sort(indirect, indirect + num_colliders, SortByColliderSort(*this));

  int num_remaining_colliders = num_colliders;
  for (i = 0; i < num_colliders; ++i) {
    OrderedColliderDef &ocd = _ordered_colliders[indirect[i]];
    NodePath cnode_path = ocd._node_path;

    if (!cnode_path.is_same_graph(root)) {
      if (ocd._in_graph) {
        // Only report this warning once.
        collide_cat.info()
          << "Collider " << cnode_path
          << " is not in scene graph.  Ignoring.\n";
        ocd._in_graph = false;
      }

    } else {
      ocd._in_graph = true;
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());

      CollisionLevelStateDouble::ColliderDef def;
      def._node = cnode;
      def._node_path = cnode_path;

      int num_solids = cnode->get_num_solids();
      for (int s = 0; s < num_solids; ++s) {
        CPT(CollisionSolid) collider = cnode->get_solid(s);
        def._collider = collider;
        level_state.prepare_collider(def, root);

        if (level_state.get_num_colliders() == max_colliders) {
          // That's the limit.  Save off this level state and make a new one.
          level_states.push_back(level_state);
          level_state.clear();
          level_state.reserve(min(num_remaining_colliders, max_colliders));
        }
      }
    }

    --num_remaining_colliders;
    nassertv(num_remaining_colliders >= 0);
  }

  if (level_state.get_num_colliders() != 0) {
    level_states.push_back(level_state);
  }
  nassertv(num_remaining_colliders == 0);
}

/**
 *
 */
void CollisionTraverser::
r_traverse_double(CollisionLevelStateDouble &level_state, size_t pass) {
  if (!level_state.any_in_bounds()) {
    return;
  }
  if (!level_state.apply_transform()) {
    return;
  }

  PandaNode *node = level_state.node();
  if (node->is_collision_node()) {
    CollisionNode *cnode;
    DCAST_INTO_V(cnode, node);
    CPT(BoundingVolume) node_bv = cnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = cnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             cnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }

  } else if (node->is_geom_node()) {
    #ifndef NDEBUG
    if (collide_cat.is_spam()) {
      collide_cat.spam()
        << "Reached " << *node << "\n";
    }
    #endif

    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);
    CPT(BoundingVolume) node_bv = gnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = gnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             gnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_geom_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }
  }

  if (node->has_single_child_visibility()) {
    // If it's a switch node or sequence node, visit just the one visible
    // child.
    int index = node->get_visible_child();
    if (index >= 0 && index < node->get_num_children()) {
      CollisionLevelStateDouble next_state(level_state, node->get_child(index));
      r_traverse_double(next_state, pass);
    }

  } else if (node->is_lod_node()) {
    // If it's an LODNode, visit the lowest level of detail with all bits,
    // allowing collision with geometry under the lowest level of default; and
    // visit all other levels without GeomNode::get_default_collide_mask(),
    // allowing only collision with CollisionNodes and special geometry under
    // higher levels of detail.
    int index = DCAST(LODNode, node)->get_lowest_switch();
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateDouble next_state(level_state, children.get_child(i));
      if (i != index) {
        next_state.set_include_mask(next_state.get_include_mask() &
          ~GeomNode::get_default_collide_mask());
      }
      r_traverse_double(next_state, pass);
    }

  } else {
    // Otherwise, visit all the children.
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateDouble next_state(level_state, children.get_child(i));
      r_traverse_double(next_state, pass);
    }
  }
}

/**
 * Fills up the set of LevelStates corresponding to the active colliders in
 * use.
 *
 * This flavor uses a CollisionLevelStateQuad, which is limited to a certain
 * number of colliders per pass (typically 32).
 */
void CollisionTraverser::
prepare_colliders_quad(CollisionTraverser::LevelStatesQuad &level_states,
                         const NodePath &root) {
  int num_colliders = _colliders.size();
  int max_colliders = CollisionLevelStateQuad::get_max_colliders();

  CollisionLevelStateQuad level_state(root);
  // This reserve() call is only correct if there is exactly one solid per
  // collider added to the traverser, which is the normal case.  If there is
  // more than one solid in any of the colliders, this reserve() call won't
  // reserve enough, but the code is otherwise correct.
  level_state.reserve(min(num_colliders, max_colliders));

  // Create an indirect index array to walk through the colliders in sorted
  // order, without affect the actual collider order.
  int *indirect = (int *)alloca(sizeof(int) * num_colliders);
  int i;
  for (i = 0; i < num_colliders; ++i) {
    indirect[i] = i;
  }
  std::sort(indirect, indirect + num_colliders, SortByColliderSort(*this));

  int num_remaining_colliders = num_colliders;
  for (i = 0; i < num_colliders; ++i) {
    OrderedColliderDef &ocd = _ordered_colliders[indirect[i]];
    NodePath cnode_path = ocd._node_path;

    if (!cnode_path.is_same_graph(root)) {
      if (ocd._in_graph) {
        // Only report this warning once.
        collide_cat.info()
          << "Collider " << cnode_path
          << " is not in scene graph.  Ignoring.\n";
        ocd._in_graph = false;
      }

    } else {
      ocd._in_graph = true;
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());

      CollisionLevelStateQuad::ColliderDef def;
      def._node = cnode;
      def._node_path = cnode_path;

      int num_solids = cnode->get_num_solids();
      for (int s = 0; s < num_solids; ++s) {
        CPT(CollisionSolid) collider = cnode->get_solid(s);
        def._collider = collider;
        level_state.prepare_collider(def, root);

        if (level_state.get_num_colliders() == max_colliders) {
          // That's the limit.  Save off this level state and make a new one.
          level_states.push_back(level_state);
          level_state.clear();
          level_state.reserve(min(num_remaining_colliders, max_colliders));
        }
      }
    }

    --num_remaining_colliders;
    nassertv(num_remaining_colliders >= 0);
  }

  if (level_state.get_num_colliders() != 0) {
    level_states.push_back(level_state);
  }
  nassertv(num_remaining_colliders == 0);
}

/**
 *
 */
void CollisionTraverser::
r_traverse_quad(CollisionLevelStateQuad &level_state, size_t pass) {
  if (!level_state.any_in_bounds()) {
    return;
  }
  if (!level_state.apply_transform()) {
    return;
  }

  PandaNode *node = level_state.node();
  if (node->is_collision_node()) {
    CollisionNode *cnode;
    DCAST_INTO_V(cnode, node);
    CPT(BoundingVolume) node_bv = cnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = cnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             cnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }

  } else if (node->is_geom_node()) {
    #ifndef NDEBUG
    if (collide_cat.is_spam()) {
      collide_cat.spam()
        << "Reached " << *node << "\n";
    }
    #endif

    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);
    CPT(BoundingVolume) node_bv = gnode->get_bounds();
    const GeometricBoundingVolume *node_gbv = nullptr;
    if (node_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, node_bv);
    }

    CollisionEntry entry;
    entry._into_node = gnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; ++c) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             gnode->get_into_collide_mask()) != 0) {
          #ifdef DO_PSTATS
          // PStatTimer collide_timer(_solid_collide_collectors[pass]);
          #endif
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_geom_node(
              entry,
              level_state.get_parent_bound(c),
              level_state.get_local_bound(c),
              node_gbv);
        }
      }
    }
  }

  if (node->has_single_child_visibility()) {
    // If it's a switch node or sequence node, visit just the one visible
    // child.
    int index = node->get_visible_child();
    if (index >= 0 && index < node->get_num_children()) {
      CollisionLevelStateQuad next_state(level_state, node->get_child(index));
      r_traverse_quad(next_state, pass);
    }

  } else if (node->is_lod_node()) {
    // If it's an LODNode, visit the lowest level of detail with all bits,
    // allowing collision with geometry under the lowest level of default; and
    // visit all other levels without GeomNode::get_default_collide_mask(),
    // allowing only collision with CollisionNodes and special geometry under
    // higher levels of detail.
    int index = DCAST(LODNode, node)->get_lowest_switch();
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateQuad next_state(level_state, children.get_child(i));
      if (i != index) {
        next_state.set_include_mask(next_state.get_include_mask() &
          ~GeomNode::get_default_collide_mask());
      }
      r_traverse_quad(next_state, pass);
    }

  } else {
    // Otherwise, visit all the children.
    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      CollisionLevelStateQuad next_state(level_state, children.get_child(i));
      r_traverse_quad(next_state, pass);
    }
  }
}

/**
 *
 */
void CollisionTraverser::
compare_collider_to_node(CollisionEntry &entry,
                         const GeometricBoundingVolume *from_parent_gbv,
                         const GeometricBoundingVolume *from_node_gbv,
                         const GeometricBoundingVolume *into_node_gbv) {
  bool within_node_bounds = true;
  if (from_parent_gbv != nullptr &&
      into_node_gbv != nullptr) {
    within_node_bounds = (into_node_gbv->contains(from_parent_gbv) != 0);
    _cnode_volume_pcollector.add_level(1);
  }

  if (within_node_bounds) {
    Thread *current_thread = Thread::get_current_thread();

    CollisionNode *cnode;
    DCAST_INTO_V(cnode, entry._into_node);

    int num_solids = cnode->get_num_solids();
    if (collide_cat.is_spam()) {
      collide_cat.spam()
        << "Colliding against CollisionNode " << entry._into_node
        << " which has " << num_solids << " collision solids.\n";
    }

    // Only bother to test against each solid's bounding volume if we have
    // more than one solid in the node, as a slight optimization.  (If the
    // node contains just one solid, then the node's bounding volume, which
    // we just tested, is the same as the solid's bounding volume.)
    if (num_solids == 1) {
      entry._into = cnode->_solids[0].get_read_pointer(current_thread);
      Colliders::const_iterator ci;
      ci = _colliders.find(entry.get_from_node_path());
      nassertv(ci != _colliders.end());
      entry.test_intersection((*ci).second, this);
    } else {
      CollisionNode::Solids::const_iterator si;
      for (si = cnode->_solids.begin(); si != cnode->_solids.end(); ++si) {
        entry._into = (*si).get_read_pointer(current_thread);

        // We should allow a collision test for solid into itself, because the
        // solid might be simply instanced into multiple different
        // CollisionNodes.  We are already filtering out tests for a
        // CollisionNode into itself.
        CPT(BoundingVolume) solid_bv = entry._into->get_bounds();
        const GeometricBoundingVolume *solid_gbv = nullptr;
        if (solid_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
          solid_gbv = (const GeometricBoundingVolume *)solid_bv.p();
        }

        compare_collider_to_solid(entry, from_node_gbv, solid_gbv);
      }
    }
  }
}

/**
 *
 */
void CollisionTraverser::
compare_collider_to_geom_node(CollisionEntry &entry,
                              const GeometricBoundingVolume *from_parent_gbv,
                              const GeometricBoundingVolume *from_node_gbv,
                              const GeometricBoundingVolume *into_node_gbv) {
  bool within_node_bounds = true;
  if (from_parent_gbv != nullptr &&
      into_node_gbv != nullptr) {
    within_node_bounds = (into_node_gbv->contains(from_parent_gbv) != 0);
    _gnode_volume_pcollector.add_level(1);
  }

  if (within_node_bounds) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, entry._into_node);
    int num_geoms = gnode->get_num_geoms();
    for (int s = 0; s < num_geoms; ++s) {
      entry._into = nullptr;
      const Geom *geom = DCAST(Geom, gnode->get_geom(s));
      if (geom != nullptr) {
        CPT(BoundingVolume) geom_bv = geom->get_bounds();
        const GeometricBoundingVolume *geom_gbv = nullptr;
        if (num_geoms > 1 &&
            geom_bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
          // Only bother to test against each geom's bounding volume if we
          // have more than one geom in the node, as a slight optimization.
          // (If the node contains just one geom, then the node's bounding
          // volume, which we just tested, is the same as the geom's bounding
          // volume.)
          DCAST_INTO_V(geom_gbv, geom_bv);
        }

        compare_collider_to_geom(entry, geom, from_node_gbv, geom_gbv);
      }
    }
  }
}

/**
 *
 */
void CollisionTraverser::
compare_collider_to_solid(CollisionEntry &entry,
                          const GeometricBoundingVolume *from_node_gbv,
                          const GeometricBoundingVolume *solid_gbv) {
  bool within_solid_bounds = true;
  if (from_node_gbv != nullptr &&
      solid_gbv != nullptr) {
    within_solid_bounds = (solid_gbv->contains(from_node_gbv) != 0);
    #ifdef DO_PSTATS
    ((CollisionSolid *)entry.get_into())->get_volume_pcollector().add_level(1);
    #endif  // DO_PSTATS
#ifndef NDEBUG
    if (collide_cat.is_spam()) {
      collide_cat.spam(false)
        << "Comparing to solid: " << *from_node_gbv
        << " to " << *solid_gbv << ", within_solid_bounds = "
        << within_solid_bounds << "\n";
    }
#endif  // NDEBUG
  }
  if (within_solid_bounds) {
    Colliders::const_iterator ci;
    ci = _colliders.find(entry.get_from_node_path());
    nassertv(ci != _colliders.end());
    entry.test_intersection((*ci).second, this);
  }
}

/**
 *
 */
void CollisionTraverser::
compare_collider_to_geom(CollisionEntry &entry, const Geom *geom,
                         const GeometricBoundingVolume *from_node_gbv,
                         const GeometricBoundingVolume *geom_gbv) {
  bool within_geom_bounds = true;
  if (from_node_gbv != nullptr &&
      geom_gbv != nullptr) {
    within_geom_bounds = (geom_gbv->contains(from_node_gbv) != 0);
    _geom_volume_pcollector.add_level(1);
  }
  if (within_geom_bounds) {
    Colliders::const_iterator ci;
    ci = _colliders.find(entry.get_from_node_path());
    nassertv(ci != _colliders.end());

    if (geom->get_primitive_type() == Geom::PT_polygons) {
      Thread *current_thread = Thread::get_current_thread();
      CPT(GeomVertexData) data = geom->get_animated_vertex_data(true, current_thread);
      GeomVertexReader vertex(data, InternalName::get_vertex());

      int num_primitives = geom->get_num_primitives();
      for (int i = 0; i < num_primitives; ++i) {
        const GeomPrimitive *primitive = geom->get_primitive(i);
        CPT(GeomPrimitive) tris = primitive->decompose();
        nassertv(tris->is_of_type(GeomTriangles::get_class_type()));

        if (tris->is_indexed()) {
          // Indexed case.
          GeomVertexReader index(tris->get_vertices(), 0);
          while (!index.is_at_end()) {
            LPoint3 v[3];

            vertex.set_row_unsafe(index.get_data1i());
            v[0] = vertex.get_data3();
            vertex.set_row_unsafe(index.get_data1i());
            v[1] = vertex.get_data3();
            vertex.set_row_unsafe(index.get_data1i());
            v[2] = vertex.get_data3();

            // Generate a temporary CollisionGeom on the fly for each triangle
            // in the Geom.
            if (CollisionPolygon::verify_points(v[0], v[1], v[2])) {
              bool within_solid_bounds = true;
              if (from_node_gbv != nullptr) {
                PT(BoundingSphere) sphere = new BoundingSphere;
                sphere->around(v, v + 3);
                within_solid_bounds = (sphere->contains(from_node_gbv) != 0);
#ifdef DO_PSTATS
                CollisionGeom::_volume_pcollector.add_level(1);
#endif  // DO_PSTATS
              }
              if (within_solid_bounds) {
                PT(CollisionGeom) cgeom = new CollisionGeom(LVecBase3(v[0]), LVecBase3(v[1]), LVecBase3(v[2]));
                entry._into = cgeom;
                entry.test_intersection((*ci).second, this);
              }
            }
          }
        } else {
          // Non-indexed case.
          vertex.set_row_unsafe(primitive->get_first_vertex());
          int num_vertices = primitive->get_num_vertices();
          for (int i = 0; i < num_vertices; i += 3) {
            LPoint3 v[3];

            v[0] = vertex.get_data3();
            v[1] = vertex.get_data3();
            v[2] = vertex.get_data3();

            // Generate a temporary CollisionGeom on the fly for each triangle
            // in the Geom.
            if (CollisionPolygon::verify_points(v[0], v[1], v[2])) {
              bool within_solid_bounds = true;
              if (from_node_gbv != nullptr) {
                PT(BoundingSphere) sphere = new BoundingSphere;
                sphere->around(v, v + 3);
                within_solid_bounds = (sphere->contains(from_node_gbv) != 0);
#ifdef DO_PSTATS
                CollisionGeom::_volume_pcollector.add_level(1);
#endif  // DO_PSTATS
              }
              if (within_solid_bounds) {
                PT(CollisionGeom) cgeom = new CollisionGeom(LVecBase3(v[0]), LVecBase3(v[1]), LVecBase3(v[2]));
                entry._into = cgeom;
                entry.test_intersection((*ci).second, this);
              }
            }
          }
        }
      }
    }
  }
}

/**
 * Removes the indicated CollisionHandler from the list of handlers to be
 * processed, and returns the iterator to the next handler in the list.  This
 * is designed to be called safely from within a traversal of the handler
 * list.
 *
 * This also removes any colliders that depend on this handler, to keep
 * internal structures intact.
 */
CollisionTraverser::Handlers::iterator CollisionTraverser::
remove_handler(CollisionTraverser::Handlers::iterator hi) {
  nassertr(hi != _handlers.end(), hi);

  CollisionHandler *handler = (*hi).first;
  Handlers::iterator hnext = hi;
  ++hnext;
  _handlers.erase(hi);
  hi = hnext;

  // Now scan for colliders that reference this handler.
  Colliders::iterator ci;
  ci = _colliders.begin();
  while (ci != _colliders.end()) {
    if ((*ci).second == handler) {
      // This collider references this handler; remove it.
      NodePath node_path = (*ci).first;

      Colliders::iterator cnext = ci;
      ++cnext;
      _colliders.erase(ci);
      ci = cnext;

      // Also remove it from the ordered list.
      OrderedColliders::iterator oci;
      oci = _ordered_colliders.begin();
      while (oci != _ordered_colliders.end() &&
             (*oci)._node_path != node_path) {
        ++oci;
      }
      nassertr(oci != _ordered_colliders.end(), hi);
      _ordered_colliders.erase(oci);

      nassertr(_ordered_colliders.size() == _colliders.size(), hi);

    } else {
      // This collider references some other handler; keep it.
      ++ci;
    }
  }

  return hi;
}

/**
 * Returns the PStatCollector suitable for timing the nth pass.
 */
PStatCollector &CollisionTraverser::
get_pass_collector(int pass) {
  nassertr(pass >= 0, _this_pcollector);
  while ((int)_pass_collectors.size() <= pass) {
    std::ostringstream name;
    name << "pass" << (_pass_collectors.size() + 1);
    PStatCollector col(_this_pcollector, name.str());
    _pass_collectors.push_back(col);
    PStatCollector sc_col(col, "solid_collide");
    _solid_collide_collectors.push_back(sc_col);
  }

  return _pass_collectors[pass];
}
