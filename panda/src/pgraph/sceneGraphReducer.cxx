/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphReducer.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "sceneGraphReducer.h"
#include "config_pgraph.h"
#include "accumulatedAttribs.h"
#include "boundingSphere.h"
#include "modelNode.h"
#include "pointerTo.h"
#include "plist.h"
#include "pmap.h"
#include "geomNode.h"
#include "config_gobj.h"
#include "thread.h"

PStatCollector SceneGraphReducer::_flatten_collector("*:Flatten:flatten");
PStatCollector SceneGraphReducer::_apply_collector("*:Flatten:apply");
PStatCollector SceneGraphReducer::_remove_column_collector("*:Flatten:remove column");
PStatCollector SceneGraphReducer::_compatible_state_collector("*:Flatten:compatible colors");
PStatCollector SceneGraphReducer::_collect_collector("*:Flatten:collect");
PStatCollector SceneGraphReducer::_make_nonindexed_collector("*:Flatten:make nonindexed");
PStatCollector SceneGraphReducer::_unify_collector("*:Flatten:unify");
PStatCollector SceneGraphReducer::_remove_unused_collector("*:Flatten:remove unused vertices");
PStatCollector SceneGraphReducer::_premunge_collector("*:Premunge");

/**
 * Specifies the particular GraphicsStateGuardian that this object will
 * attempt to optimize to.  The GSG may specify parameters such as maximum
 * number of vertices per vertex data, max number of vertices per primitive,
 * and whether triangle strips are preferred.  It also affects the types of
 * vertex column data that is created by premunge().
 */
void SceneGraphReducer::
set_gsg(GraphicsStateGuardianBase *gsg) {
  if (gsg != nullptr) {
    _gsg = gsg;
  } else {
    _gsg = GraphicsStateGuardianBase::get_default_gsg();
  }

  int max_vertices = max_collect_vertices;

  if (_gsg != nullptr) {
    max_vertices = std::min(max_vertices, _gsg->get_max_vertices_per_array());
  }

  _transformer.set_max_collect_vertices(max_vertices);
}

/**
 * Specifies that no particular GraphicsStateGuardian will be used to guide
 * the optimization.  The SceneGraphReducer will instead use config variables
 * such as max-collect-vertices and max-collect-indices.
 */
void SceneGraphReducer::
clear_gsg() {
  _gsg = nullptr;
  _transformer.set_max_collect_vertices(max_collect_vertices);
}

/**
 * Simplifies the graph by removing unnecessary nodes and nodes.
 *
 * In general, a node (and its parent node) is a candidate for removal if the
 * node has no siblings and the node has no special properties.
 *
 * If combine_siblings_bits is nonzero, some sibling nodes (according to the
 * bits set in combine_siblings_bits) may also be collapsed into a single
 * node.  This will further reduce scene graph complexity, sometimes
 * substantially, at the cost of reduced spatial separation.
 *
 * Returns the number of nodes removed from the graph.
 */
int SceneGraphReducer::
flatten(PandaNode *root, int combine_siblings_bits) {
  nassertr(check_live_flatten(root), 0);

  PStatTimer timer(_flatten_collector);
  int num_total_nodes = 0;
  int num_pass_nodes;

  do {
    num_pass_nodes = 0;

    // Get a copy of the children list, so we don't have to worry about self-
    // modifications.
    PandaNode::Children cr = root->get_children();

    // Now visit each of the children in turn.
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PT(PandaNode) child_node = cr.get_child(i);
      num_pass_nodes += r_flatten(root, child_node, combine_siblings_bits);
    }

    if (combine_siblings_bits != 0 &&
        root->get_num_children() >= 2 &&
        root->safe_to_combine_children()) {
      num_pass_nodes += flatten_siblings(root, combine_siblings_bits);
    }

    num_total_nodes += num_pass_nodes;

    // If combine_siblings_bits has CS_recurse set, we should repeat the above
    // until we don't get any more benefit from flattening, because each pass
    // could convert cousins into siblings, which may get flattened next pass.
  } while ((combine_siblings_bits & CS_recurse) != 0 && num_pass_nodes != 0);

  return num_total_nodes;
}

/**
 * Removes the indicated data column from any GeomVertexDatas found at the
 * indicated root and below.  Returns the number of GeomNodes modified.
 */
