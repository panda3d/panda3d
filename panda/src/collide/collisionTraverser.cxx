// Filename: collisionTraverser.cxx
// Created by:  drose (16Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "collisionTraverser.h"
#include "collisionNode.h"
#include "collisionEntry.h"
#include "collisionPolygon.h"
#include "collisionRecorder.h"
#include "collisionVisualizer.h"
#include "config_collide.h"

#include "transformState.h"
#include "geomNode.h"
#include "geom.h"
#include "lodNode.h"
#include "nodePath.h"
#include "pStatTimer.h"
#include "indent.h"

PStatCollector CollisionTraverser::_collisions_pcollector("App:Collisions");
PStatCollector CollisionTraverser::_reset_prev_pcollector("App:Collisions:Reset");

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CollisionTraverser::
CollisionTraverser(const string &name) : 
  Namable(name),
  _this_pcollector(_collisions_pcollector, name)
{
  _respect_prev_transform = respect_prev_transform;
#ifdef DO_COLLISION_RECORDING
  _recorder = (CollisionRecorder *)NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
CollisionTraverser::
~CollisionTraverser() {
#ifdef DO_COLLISION_RECORDING
  clear_recorder();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::add_collider
//       Access: Published
//  Description: Adds a new CollisionNode, representing an object that
//               will be tested for collisions into other objects,
//               along with the handler that will serve each detected
//               collision.  Each CollisionNode may be served by only
//               one handler at a time, but a given handler may serve
//               many CollisionNodes.
//
//               The handler that serves a particular node may be
//               changed from time to time by calling add_collider()
//               again on the same node.
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
add_collider(const NodePath &collider, CollisionHandler *handler) {
  nassertv(_ordered_colliders.size() == _colliders.size());
  nassertv(!collider.is_empty() && collider.node()->is_of_type(CollisionNode::get_class_type()));
  nassertv(handler != (CollisionHandler *)NULL);

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
        (*hi).second++;
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
      (*hi).second++;
    }
  }

  nassertv(_ordered_colliders.size() == _colliders.size());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::remove_collider
//       Access: Published
//  Description: Removes the collider (and its associated handler)
//               from the set of CollisionNodes that will be tested
//               each frame for collisions into other objects.
//               Returns true if the definition was found and removed,
//               false if it wasn't present to begin with.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::has_collider
//       Access: Published
//  Description: Returns true if the indicated node is current in the
//               set of nodes that will be tested each frame for
//               collisions into other objects.
////////////////////////////////////////////////////////////////////
bool CollisionTraverser::
has_collider(const NodePath &collider) const {
  Colliders::const_iterator ci = _colliders.find(collider);
  return (ci != _colliders.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::get_num_colliders
//       Access: Published
//  Description: Returns the number of CollisionNodes that have been
//               added to the traverser via add_collider().
////////////////////////////////////////////////////////////////////
int CollisionTraverser::
get_num_colliders() const {
  nassertr(_ordered_colliders.size() == _colliders.size(), 0);
  return _ordered_colliders.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::get_collider
//       Access: Published
//  Description: Returns the nth CollisionNode that has been
//               added to the traverser via add_collider().
////////////////////////////////////////////////////////////////////
NodePath CollisionTraverser::
get_collider(int n) const {
  nassertr(_ordered_colliders.size() == _colliders.size(), NULL);
  nassertr(n >= 0 && n < (int)_ordered_colliders.size(), NULL);
  return _ordered_colliders[n]._node_path;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::get_handler
//       Access: Published
//  Description: Returns the handler that is currently assigned to
//               serve the indicated collision node, or NULL if the
//               node is not on the traverser's set of active nodes.
////////////////////////////////////////////////////////////////////
CollisionHandler *CollisionTraverser::
get_handler(const NodePath &collider) const {
  Colliders::const_iterator ci = _colliders.find(collider);
  if (ci != _colliders.end()) {
    return (*ci).second;
  };
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::clear_colliders
//       Access: Published
//  Description: Completely empties the set of collision nodes and
//               their associated handlers.
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
clear_colliders() {
  _colliders.clear();
  _ordered_colliders.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::add_collider
//       Access: Published
//  Description: This method is deprecated and will shortly be removed
//               in favor of the newer NodePath-based method, above.
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
add_collider(CollisionNode *node, CollisionHandler *handler) {
  add_collider(NodePath(node), handler);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::remove_collider
//       Access: Published
//  Description: This method is deprecated and will shortly be removed
//               in favor of the newer NodePath-based method, above.
////////////////////////////////////////////////////////////////////
bool CollisionTraverser::
remove_collider(CollisionNode *node) {
  return remove_collider(NodePath(node));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::traverse
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
traverse(const NodePath &root) {
  PStatTimer timer(_this_pcollector);

#ifdef DO_COLLISION_RECORDING
  if (has_recorder()) {
    get_recorder()->begin_traversal();
  }
#endif  // DO_COLLISION_RECORDING

  CollisionLevelState level_state(root);
  prepare_colliders(level_state, root);

  Handlers::iterator hi;
  for (hi = _handlers.begin(); hi != _handlers.end(); ++hi) {
    (*hi).first->begin_group();
  }

  r_traverse(level_state);

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
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::reset_prev_transform
//       Access: Published
//  Description: Once the collision traversal has finished, resets all
//               of the velocity deltas in the scene graph by setting
//               the "previous" transform to the current transform.
//               This must be called at least once per frame for
//               collisions to respect this velocity setting properly.
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
reset_prev_transform(const NodePath &root) {
  PStatTimer timer(_reset_prev_pcollector);
  r_reset_prev_transform(root.node());
}

#ifdef DO_COLLISION_RECORDING
////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::set_recorder
//       Access: Published
//  Description: Uses the indicated CollisionRecorder object to start
//               recording the intersection tests made by each
//               subsequent call to traverse() on this object.  A
//               particular CollisionRecorder object can only record
//               one traverser at a time; if this object has already
//               been assigned to another traverser, that assignment
//               is broken.
//
//               This is intended to be used in a debugging mode to
//               try to determine what work is being performed by the
//               collision traversal.  Usually, attaching a recorder
//               will impose significant runtime overhead.
//
//               This does not transfer ownership of the
//               CollisionRecorder pointer; maintenance of that
//               remains the caller's responsibility.  If the
//               CollisionRecorder is destructed, it will cleanly
//               remove itself from the traverser.
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
set_recorder(CollisionRecorder *recorder) {
  if (recorder != _recorder) {
    // Remove the old recorder, if any.
    if (_recorder != (CollisionRecorder *)NULL) {
      nassertv(_recorder->_trav == this);
      _recorder->_trav = (CollisionTraverser *)NULL;
    }
    
    _recorder = recorder;
    
    // Tell the new recorder about his new owner.
    if (_recorder != (CollisionRecorder *)NULL) {
      nassertv(_recorder->_trav != this);
      if (_recorder->_trav != (CollisionTraverser *)NULL) {
        _recorder->_trav->clear_recorder();
      }
      nassertv(_recorder->_trav == (CollisionTraverser *)NULL);
      _recorder->_trav = this;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::show_collisions
//       Access: Published
//  Description: This is a high-level function to create a
//               CollisionVisualizer object to render the collision
//               tests performed by this traverser.  The supplied root
//               should be any node in the scene graph; typically, the
//               top node (e.g. render).  The CollisionVisualizer will
//               be attached to this node.
////////////////////////////////////////////////////////////////////
CollisionVisualizer *CollisionTraverser::
show_collisions(const NodePath &root) {
  hide_collisions();
  CollisionVisualizer *viz = new CollisionVisualizer("show_collisions");
  _collision_visualizer_np = root.attach_new_node(viz);
  set_recorder(viz);
  return viz;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::hide_collisions
//       Access: Published
//  Description: Undoes the effect of a previous call to
//               show_collisions().
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
hide_collisions() {
  if (!_collision_visualizer_np.is_empty()) {
    _collision_visualizer_np.remove_node();
  }
  clear_recorder();
}

#endif  // DO_COLLISION_RECORDING

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
output(ostream &out) const {
  out << "CollisionTraverser, " << _colliders.size()
      << " colliders and " << _handlers.size() << " handlers.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
write(ostream &out, int indent_level) const {
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
    nassertv(handler != (CollisionHandler *)NULL);
    
    indent(out, indent_level + 2)
      << cnode_path;
    if (in_graph) {
      out << " handled by " << handler->get_type() << "\n";
    } else {
      out << " ignored\n";
    }

    if (!cnode_path.is_empty() && cnode_path.node()->is_of_type(CollisionNode::get_class_type())) {
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());
      
      int num_solids = cnode->get_num_solids();
      for (int i = 0; i < num_solids; i++) {
        cnode->get_solid(i)->write(out, indent_level + 4);
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::prepare_colliders
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
prepare_colliders(CollisionLevelState &level_state, const NodePath &root) {
  level_state.clear();
  level_state.reserve(_colliders.size());

  OrderedColliders::iterator oci;
  for (oci = _ordered_colliders.begin(); 
       oci != _ordered_colliders.end(); 
       ++oci) {
    NodePath cnode_path = (*oci)._node_path;

    if (!cnode_path.is_same_graph(root)) {
      if ((*oci)._in_graph) {
        // Only report this warning once.
        collide_cat.info()
          << "Collider " << cnode_path
          << " is not in scene graph.  Ignoring.\n";
        (*oci)._in_graph = false;
      }

    } else {
      (*oci)._in_graph = true;
      CollisionNode *cnode = DCAST(CollisionNode, cnode_path.node());
      
      CollisionLevelState::ColliderDef def;
      def._node = cnode;
      def._node_path = cnode_path;
      
      int num_solids = cnode->get_num_solids();
      for (int s = 0; s < num_solids; s++) {
        CollisionSolid *collider = cnode->get_solid(s);
        def._collider = collider;
        level_state.prepare_collider(def);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::r_traverse
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
r_traverse(CollisionLevelState &level_state) {
  if (!level_state.any_in_bounds()) {
    return;
  }
  level_state.apply_transform();

  PandaNode *node = level_state.node();
  if (node->is_exact_type(CollisionNode::get_class_type())) {
    CollisionNode *cnode;
    DCAST_INTO_V(cnode, node);
    const BoundingVolume &node_bv = cnode->get_bound();
    const GeometricBoundingVolume *node_gbv = NULL;
    if (node_bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, &node_bv);
    }

    CollisionEntry entry;
    entry._into_node = cnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             cnode->get_into_collide_mask()) != 0) {
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_node(entry, 
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
    const BoundingVolume &node_bv = gnode->get_bound();
    const GeometricBoundingVolume *node_gbv = NULL;
    if (node_bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
      DCAST_INTO_V(node_gbv, &node_bv);
    }

    CollisionEntry entry;
    entry._into_node = gnode;
    entry._into_node_path = level_state.get_node_path();
    if (_respect_prev_transform) {
      entry._flags |= CollisionEntry::F_respect_prev_transform;
    }

    int num_colliders = level_state.get_num_colliders();
    for (int c = 0; c < num_colliders; c++) {
      if (level_state.has_collider(c)) {
        entry._from_node = level_state.get_collider_node(c);

        if ((entry._from_node->get_from_collide_mask() &
             gnode->get_into_collide_mask()) != 0) {
          entry._from_node_path = level_state.get_collider_node_path(c);
          entry._from = level_state.get_collider(c);

          compare_collider_to_geom_node(entry, 
                                        level_state.get_parent_bound(c),
                                        level_state.get_local_bound(c),
                                        node_gbv);
        }
      }
    }
  }

  if (node->has_single_child_visibility()) {
    // If it's a switch node or sequence node, visit just the one
    // visible child.
    int index = node->get_visible_child();
    if (index >= 0 && index < node->get_num_children()) {
      CollisionLevelState next_state(level_state, node->get_child(index));
      r_traverse(next_state);
    }

  } else if (node->is_lod_node()) {
    // If it's an LODNode, visit the lowest level of detail.
    int index = DCAST(LODNode, node)->get_lowest_switch();
    if (index >= 0 && index < node->get_num_children()) {
      CollisionLevelState next_state(level_state, node->get_child(index));
      r_traverse(next_state);
    }

  } else {
    // Otherwise, visit all the children.
    int num_children = node->get_num_children();
    for (int i = 0; i < num_children; i++) {
      CollisionLevelState next_state(level_state, node->get_child(i));
      r_traverse(next_state);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::compare_collider_to_node
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
compare_collider_to_node(CollisionEntry &entry,
                         const GeometricBoundingVolume *from_parent_gbv,
                         const GeometricBoundingVolume *from_node_gbv,
                         const GeometricBoundingVolume *into_node_gbv) {
  bool within_node_bounds = true;
  if (from_parent_gbv != (GeometricBoundingVolume *)NULL &&
      into_node_gbv != (GeometricBoundingVolume *)NULL) {
    within_node_bounds = (into_node_gbv->contains(from_parent_gbv) != 0);
  }

  if (within_node_bounds) {
    CollisionNode *cnode;
    DCAST_INTO_V(cnode, entry._into_node);
    int num_solids = cnode->get_num_solids();
    for (int s = 0; s < num_solids; s++) {
      entry._into = cnode->get_solid(s);
      if (entry._from != entry._into) {
        const BoundingVolume &solid_bv = entry._into->get_bound();
        const GeometricBoundingVolume *solid_gbv = NULL;
        if (num_solids > 1 &&
            solid_bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
          // Only bother to test against each solid's bounding
          // volume if we have more than one solid in the node, as a
          // slight optimization.  (If the node contains just one
          // solid, then the node's bounding volume, which we just
          // tested, is the same as the solid's bounding volume.)
          DCAST_INTO_V(solid_gbv, &solid_bv);
        }

        compare_collider_to_solid(entry, from_node_gbv, solid_gbv);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::compare_collider_to_geom_node
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
compare_collider_to_geom_node(CollisionEntry &entry,
                              const GeometricBoundingVolume *from_parent_gbv,
                              const GeometricBoundingVolume *from_node_gbv,
                              const GeometricBoundingVolume *into_node_gbv) {
  bool within_node_bounds = true;
  if (from_parent_gbv != (GeometricBoundingVolume *)NULL &&
      into_node_gbv != (GeometricBoundingVolume *)NULL) {
    within_node_bounds = (into_node_gbv->contains(from_parent_gbv) != 0);
  }

  if (within_node_bounds) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, entry._into_node);
    int num_geoms = gnode->get_num_geoms();
    for (int s = 0; s < num_geoms; s++) {
      entry._into = (CollisionSolid *)NULL;
      const Geom *geom = DCAST(Geom, gnode->get_geom(s));
      if (geom != (Geom *)NULL) {
        const BoundingVolume &geom_bv = geom->get_bound();
        const GeometricBoundingVolume *geom_gbv = NULL;
        if (num_geoms > 1 &&
            geom_bv.is_of_type(GeometricBoundingVolume::get_class_type())) {
          // Only bother to test against each geom's bounding
          // volume if we have more than one geom in the node, as a
          // slight optimization.  (If the node contains just one
          // geom, then the node's bounding volume, which we just
          // tested, is the same as the geom's bounding volume.)
          DCAST_INTO_V(geom_gbv, &geom_bv);
        }

        compare_collider_to_geom(entry, geom, from_node_gbv, geom_gbv);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::compare_collider_to_solid
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
compare_collider_to_solid(CollisionEntry &entry,
                          const GeometricBoundingVolume *from_node_gbv,
                          const GeometricBoundingVolume *solid_gbv) {
  bool within_solid_bounds = true;
  if (from_node_gbv != (GeometricBoundingVolume *)NULL &&
      solid_gbv != (GeometricBoundingVolume *)NULL) {
    within_solid_bounds = (solid_gbv->contains(from_node_gbv) != 0);
  }
  if (within_solid_bounds) {
    Colliders::const_iterator ci;
    ci = _colliders.find(entry.get_from_node_path());
    nassertv(ci != _colliders.end());
    entry.test_intersection((*ci).second, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::compare_collider_to_geom
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
compare_collider_to_geom(CollisionEntry &entry, const Geom *geom,
                         const GeometricBoundingVolume *from_node_gbv,
                         const GeometricBoundingVolume *geom_gbv) {
  bool within_geom_bounds = true;
  if (from_node_gbv != (GeometricBoundingVolume *)NULL &&
      geom_gbv != (GeometricBoundingVolume *)NULL) {
    within_geom_bounds = (geom_gbv->contains(from_node_gbv) != 0);
  }
  if (within_geom_bounds) {
    Colliders::const_iterator ci;
    ci = _colliders.find(entry.get_from_node_path());
    nassertv(ci != _colliders.end());

    PTA_Vertexf coords;
    PTA_ushort vindex;
    geom->get_coords(coords, vindex);
    PTA_ushort tris = geom->get_tris();

    for (int i = 0; i < (int)tris.size(); i += 3) {
      if (CollisionPolygon::verify_points(coords[tris[i]],
                                          coords[tris[i + 1]],
                                          coords[tris[i + 2]])) {
        // Generate a temporary CollisionPolygon on the fly for each
        // triangle in the Geom.
        entry._into =
          new CollisionPolygon(coords[tris[i]], coords[tris[i + 1]],
                               coords[tris[i + 2]]);
        entry.test_intersection((*ci).second, this);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::remove_handler
//       Access: Private
//  Description: Removes the indicated CollisionHandler from the list
//               of handlers to be processed, and returns the iterator
//               to the next handler in the list.  This is designed to
//               be called safely from within a traversal of the handler
//               list.
//
//               This also removes any colliders that depend on this
//               handler, to keep internal structures intact.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionTraverser::r_reset_prev_transform
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void CollisionTraverser::
r_reset_prev_transform(PandaNode *node) {
  node->reset_prev_transform();

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; i++) {
    r_reset_prev_transform(children.get_child(i));
  }
}
