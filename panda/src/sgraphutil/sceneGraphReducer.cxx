// Filename: sceneGraphReducer.cxx
// Created by:  drose (22May00)
// 
////////////////////////////////////////////////////////////////////

#include "sceneGraphReducer.h"
#include "config_sgraphutil.h"

#include <geomNode.h>
#include <geom.h>
#include <indent.h>
#include <billboardTransition.h>


////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::apply_to_arc
//       Access: Public
//  Description: Stores the relevant transitions on the indicated arc,
//               and clears the transitions for continuing.
////////////////////////////////////////////////////////////////////
void SceneGraphReducer::AccumulatedTransitions::
apply_to_arc(NodeRelation *arc, int transition_types) {
  if ((transition_types & TT_transform) != 0) {
    if (!_transform->get_matrix().almost_equal(LMatrix4f::ident_mat())) {
      arc->set_transition(_transform);
    }
    _transform = new TransformTransition;
  }
  
  if ((transition_types & TT_color) != 0) {
    if (!_color->is_identity()) {
      arc->set_transition(_color);
      _color = new ColorTransition;
    }
  }
  
  if ((transition_types & TT_texture_matrix) != 0) {
    if (!_texture_matrix->get_matrix().almost_equal(LMatrix4f::ident_mat())) {
      arc->set_transition(_texture_matrix);
    }
    _texture_matrix = new TexMatrixTransition;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::AccumulatedTransitions::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void SceneGraphReducer::AccumulatedTransitions::
write(ostream &out, int transition_types, int indent_level) const {
  if ((transition_types & TT_transform) != 0) {
    _transform->write(out, indent_level);
  }
  if ((transition_types & TT_color) != 0) {
    _color->write(out, indent_level);
  }
  if ((transition_types & TT_texture_matrix) != 0) {
    _texture_matrix->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SceneGraphReducer::
SceneGraphReducer(TypeHandle graph_type) :
  GraphReducer(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::apply_transitions
//       Access: Public
//  Description: Walks the scene graph, accumulating transitions of
//               the indicated types, applying them to the vertices,
//               and removing them from the scene graph.  This has a
//               performance optimization benefit in itself, but is
//               especially useful to pave the way for a call to
//               flatten() and greatly improve the effectiveness of
//               the flattening operation.
//
//               Multiply instanced geometry is duplicated before the
//               transitions are applied.
//
//               Of course, this operation does make certain dynamic
//               operations impossible.
////////////////////////////////////////////////////////////////////
void SceneGraphReducer::
apply_transitions(NodeRelation *arc, int transition_types) {
  AccumulatedTransitions trans;
  if ((transition_types & TT_transform) != 0) {
    trans._transform = new TransformTransition;
  }
  if ((transition_types & TT_color) != 0) {
    trans._color = new ColorTransition;
  }
  if ((transition_types & TT_texture_matrix) != 0) {
    trans._texture_matrix = new TexMatrixTransition;
  }

  r_apply_transitions(arc, transition_types, trans, false);
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::r_apply_transitions
//       Access: Protected
//  Description: The recursive implementation of apply_transitions().
////////////////////////////////////////////////////////////////////
void SceneGraphReducer::
r_apply_transitions(NodeRelation *arc, int transition_types,
		    SceneGraphReducer::AccumulatedTransitions trans,
		    bool duplicate) {
  if (sgraphutil_cat.is_debug()) {
    sgraphutil_cat.debug()
      << "r_apply_transitions(" << *arc << "), arc's transitions are:\n";
    arc->write_transitions(sgraphutil_cat.debug(false), 2);
  }

  if ((transition_types & TT_transform) != 0) {
    nassertv(trans._transform != (TransformTransition *)NULL);
    TransformTransition *tt;
    if (get_transition_into(tt, arc)) {
      trans._transform =
	DCAST(TransformTransition, trans._transform->compose(tt));
      arc->clear_transition(TransformTransition::get_class_type());
    }
  }

  if ((transition_types & TT_color) != 0) {
    nassertv(trans._color != (ColorTransition *)NULL);
    ColorTransition *ct;
    if (get_transition_into(ct, arc)) {
      trans._color =
	DCAST(ColorTransition, trans._color->compose(ct));
      arc->clear_transition(ColorTransition::get_class_type());
    }
  }

  if ((transition_types & TT_texture_matrix) != 0) {
    nassertv(trans._texture_matrix != (TexMatrixTransition *)NULL);
    TexMatrixTransition *tmt;
    if (get_transition_into(tmt, arc)) {
      trans._texture_matrix = 
	DCAST(TexMatrixTransition, trans._texture_matrix->compose(tmt));
      arc->clear_transition(TexMatrixTransition::get_class_type());
    }
  }

  PT(Node) node = arc->get_child();
  nassertv(node != (Node *)NULL);

  if (sgraphutil_cat.is_debug()) {
    sgraphutil_cat.debug()
      << "Applying transitions to " << *node << "\n"
      << "Accumulated transitions are:\n";
    trans.write(sgraphutil_cat.debug(false), transition_types, 2);
  }


  if (node->get_num_parents(_graph_type) > 1) {
    // This node has multiple instances; we need to duplicate
    // everything in the graph from this point down.
    duplicate = true;
  }

  if (duplicate) {
    // Make another copy of the Node.

    if (!node->safe_to_flatten()) {
      // Oops!  We *can't* flatten below this node.  We'll have to stop
      // here.  Drop transitions onto this arc to reflect what's been
      // accumulated so far.
      if (sgraphutil_cat.is_debug()) {
	sgraphutil_cat.debug()
	  << "Cannot duplicate nodes of type " << node->get_type()
	  << "; dropping transitions here and stopping.\n";
      }

      trans.apply_to_arc(arc, transition_types);
      return;
    }

    PT(Node) new_node = node->make_copy();
    if (new_node->get_type() != node->get_type()) {
      sgraphutil_cat.error()
	<< "Cannot apply transitions to " << *node 
	<< "; don't know how to copy nodes of this type.\n";

      trans.apply_to_arc(arc, transition_types);
      return;
    }

    if (sgraphutil_cat.is_debug()) {
      sgraphutil_cat.debug()
	<< "Duplicated " << *node << "\n";
    }

    copy_children(new_node, node);
    node = new_node.p();
    arc->change_child(node);
  }

  // Check to see if we can't propagate any of these transitions past
  // this arc for some reason.  A little bit kludgey, since we have to
  // know about all the special kinds of transitions, but attempting
  // to make this general and also work correctly is kind of tricky.
  int apply_types = 0;
  if (arc->has_transition(BillboardTransition::get_class_type())) {
    if (sgraphutil_cat.is_debug()) {
      sgraphutil_cat.debug()
	<< "Arc " << *arc 
	<< " contains a BillboardTransition; leaving transform here.\n";
    }
    apply_types |= TT_transform;
  }
  if (!node->safe_to_transform()) {
    if (sgraphutil_cat.is_debug()) {
      sgraphutil_cat.debug()
	<< "Cannot safely transform nodes of type " << node->get_type()
	<< "; leaving a transform here but carrying on otherwise.\n";
    }
    apply_types |= TT_transform;
  }    
  trans.apply_to_arc(arc, transition_types & apply_types);

  // Now apply what's left to the vertices.
  if (node->is_of_type(GeomNode::get_class_type())) {
    if (sgraphutil_cat.is_debug()) {
      sgraphutil_cat.debug()
	<< "Transforming geometry.\n";
    }

    // We treat GeomNodes as a special case, since we can apply more
    // than just a transform matrix and so we can share vertex arrays
    // across different GeomNodes.
    GeomNode *gnode = DCAST(GeomNode, node);
    if ((transition_types & TT_transform) != 0) {
      if (trans._transform->get_matrix() != LMatrix4f::ident_mat()) {
	_transformer.transform_vertices(gnode, trans._transform->get_matrix());
      }
    }
    if ((transition_types & TT_color) != 0) {
      if (trans._color->is_on() && trans._color->is_real()) {
	_transformer.set_color(gnode, trans._color->get_color());
      }
    }
    if ((transition_types & TT_texture_matrix) != 0) {
      if (trans._texture_matrix->get_matrix() != LMatrix4f::ident_mat()) {
	_transformer.transform_texcoords(gnode, 
					 trans._texture_matrix->get_matrix());
      }
    }
    
  } else {
    // This handles any kind of node other than a GeomNode.
    if ((transition_types & TT_transform) != 0) {
      node->xform(trans._transform->get_matrix());
    }
  }
  
  DownRelations::const_iterator dri;
  dri = node->_children.find(_graph_type);
  if (dri != node->_children.end()) {
    const DownRelationPointers &drp = (*dri).second;
    DownRelationPointers drp_copy = drp;

    DownRelationPointers::const_iterator drpi;
    for (drpi = drp_copy.begin(); drpi != drp_copy.end(); ++drpi) {
      NodeRelation *child_arc = (*drpi);

      r_apply_transitions(child_arc, transition_types, trans, duplicate);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SceneGraphReducer::collapse_nodes
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
Node *SceneGraphReducer::
collapse_nodes(Node *node1, Node *node2, bool siblings) {
  // We can collapse two GeomNodes easily, if they're siblings.  If
  // they're parent-child, we'd probably better not (it might
  // interfere with decaling).
  if (siblings && 
      node1->is_exact_type(GeomNode::get_class_type()) &&
      node2->is_exact_type(GeomNode::get_class_type())) {
    GeomNode *gnode1;
    DCAST_INTO_R(gnode1, node1, NULL);
    GeomNode *gnode2;
    DCAST_INTO_R(gnode2, node2, NULL);
    gnode1->add_geoms_from(gnode2);
    return gnode1;

  } else {
    return GraphReducer::collapse_nodes(node1, node2, siblings);
  }
}