int SceneGraphReducer::
remove_column(PandaNode *root, const InternalName *column) {
  nassertr(check_live_flatten(root), 0);

  PStatTimer timer(_remove_column_collector);
  int count = r_remove_column(root, column, _transformer);
  _transformer.finish_apply();
  return count;
}

/**
 * Searches for GeomNodes that contain multiple Geoms that differ only in
 * their ColorAttribs.  If such a GeomNode is found, then all the colors are
 * pushed down into the vertices.  This makes it feasible for the geoms to be
 * unified later.
 */
int SceneGraphReducer::
make_compatible_state(PandaNode *root) {
  nassertr(check_live_flatten(root), 0);

  PStatTimer timer(_compatible_state_collector);
  int count = r_make_compatible_state(root, _transformer);
  _transformer.finish_apply();
  return count;
}

/**
 * Calls decompose() on every GeomNode at this level and below.
 *
 * There is usually no reason to call this explicitly, since unify() will do
 * this anyway if it needs to be done.  However, calling it ahead of time can
 * make that future call to unify() run a little bit faster.
 *
 * This operation has no effect if the config variable preserve-triangle-
 * strips has been set true.
 */
void SceneGraphReducer::
decompose(PandaNode *root) {
  nassertv(check_live_flatten(root));

  if (!preserve_triangle_strips) {
    PStatTimer timer(_unify_collector);
    r_decompose(root);
  }
}

/**
 * Calls unify() on every GeomNode at this level and below.  This attempts to
 * reduce the total number of individual Geoms and GeomPrimitives by combining
 * these objects wherever possible.  See GeomNode::unify().
 */
void SceneGraphReducer::
unify(PandaNode *root, bool preserve_order) {
  nassertv(check_live_flatten(root));
  PStatTimer timer(_unify_collector);

  int max_indices = max_collect_indices;
  if (_gsg != nullptr) {
    max_indices = std::min(max_indices, _gsg->get_max_vertices_per_primitive());
  }
  r_unify(root, max_indices, preserve_order);
}

/**
 * Removes any vertices in GeomVertexDatas that are no longer used at this
 * level and below.  This requires remapping vertex indices in all of the
 * GeomPrimitives, to remove holes in the GeomVertexDatas.  It is normally not
 * necessary to call this explicitly.
 */
void SceneGraphReducer::
remove_unused_vertices(PandaNode *root) {
  nassertv(check_live_flatten(root));
  PStatTimer timer(_remove_unused_collector);

  r_register_vertices(root, _transformer);
  _transformer.finish_apply();
  Thread::consider_yield();
}

/**
 * In a non-release build, returns false if the node is correctly not in a
 * live scene graph.  (Calling flatten on a node that is part of a live scene
 * graph, for instance, a node somewhere under render, can cause problems in a
 * multithreaded environment.)
 *
 * If allow_live_flatten is true, or in a release build, this always returns
 * true.
 */
bool SceneGraphReducer::
check_live_flatten(PandaNode *node) {
#ifndef NDEBUG
  if (allow_live_flatten) {
    return true;
  }

  if (node->is_under_scene_root()) {
    return false;
  }

#endif  // NDEBUG
  return true;
}

/**
 * The recursive implementation of apply_attribs().
 */
