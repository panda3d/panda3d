// Filename: qpqpSceneGraphReducer.cxx
// Created by:  drose (14Mar02)
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

#include "qpsceneGraphReducer.h"
#include "config_pgraph.h"
#include "colorAttrib.h"
#include "texMatrixAttrib.h"
#include "colorScaleAttrib.h"

#include "qpgeomNode.h"
#include "geom.h"
#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::AccumulatedAttribs::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::AccumulatedAttribs::
write(ostream &out, int attrib_types, int indent_level) const {
  if ((attrib_types & TT_transform) != 0) {
    _transform->write(out, indent_level);
  }
  if ((attrib_types & TT_color) != 0) {
    if (_color == (const RenderAttrib *)NULL) {
      indent(out, indent_level) << "no color\n";
    } else {
      _color->write(out, indent_level);
    }
  }
  if ((attrib_types & TT_color_scale) != 0) {
    if (_color_scale == (const RenderAttrib *)NULL) {
      indent(out, indent_level) << "no color scale\n";
    } else {
      _color_scale->write(out, indent_level);
    }
  }
  if ((attrib_types & TT_tex_matrix) != 0) {
    if (_tex_matrix == (const RenderAttrib *)NULL) {
      indent(out, indent_level) << "no tex matrix\n";
    } else {
      _tex_matrix->write(out, indent_level);
    }
  }
  if ((attrib_types & TT_other) != 0) {
    _other->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::AccumulatedAttribs::collect
//       Access: Public
//  Description: Collects the state and transform from the indicated
//               node and adds it to the accumulator, removing it from
//               the node.
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::AccumulatedAttribs::
collect(PandaNode *node, int attrib_types) {
  if ((attrib_types & TT_transform) != 0) {
    // Collect the node's transform.
    nassertv(_transform != (TransformState *)NULL);
    _transform = _transform->compose(node->get_transform());
    node->set_transform(TransformState::make_identity());
  }

  if ((attrib_types & TT_color) != 0) {
    const RenderAttrib *node_attrib = 
      node->get_attrib(ColorAttrib::get_class_type());
    if (node_attrib != (const RenderAttrib *)NULL) {
      // The node has a color attribute; apply it.
      if (_color == (const RenderAttrib *)NULL) {
        _color = node_attrib;
      } else {
        _color = _color->compose(node_attrib);
      }
      node->clear_attrib(ColorAttrib::get_class_type());
    }
  }

  if ((attrib_types & TT_color_scale) != 0) {
    const RenderAttrib *node_attrib = 
      node->get_attrib(ColorScaleAttrib::get_class_type());
    if (node_attrib != (const RenderAttrib *)NULL) {
      // The node has a color scale attribute; apply it.
      if (_color_scale == (const RenderAttrib *)NULL) {
        _color_scale = node_attrib;
      } else {
        _color_scale = _color_scale->compose(node_attrib);
      }
      node->clear_attrib(ColorScaleAttrib::get_class_type());
    }
  }

  if ((attrib_types & TT_tex_matrix) != 0) {
    const RenderAttrib *node_attrib = 
      node->get_attrib(TexMatrixAttrib::get_class_type());
    if (node_attrib != (const RenderAttrib *)NULL) {
      // The node has a tex matrix attribute; apply it.
      if (_tex_matrix == (const RenderAttrib *)NULL) {
        _tex_matrix = node_attrib;
      } else {
        _tex_matrix = _tex_matrix->compose(node_attrib);
      }
      node->clear_attrib(TexMatrixAttrib::get_class_type());
    }
  }

  if ((attrib_types & TT_transform) != 0) {
    // Collect everything else.
    nassertv(_other != (RenderState *)NULL);
    _other = _other->compose(node->get_state());
    node->set_state(RenderState::make_empty());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::AccumulatedAttribs::apply_to_node
//       Access: Public
//  Description: Stores the indicated attributes in the node's
//               transform and state information; does not attempt to
//               apply the properties to the vertices.  Clears the
//               attributes from the accumulator for future
//               traversals.
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::AccumulatedAttribs::
apply_to_node(PandaNode *node, int attrib_types) {
  if ((attrib_types & TT_transform) != 0) {
    node->set_transform(_transform->compose(node->get_transform()));
    _transform = TransformState::make_identity();
  }

  if ((attrib_types & TT_color) != 0) {
    if (_color != (RenderAttrib *)NULL) {
      const RenderAttrib *node_attrib =
        node->get_attrib(ColorAttrib::get_class_type());
      if (node_attrib != (RenderAttrib *)NULL) {
        node->set_attrib(_color->compose(node_attrib));
      } else {
        node->set_attrib(_color);
      }
      _color = (RenderAttrib *)NULL;
    }
  }

  if ((attrib_types & TT_color_scale) != 0) {
    if (_color_scale != (RenderAttrib *)NULL) {
      const RenderAttrib *node_attrib =
        node->get_attrib(ColorScaleAttrib::get_class_type());
      if (node_attrib != (RenderAttrib *)NULL) {
        node->set_attrib(_color_scale->compose(node_attrib));
      } else {
        node->set_attrib(_color_scale);
      }
      _color_scale = (RenderAttrib *)NULL;
    }
  }

  if ((attrib_types & TT_tex_matrix) != 0) {
    if (_tex_matrix != (RenderAttrib *)NULL) {
      const RenderAttrib *node_attrib =
        node->get_attrib(TexMatrixAttrib::get_class_type());
      if (node_attrib != (RenderAttrib *)NULL) {
        node->set_attrib(_tex_matrix->compose(node_attrib));
      } else {
        node->set_attrib(_tex_matrix);
      }
      _tex_matrix = (RenderAttrib *)NULL;
    }
  }

  if ((attrib_types & TT_other) != 0) {
    node->set_state(_other->compose(node->get_state()));
    _other = RenderState::make_empty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::AccumulatedAttribs::apply_to_vertices
//       Access: Public
//  Description: Applies the indicated attributes to the node so that
//               they do not need to be stored as separate attributes
//               any more.
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::AccumulatedAttribs::
apply_to_vertices(PandaNode *node, int attrib_types, 
                  qpGeomTransformer &transformer) {
  if (node->is_geom_node()) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Transforming geometry.\n";
    }

    // We treat GeomNodes as a special case, since we can apply more
    // than just a transform matrix and so we can share vertex arrays
    // across different GeomNodes.
    qpGeomNode *gnode = DCAST(qpGeomNode, node);
    if ((attrib_types & TT_transform) != 0) {
      if (!_transform->is_identity()) {
        transformer.transform_vertices(gnode, _transform->get_mat());
      }
    }
    if ((attrib_types & TT_color) != 0) {
      if (_color != (const RenderAttrib *)NULL) {
        const ColorAttrib *ca = DCAST(ColorAttrib, _color);
        if (ca->get_color_type() == ColorAttrib::T_flat) {
          transformer.set_color(gnode, ca->get_color());
        }
      }
    }
    if ((attrib_types & TT_color_scale) != 0) {
      if (_color_scale != (const RenderAttrib *)NULL) {
        const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, _color_scale);
        if (csa->get_scale() != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f)) {
          transformer.transform_colors(gnode, csa->get_scale());
        }
      }
    }
    if ((attrib_types & TT_tex_matrix) != 0) {
      if (_tex_matrix != (const RenderAttrib *)NULL) {
        const TexMatrixAttrib *tma = DCAST(TexMatrixAttrib, _tex_matrix);
        if (tma->get_mat() != LMatrix4f::ident_mat()) {
          transformer.transform_texcoords(gnode, tma->get_mat());
        }
      }
    }
    if ((attrib_types & TT_other) != 0) {
      if (!_other->is_empty()) {
        transformer.apply_state(gnode, _other);
      }
    }

  } else {
    // This handles any kind of node other than a GeomNode.
    if ((attrib_types & TT_transform) != 0) {
      node->xform(_transform->get_mat());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
qpSceneGraphReducer::
qpSceneGraphReducer() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
qpSceneGraphReducer::
~qpSceneGraphReducer() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::apply_attribs
//       Access: Public
//  Description: Walks the scene graph, accumulating attribs of
//               the indicated types, applying them to the vertices,
//               and removing them from the scene graph.  This has a
//               performance optimization benefit in itself, but is
//               especially useful to pave the way for a call to
//               flatten() and greatly improve the effectiveness of
//               the flattening operation.
//
//               Multiply instanced geometry is duplicated before the
//               attribs are applied.
//
//               Of course, this operation does make certain dynamic
//               operations impossible.
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::
apply_attribs(PandaNode *node, int attrib_types) {
  AccumulatedAttribs trans;

  trans._transform = TransformState::make_identity();
  trans._color = (ColorAttrib *)NULL;
  trans._color_scale = (ColorScaleAttrib *)NULL;
  trans._tex_matrix = (TexMatrixAttrib *)NULL;
  trans._other = RenderState::make_empty();

  r_apply_attribs(node, attrib_types, trans);
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::flatten
//       Access: Public
//  Description: Simplifies the graph by removing unnecessary nodes
//               and nodes.
//
//               In general, a node (and its parent node) is a
//               candidate for removal if the node has no siblings and
//               the node and node have no special properties.  The
//               definition of what, precisely, is a 'special
//               property' may be extended by subclassing from this
//               type and redefining consider_node() appropriately.
//
//               If combine_siblings is true, sibling nodes may also
//               be collapsed into a single node.  This will further
//               reduce scene graph complexity, sometimes
//               substantially, at the cost of reduced spatial
//               separation.
//
//               Returns the number of nodes removed from the graph.
////////////////////////////////////////////////////////////////////
int qpSceneGraphReducer::
flatten(PandaNode *root, bool combine_siblings) {
  int num_total_nodes = 0;
  int num_pass_nodes;

  do {
    num_pass_nodes = 0;

    // Get a copy of the children list, so we don't have to worry
    // about self-modifications.
    PandaNode::ChildrenCopy cr = root->get_children_copy();

    // Now visit each of the children in turn.
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = cr.get_child(i);
      num_pass_nodes += r_flatten(root, child_node, combine_siblings);
    }

    if (combine_siblings && root->get_num_children() >= 2) {
      num_pass_nodes += flatten_siblings(root);
    }

    num_total_nodes += num_pass_nodes;

    // If combine_siblings is true, we should repeat the above until
    // we don't get any more benefit from flattening, because each
    // pass could convert cousins into siblings, which may get
    // flattened next pass.  If combine_siblings is not true, the
    // first pass will be fully effective, and there's no point in
    // trying again.
  } while (combine_siblings && num_pass_nodes != 0);

  return num_total_nodes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::r_apply_attribs
//       Access: Protected
//  Description: The recursive implementation of apply_attribs().
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::
r_apply_attribs(PandaNode *node, int attrib_types,
                qpSceneGraphReducer::AccumulatedAttribs trans) {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "r_apply_attribs(" << *node << "), node's attribs are:\n";
    node->get_transform()->write(pgraph_cat.debug(false), 2);
    node->get_state()->write(pgraph_cat.debug(false), 2);
    node->get_effects()->write(pgraph_cat.debug(false), 2);
  }

  trans.collect(node, attrib_types);

  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Got attribs from " << *node << "\n"
      << "Accumulated attribs are:\n";
    trans.write(pgraph_cat.debug(false), attrib_types, 2);
  }

  // Check to see if we can't propagate any of these attribs past
  // this node for some reason.
  int apply_types = 0;

  const RenderEffects *effects = node->get_effects();
  if (!effects->safe_to_transform()) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Node " << *node
        << " contains a non-transformable effect; leaving transform here.\n";
    }
    apply_types |= TT_transform;
  }
  if (!node->safe_to_transform()) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Cannot safely transform nodes of type " << node->get_type()
        << "; leaving a transform here but carrying on otherwise.\n";
    }
    apply_types |= TT_transform;
  }

  // Directly store whatever attributes we must,
  trans.apply_to_node(node, attrib_types & apply_types);

  // And apply the rest to the vertices.
  trans.apply_to_vertices(node, attrib_types, _transformer);

  // Do we need to copy any children to flatten instances?
  bool resist_copy = false;
  int num_children = node->get_num_children();
  int i;
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = node->get_child(i);

    if (child_node->get_num_parents() > 1) {
      if (!child_node->safe_to_flatten()) {
        if (pgraph_cat.is_debug()) {
          pgraph_cat.debug()
            << "Cannot duplicate nodes of type " << child_node->get_type()
            << ".\n";
          resist_copy = true;
        }

      } else {
        PT(PandaNode) new_node = child_node->make_copy();
        if (new_node->get_type() != child_node->get_type()) {
          pgraph_cat.error()
            << "Don't know how to copy nodes of type "
            << child_node->get_type() << "\n";
          resist_copy = true;

        } else {
          if (pgraph_cat.is_debug()) {
            pgraph_cat.debug()
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
    // If any of our children should have been copied but weren't, we
    // need to drop the state here before continuing.
    trans.apply_to_node(node, attrib_types);
  }

  // Now it's safe to traverse through all of our children.
  nassertv(num_children == node->get_num_children());
  for (i = 0; i < num_children; i++) {
    PandaNode *child_node = node->get_child(i);
    r_apply_attribs(child_node, attrib_types, trans);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::r_flatten
//       Access: Protected
//  Description: The recursive implementation of flatten().
////////////////////////////////////////////////////////////////////
int qpSceneGraphReducer::
r_flatten(PandaNode *grandparent_node, PandaNode *parent_node,
          bool combine_siblings) {
  int num_nodes = 0;

  // First, recurse on each of the children.
  {
    PandaNode::ChildrenCopy cr = parent_node->get_children_copy();
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = cr.get_child(i);
      num_nodes += r_flatten(parent_node, child_node, combine_siblings);
    }
  }

  // Now that the above loop has removed some children, the child list
  // saved above is no longer accurate, so hereafter we must ask the
  // node for its real child list.
  if (combine_siblings && parent_node->get_num_children() >= 2) {
    num_nodes += flatten_siblings(parent_node);
  }

  if (parent_node->get_num_children() == 1) {
    // If we now have exactly one child, consider flattening the node
    // out.
    PT(PandaNode) child_node = parent_node->get_child(0);
    int child_sort = parent_node->get_child_sort(0);

    if (consider_child(grandparent_node, parent_node, child_node)) {
      // Ok, do it.
      parent_node->remove_child(0);

      if (do_flatten_child(grandparent_node, parent_node, child_node)) {
        // Done!
        num_nodes++;
      } else {
        // Chicken out.
        parent_node->add_child(child_node, child_sort);
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
  return node1->get_effects() < node2->get_effects();
}


////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::flatten_siblings
//       Access: Protected
//  Description: Attempts to collapse together any pairs of siblings
//               of the indicated node that share the same properties.
////////////////////////////////////////////////////////////////////
int qpSceneGraphReducer::
flatten_siblings(PandaNode *parent_node) {
  int num_nodes = 0;

  // First, collect the children into groups of nodes with common
  // properties.
  typedef plist< PT(PandaNode) > NodeList;
  typedef pmap<PandaNode *, NodeList, SortByState> Collected;
  Collected collected;

  {
    // Protect this within a local scope, so the Children member will
    // destruct and free the read pointer before we try to write to
    // these nodes.
    PandaNode::Children cr = parent_node->get_children();
    int num_children = cr.get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child_node = cr.get_child(i);
      if (child_node->safe_to_combine()) {
        collected[child_node].push_back(child_node);
      }
    }
  }

  // Now visit each of those groups and try to collapse them together.
  // A O(n^2) operation, but presumably the number of nodes in each
  // group is small.  And if each node in the group can collapse with
  // any other node, it becomes a O(n) operation.
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
            if (new_node != (PandaNode *)NULL) {
              // We successfully collapsed a node.
              nodes.erase(ai2_hold);
              nodes.erase(ai1_hold);
              nodes.push_back(new_node);
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

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::consider_child
//       Access: Protected, Virtual
//  Description: Decides whether or not the indicated child node is a
//               suitable candidate for removal.  Returns true if the
//               node may be removed, false if it should be kept.  This
//               function may be extended in a user class to protect
//               special kinds of nodes from deletion.
////////////////////////////////////////////////////////////////////
bool qpSceneGraphReducer::
consider_child(PandaNode *grandparent_node, PandaNode *parent_node, 
               PandaNode *child_node) {
  if (!parent_node->safe_to_combine() || !child_node->safe_to_combine()) {
    // One or both nodes cannot be safely combined with another node;
    // do nothing.
    return false;
  }

  if (parent_node->get_transform() != child_node->get_transform() ||
      parent_node->get_state() != child_node->get_state() ||
      parent_node->get_effects() != child_node->get_effects()) {
    // The two nodes have a different state; too bad.
    return false;
  }

  if (!parent_node->get_effects()->safe_to_combine()) {
    // The effects don't want to be combined.
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::consider_siblings
//       Access: Protected, Virtual
//  Description: Decides whether or not the indicated sibling nodes
//               should be collapsed into a single node or not.
//               Returns true if the nodes may be collapsed, false if
//               they should be kept distinct.
////////////////////////////////////////////////////////////////////
bool qpSceneGraphReducer::
consider_siblings(PandaNode *parent_node, PandaNode *child1,
                  PandaNode *child2) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::do_flatten_child
//       Access: Protected, Virtual
//  Description: Collapses together the indicated parent node and
//               child node and leaves the result attached to the
//               grandparent.  The return value is true if the node is
//               successfully collapsed, false if we chickened out.
//
//               This function may be extended in a user class to
//               handle special kinds of nodes.
////////////////////////////////////////////////////////////////////
bool qpSceneGraphReducer::
do_flatten_child(PandaNode *grandparent_node, PandaNode *parent_node, 
                 PandaNode *child_node) {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Collapsing " << *parent_node << " and " << *child_node << "\n";
  }

  PT(PandaNode) new_parent = collapse_nodes(parent_node, child_node, false);
  if (new_parent == (PandaNode *)NULL) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Decided not to collapse " << *parent_node 
        << " and " << *child_node << "\n";
    }
    return false;
  }

  choose_name(new_parent, parent_node, child_node);

  if (new_parent != child_node) {
    new_parent->steal_children(child_node);
  }

  if (new_parent != parent_node) {
    grandparent_node->replace_child(parent_node, new_parent);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::do_flatten_siblings
//       Access: Protected, Virtual
//  Description: Performs the work of collapsing two sibling nodes
//               together into a single node, leaving the resulting
//               node attached to the parent.
//
//               Returns a pointer to a PandaNode the reflects the
//               combined node (which may be either of the source nodes,
//               or a new node altogether) if the siblings are
//               successfully collapsed, or NULL if we chickened out.
////////////////////////////////////////////////////////////////////
PandaNode *qpSceneGraphReducer::
do_flatten_siblings(PandaNode *parent_node, PandaNode *child1,
                    PandaNode *child2) {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Collapsing " << *child1 << " and " << *child2 << "\n";
  }

  PT(PandaNode) new_child = collapse_nodes(child2, child1, true);
  if (new_child == (PandaNode *)NULL) {
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Decided not to collapse " << *child1 << " and " << *child2 << "\n";
    }
    return NULL;
  }

  choose_name(new_child, child2, child1);

  if (new_child == child1) {
    new_child->steal_children(child2);
    parent_node->remove_child(child2);

  } else if (new_child == child2) {
    new_child->steal_children(child1);
    parent_node->remove_child(child1);

  } else {
    new_child->steal_children(child1);
    new_child->steal_children(child2);
    parent_node->remove_child(child2);
    parent_node->replace_child(child1, new_child);
  }

  return new_child;
}

////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::collapse_nodes
//       Access: Protected, Virtual
//  Description: Collapses the two nodes into a single node, if
//               possible.  The 'siblings' flag is true if the two
//               nodes are siblings nodes; otherwise, node1 is a
//               parent of node2.  The return value is the resulting
//               node, which may be either one of the source nodes, or
//               a new node altogether, or it may be NULL to indicate
//               that the collapse operation could not take place.
//
//               This function may be extended in a user class to
//               handle combining special kinds of nodes.
////////////////////////////////////////////////////////////////////
PT(PandaNode) qpSceneGraphReducer::
collapse_nodes(PandaNode *node1, PandaNode *node2, bool siblings) {
  return node2->combine_with(node1);
}


////////////////////////////////////////////////////////////////////
//     Function: qpSceneGraphReducer::choose_name
//       Access: Protected, Virtual
//  Description: Chooses a suitable name for the collapsed node, based
//               on the names of the two sources nodes.
////////////////////////////////////////////////////////////////////
void qpSceneGraphReducer::
choose_name(PandaNode *preserve, PandaNode *source1, PandaNode *source2) {
  string name;
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