void SceneGraphReducer::
r_apply_attribs(PandaNode *node, const AccumulatedAttribs &attribs,
                int attrib_types, GeomTransformer &transformer) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "r_apply_attribs(" << *node << "), node's attribs are:\n";
    node->get_transform()->write(pgraph_cat.spam(false), 2);
    node->get_state()->write(pgraph_cat.spam(false), 2);
    node->get_effects()->write(pgraph_cat.spam(false), 2);
  }

  AccumulatedAttribs next_attribs(attribs);
  next_attribs.collect(node, attrib_types);

  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Got attribs from " << *node << "\n"
      << "Accumulated attribs are:\n";
    next_attribs.write(pgraph_cat.spam(false), attrib_types, 2);
  }

  // Check to see if we can't propagate any of these attribs past this node
  // for some reason.
  if (!node->safe_to_flatten_below()) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Not applying further; " << *node
        << " doesn't allow flattening below itself.\n";
    }
    next_attribs.apply_to_node(node, attrib_types);
    return;
  }

  int apply_types = 0;

  const RenderEffects *effects = node->get_effects();
  if (!effects->safe_to_transform()) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Node " << *node
        << " contains a non-transformable effect; leaving transform here.\n";
    }
    next_attribs._transform = effects->prepare_flatten_transform(next_attribs._transform);
    apply_types |= TT_transform;
  }
  if (!node->safe_to_transform()) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Cannot safely transform nodes of type " << node->get_type()
        << "; leaving a transform here but carrying on otherwise.\n";
    }
    apply_types |= TT_transform;
  }
  apply_types |= node->get_unsafe_to_apply_attribs();

  // Also, check the children of this node.  If any of them indicates it is
  // not safe to modify its transform, we must drop our transform here.
  int num_children = node->get_num_children();
  int i;
  if ((apply_types & TT_transform) == 0) {
    bool children_transform_friendly = true;
    for (i = 0; i < num_children && children_transform_friendly; i++) {
      PandaNode *child_node = node->get_child(i);
      children_transform_friendly = child_node->safe_to_modify_transform();
    }

    if (!children_transform_friendly) {
      if (pgraph_cat.is_spam()) {
        pgraph_cat.spam()
          << "Node " << *node
          << " has a child that cannot modify its transform; leaving transform here.\n";
      }
      apply_types |= TT_transform;
    }
  }

  // Directly store whatever attributes we must,
  next_attribs.apply_to_node(node, attrib_types & apply_types);

  // And apply the rest to the vertices.
  node->apply_attribs_to_vertices(next_attribs, attrib_types, transformer);

  // Do we need to copy any children to flatten instances?
  bool resist_copy = false;
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = node->get_child(i);

    if (child_node->get_num_parents() > 1) {
      if (!child_node->safe_to_flatten()) {
        if (pgraph_cat.is_spam()) {
          pgraph_cat.spam()
            << "Cannot duplicate nodes of type " << child_node->get_type()
            << ".\n";
        }
        resist_copy = true;

      } else {
        PT(PandaNode) new_node = child_node->dupe_for_flatten();
        if (new_node->get_type() != child_node->get_type()) {
          pgraph_cat.error()
            << "Don't know how to copy nodes of type "
            << child_node->get_type() << "\n";

          if (no_unsupported_copy) {
            nassert_raise("unsupported copy");
            return;
          }
          resist_copy = true;

        } else {
          if (pgraph_cat.is_spam()) {
            pgraph_cat.spam()
              << "Duplicated " << *child_node << "\n";
          }

          new_node->copy_children(child_node);
          node->replace_child(child_node, new_node);
          child_node = new_node;
        }
      }
    }
  }

  if (resist_copy) {
    // If any of our children should have been copied but weren't, we need to
    // drop the state here before continuing.
    next_attribs.apply_to_node(node, attrib_types);
  }

  // Now it's safe to traverse through all of our children.
  nassertv(num_children == node->get_num_children());
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = node->get_child(i);
    r_apply_attribs(child_node, next_attribs, attrib_types, transformer);
  }
  Thread::consider_yield();
}


/**
 * The recursive implementation of flatten().
 */
int SceneGraphReducer::
r_flatten(PandaNode *grandparent_node, PandaNode *parent_node,
          int combine_siblings_bits) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "SceneGraphReducer::r_flatten(" << *grandparent_node << ", "
      << *parent_node << ", " << std::hex << combine_siblings_bits << std::dec
      << ")\n";
  }

  if ((combine_siblings_bits & (CS_geom_node | CS_other | CS_recurse)) != 0) {
    // Unset CS_within_radius, since we're going to flatten everything anyway.
    // This avoids needlessly calculating the bounding volume.
    combine_siblings_bits &= ~CS_within_radius;
  }

  int num_nodes = 0;

  if (!parent_node->safe_to_flatten_below()) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Not traversing further; " << *parent_node
        << " doesn't allow flattening below itself.\n";
    }

  } else {
    if ((combine_siblings_bits & CS_within_radius) != 0) {
      CPT(BoundingVolume) bv = parent_node->get_bounds();
      if (bv->is_of_type(BoundingSphere::get_class_type())) {
        const BoundingSphere *bs = DCAST(BoundingSphere, bv);
        if (pgraph_cat.is_spam()) {
          pgraph_cat.spam()
            << "considering radius of " << *parent_node
            << ": " << *bs << " vs. " << _combine_radius << "\n";
        }
        if (!bs->is_infinite() && (bs->is_empty() || bs->get_radius() <= _combine_radius)) {
          // This node fits within the specified radius; from here on down, we
          // will have CS_other set, instead of CS_within_radius.
          if (pgraph_cat.is_spam()) {
            pgraph_cat.spam()
              << "node fits within radius; flattening tighter.\n";
          }
          combine_siblings_bits &= ~CS_within_radius;
          combine_siblings_bits |= (CS_geom_node | CS_other | CS_recurse);
        }
      }
    }

    // First, recurse on each of the children.
    {
      PandaNode::Children cr = parent_node->get_children();
      int num_children = cr.get_num_children();
      for (int i = 0; i < num_children; i++) {
        PT(PandaNode) child_node = cr.get_child(i);
        num_nodes += r_flatten(parent_node, child_node, combine_siblings_bits);
      }
    }

    // Now that the above loop has removed some children, the child list saved
    // above is no longer accurate, so hereafter we must ask the node for its
    // real child list.

    // If we have CS_recurse set, then we flatten siblings before trying to
    // flatten children.  Otherwise, we flatten children first, and then
    // flatten siblings, which avoids overly enthusiastic flattening.
    if ((combine_siblings_bits & CS_recurse) != 0 &&
        parent_node->get_num_children() >= 2 &&
        parent_node->safe_to_combine_children()) {
      num_nodes += flatten_siblings(parent_node, combine_siblings_bits);
    }

    if (parent_node->get_num_children() == 1) {
      // If we now have exactly one child, consider flattening the node out.
      PT(PandaNode) child_node = parent_node->get_child(0);
      int child_sort = parent_node->get_child_sort(0);

      if (consider_child(grandparent_node, parent_node, child_node)) {
        // Ok, do it.
        parent_node->remove_child(child_node);

        if (do_flatten_child(grandparent_node, parent_node, child_node)) {
          // Done!
          num_nodes++;
        } else {
          // Chicken out.
          parent_node->add_child(child_node, child_sort);
        }
      }
    }

    if ((combine_siblings_bits & CS_recurse) == 0 &&
        (combine_siblings_bits & ~CS_recurse) != 0 &&
        parent_node->get_num_children() >= 2 &&
        parent_node->safe_to_combine_children()) {
      num_nodes += flatten_siblings(parent_node, combine_siblings_bits);
    }

    // Finally, if any of our remaining children are plain PandaNodes with no
    // children, just remove them.
    if (parent_node->safe_to_combine_children()) {
      for (int i = parent_node->get_num_children() - 1; i >= 0; --i) {
        PandaNode *child_node = parent_node->get_child(i);
        if (child_node->is_exact_type(PandaNode::get_class_type()) &&
            child_node->get_num_children() == 0 &&
            child_node->get_transform()->is_identity() &&
            child_node->get_effects()->is_empty()) {
          parent_node->remove_child(child_node);
          ++num_nodes;
        }
      }
    }
  }

  return num_nodes;
}

class SortByState {
public:
  INLINE bool
  operator () (const PandaNode *node1, const PandaNode *node2) const;
};

INLINE bool SortByState::
operator () (const PandaNode *node1, const PandaNode *node2) const {
  if (node1->get_transform() != node2->get_transform()) {
    return node1->get_transform() < node2->get_transform();
  }
  if (node1->get_state() != node2->get_state()) {
    return node1->get_state() < node2->get_state();
  }
  if (node1->get_effects() != node2->get_effects()) {
    return node1->get_effects() < node2->get_effects();
  }
  if (node1->get_draw_control_mask() != node2->get_draw_control_mask()) {
    return node1->get_draw_control_mask() < node2->get_draw_control_mask();
  }
  if (node1->get_draw_show_mask() != node2->get_draw_show_mask()) {
    return node1->get_draw_show_mask() < node2->get_draw_show_mask();
  }
  int cmp = (node1->compare_tags(node2));
  if (cmp != 0) {
    return cmp < 0;
  }

  return 0;
}


/**
 * Attempts to collapse together any pairs of siblings of the indicated node
 * that share the same properties.
 */
int SceneGraphReducer::
flatten_siblings(PandaNode *parent_node, int combine_siblings_bits) {
  int num_nodes = 0;

  // First, collect the children into groups of nodes with common properties.
  typedef plist< PT(PandaNode) > NodeList;
  typedef pmap<PandaNode *, NodeList, SortByState> Collected;
  Collected collected;

  {
    // Protect this within a local scope, so the Children member will destruct
    // and free the read pointer before we try to write to these nodes.
    PandaNode::Children cr = parent_node->get_children();
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = cr.get_child(i);
      bool safe_to_combine = child_node->safe_to_combine();
      if (safe_to_combine) {
        if (child_node->is_geom_node()) {
          safe_to_combine = (combine_siblings_bits & CS_geom_node) != 0;
        } else {
          safe_to_combine = (combine_siblings_bits & CS_other) != 0;
        }
      }

      if (safe_to_combine) {
        collected[child_node].push_back(child_node);
      }
    }
  }

  // Now visit each of those groups and try to collapse them together.  A
  // O(n^2) operation, but presumably the number of nodes in each group is
  // small.  And if each node in the group can collapse with any other node,
  // it becomes a O(n) operation.
  Collected::iterator ci;
  for (ci = collected.begin(); ci != collected.end(); ++ci) {
    const RenderEffects *effects = (*ci).first->get_effects();
    if (effects->safe_to_combine()) {
      NodeList &nodes = (*ci).second;

      NodeList::iterator ai1;
      ai1 = nodes.begin();
      while (ai1 != nodes.end()) {
        NodeList::iterator ai1_hold = ai1;
        PandaNode *child1 = (*ai1);
        ++ai1;
        NodeList::iterator ai2 = ai1;
        while (ai2 != nodes.end()) {
          NodeList::iterator ai2_hold = ai2;
          PandaNode *child2 = (*ai2);
          ++ai2;

          if (consider_siblings(parent_node, child1, child2)) {
            PT(PandaNode) new_node =
              do_flatten_siblings(parent_node, child1, child2);
            if (new_node != nullptr) {
              // We successfully collapsed a node.
              (*ai1_hold) = new_node;
              nodes.erase(ai2_hold);
              ai1 = nodes.begin();
              ai2 = nodes.end();
              num_nodes++;
            }
          }
        }
      }
    }
  }

  return num_nodes;
}

/**
 * Decides whether or not the indicated child node is a suitable candidate for
 * removal.  Returns true if the node may be removed, false if it should be
 * kept.
 */
bool SceneGraphReducer::
consider_child(PandaNode *grandparent_node, PandaNode *parent_node,
               PandaNode *child_node) {
  if (!parent_node->safe_to_combine() || !child_node->safe_to_combine()) {
    // One or both nodes cannot be safely combined with another node; do
    // nothing.
    return false;
  }

  if (parent_node->get_transform() != child_node->get_transform() ||
      parent_node->get_state() != child_node->get_state() ||
      parent_node->get_effects() != child_node->get_effects() ||
      parent_node->get_draw_control_mask() != child_node->get_draw_control_mask() ||
      parent_node->get_draw_show_mask() != child_node->get_draw_show_mask() ||
      parent_node->compare_tags(child_node) != 0) {
    // The two nodes have a different state; too bad.
    return false;
  }

  if (!parent_node->get_effects()->safe_to_combine()) {
    // The effects don't want to be combined.
    return false;
  }

  return true;
}

/**
 * Decides whether or not the indicated sibling nodes should be collapsed into
 * a single node or not.  Returns true if the nodes may be collapsed, false if
 * they should be kept distinct.
 */
bool SceneGraphReducer::
consider_siblings(PandaNode *parent_node, PandaNode *child1,
                  PandaNode *child2) {
  // We don't have to worry about the states being different betweeen child1
  // and child2, since the SortByState object already guaranteed we only
  // consider children that have the same state.
  return true;
}

/**
 * Collapses together the indicated parent node and child node and leaves the
 * result attached to the grandparent.  The return value is true if the node
 * is successfully collapsed, false if we chickened out.
 */
bool SceneGraphReducer::
do_flatten_child(PandaNode *grandparent_node, PandaNode *parent_node,
                 PandaNode *child_node) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Collapsing " << *parent_node << " and " << *child_node << "\n";
  }

  PT(PandaNode) new_parent = collapse_nodes(parent_node, child_node, false);
  if (new_parent == nullptr) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Decided not to collapse " << *parent_node
        << " and " << *child_node << "\n";
    }
    return false;
  }

  choose_name(new_parent, parent_node, child_node);

  new_parent->replace_node(child_node);
  new_parent->replace_node(parent_node);

  return true;
}

/**
 * Performs the work of collapsing two sibling nodes together into a single
 * node, leaving the resulting node attached to the parent.
 *
 * Returns a pointer to a PandaNode that reflects the combined node (which may
 * be either of the source nodes, or a new node altogether) if the siblings
 * are successfully collapsed, or NULL if we chickened out.
 */
PandaNode *SceneGraphReducer::
do_flatten_siblings(PandaNode *parent_node, PandaNode *child1,
                    PandaNode *child2) {
  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Collapsing " << *child1 << " and " << *child2 << "\n";
  }

  PT(PandaNode) new_child = collapse_nodes(child2, child1, true);
  if (new_child == nullptr) {
    if (pgraph_cat.is_spam()) {
      pgraph_cat.spam()
        << "Decided not to collapse " << *child1 << " and " << *child2 << "\n";
    }
    return nullptr;
  }

  choose_name(new_child, child2, child1);

  // Make sure the new child list has child1's children first, followed by
  // child2's children.
  child1->replace_node(child2);
  new_child->replace_node(child1);

  return new_child;
}

/**
 * Collapses the two nodes into a single node, if possible.  The 'siblings'
 * flag is true if the two nodes are siblings nodes; otherwise, node1 is a
 * parent of node2.  The return value is the resulting node, which may be
 * either one of the source nodes, or a new node altogether, or it may be NULL
 * to indicate that the collapse operation could not take place.
 */
PT(PandaNode) SceneGraphReducer::
collapse_nodes(PandaNode *node1, PandaNode *node2, bool siblings) {
  PT(PandaNode) result = node2->combine_with(node1);
  if (result == nullptr) {
    result = node1->combine_with(node2);
  }
  return result;
}


/**
 * Chooses a suitable name for the collapsed node, based on the names of the
 * two sources nodes.
 */
void SceneGraphReducer::
choose_name(PandaNode *preserve, PandaNode *source1, PandaNode *source2) {
  std::string name;
  bool got_name = false;

  name = source1->get_name();
  got_name = !name.empty() || source1->preserve_name();

  if (source2->preserve_name() || !got_name) {
    name = source2->get_name();
    got_name = !name.empty() || source2->preserve_name();
  }

  if (got_name) {
    preserve->set_name(name);
  }
}

/**
 * The recursive implementation of remove_column().
 */
int SceneGraphReducer::
r_remove_column(PandaNode *node, const InternalName *column,
                GeomTransformer &transformer) {
  int num_changed = 0;

  if (node->is_geom_node()) {
    if (transformer.remove_column(DCAST(GeomNode, node), column)) {
      ++num_changed;
    }
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    num_changed +=
      r_remove_column(children.get_child(i), column, transformer);
  }

  return num_changed;
}

/**
 * The recursive implementation of make_compatible_state().
 */
int SceneGraphReducer::
r_make_compatible_state(PandaNode *node, GeomTransformer &transformer) {
  int num_changed = 0;

  if (node->is_geom_node()) {
    if (transformer.make_compatible_state(DCAST(GeomNode, node))) {
      ++num_changed;
    }
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    num_changed +=
      r_make_compatible_state(children.get_child(i), transformer);
  }

  return num_changed;
}

/**
 * The recursive implementation of collect_vertex_data().
 */
int SceneGraphReducer::
r_collect_vertex_data(PandaNode *node, int collect_bits,
                      GeomTransformer &transformer, bool format_only) {
  int num_adjusted = 0;

  int this_node_bits = 0;
  if (node->is_of_type(ModelNode::get_class_type())) {
    this_node_bits |= CVD_model;
  }
  if (!node->get_transform()->is_identity()) {
    this_node_bits |= CVD_transform;
  }
  if (node->is_geom_node()) {
    this_node_bits |= CVD_one_node_only;
  }

  if ((collect_bits & this_node_bits) != 0) {
    // We need to start a unique collection here.
    GeomTransformer new_transformer(transformer);

    if (node->is_geom_node()) {
      // When we come to a geom node, collect.
      num_adjusted += new_transformer.collect_vertex_data(DCAST(GeomNode, node), collect_bits, format_only);
    }

    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      num_adjusted +=
        r_collect_vertex_data(children.get_child(i), collect_bits, new_transformer, format_only);
    }

    num_adjusted += new_transformer.finish_collect(format_only);

  } else {
    // Keep the same collection.

    if (node->is_geom_node()) {
      num_adjusted += transformer.collect_vertex_data(DCAST(GeomNode, node), collect_bits, format_only);
    }

    PandaNode::Children children = node->get_children();
    int num_children = children.get_num_children();
    for (int i = 0; i < num_children; ++i) {
      num_adjusted +=
        r_collect_vertex_data(children.get_child(i), collect_bits, transformer, format_only);
    }
  }

  Thread::consider_yield();
  return num_adjusted;
}

/**
 * The recursive implementation of make_nonindexed().
 */
int SceneGraphReducer::
r_make_nonindexed(PandaNode *node, int nonindexed_bits) {
  int num_changed = 0;

  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    int num_geoms = geom_node->get_num_geoms();
    for (int i = 0; i < num_geoms; ++i) {
      const Geom *geom = geom_node->get_geom(i);

      // Check whether the geom is animated or dynamic, and skip it if the
      // user specified so.
      const GeomVertexData *data = geom->get_vertex_data();
      int this_geom_bits = 0;
      if (data->get_format()->get_animation().get_animation_type() !=
          Geom::AT_none) {
        this_geom_bits |= MN_avoid_animated;
      }
      if (data->get_usage_hint() != Geom::UH_static ||
          geom->get_usage_hint() != Geom::UH_static) {
        this_geom_bits |= MN_avoid_dynamic;
      }

      if ((nonindexed_bits & this_geom_bits) == 0) {
        // The geom meets the user's qualifications for making nonindexed, so
        // do it.
        PT(Geom) mgeom = geom_node->modify_geom(i);
        num_changed += mgeom->make_nonindexed((nonindexed_bits & MN_composite_only) != 0);
      }
    }
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    num_changed +=
      r_make_nonindexed(children.get_child(i), nonindexed_bits);
  }

  return num_changed;
}

/**
 * The recursive implementation of unify().
 */
void SceneGraphReducer::
r_unify(PandaNode *node, int max_indices, bool preserve_order) {
  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    geom_node->unify(max_indices, preserve_order);
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    r_unify(children.get_child(i), max_indices, preserve_order);
  }
  Thread::consider_yield();
}

/**
 * Recursively calls GeomTransformer::register_vertices() on all GeomNodes at
 * the indicated root and below.
 */
void SceneGraphReducer::
r_register_vertices(PandaNode *node, GeomTransformer &transformer) {
  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    transformer.register_vertices(geom_node, true);
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    r_register_vertices(children.get_child(i), transformer);
  }
}

/**
 * The recursive implementation of decompose().
 */
void SceneGraphReducer::
r_decompose(PandaNode *node) {
  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    geom_node->decompose();
  }

  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (int i = 0; i < num_children; ++i) {
    r_decompose(children.get_child(i));
  }
}

/**
 * The recursive implementation of premunge().
 */
void SceneGraphReducer::
r_premunge(PandaNode *node, const RenderState *state) {
  CPT(RenderState) next_state = state->compose(node->get_state());

  if (node->is_geom_node()) {
    GeomNode *geom_node = DCAST(GeomNode, node);
    geom_node->do_premunge(_gsg, next_state, _transformer);
  }

  int i;
  PandaNode::Children children = node->get_children();
  int num_children = children.get_num_children();
  for (i = 0; i < num_children; ++i) {
    r_premunge(children.get_child(i), next_state);
  }

  PandaNode::Stashed stashed = node->get_stashed();
  int num_stashed = stashed.get_num_stashed();
  for (i = 0; i < num_stashed; ++i) {
    r_premunge(stashed.get_stashed(i), next_state);
  }
}
