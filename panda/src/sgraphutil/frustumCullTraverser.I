// Filename: frustumCullTraverser.I
// Created by:  drose (14Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "frustumCullTraverser.h"
#include "get_rel_pos.h"
#include "config_sgraphutil.h"

#include <notify.h>
#include <nodeTransitionWrapper.h>
#include <wrt.h>

////////////////////////////////////////////////////////////////////
//     Function: FrustumCullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
FrustumCullTraverser<Visitor, LevelState>::
FrustumCullTraverser(Node *root,
		     Visitor &visitor,
		     const AttributeWrapper &initial_render_state,
		     const LevelState &initial_level_state,
		     GraphicsStateGuardian *gsg, 
		     TypeHandle graph_type) :
  _visitor(visitor),
  _initial_render_state(initial_render_state),
  _gsg(gsg),
  _graph_type(graph_type)
{
  _view_frustum = NULL;
  PT(GeometricBoundingVolume) local_frustum;

  if (view_frustum_cull && _gsg != (GraphicsStateGuardian *)NULL) {
    // If we're to be performing view-frustum culling, determine the
    // bounding volume associated with the current viewing frustum.

    const ProjectionNode *camera = _gsg->get_current_projection_node();
    if (camera != (const ProjectionNode *)NULL) {
      const Projection *proj = camera->get_projection();
      nassertv(proj != (const Projection *)NULL);
      BoundingVolume *bv = proj->make_bounds();

      if (bv->is_of_type(GeometricBoundingVolume::get_class_type())) {
	_view_frustum = DCAST(GeometricBoundingVolume, bv);
      } else {
	delete bv;
      }
    }

    LMatrix4f mat;
    get_rel_mat(_gsg->get_current_projection_node(), root, mat);

    local_frustum = DCAST(GeometricBoundingVolume, _view_frustum->make_copy());
    local_frustum->xform(mat);

    if (sgraphutil_cat.is_spam()) {
      sgraphutil_cat.spam()
	<< "Beginning frustum cull, frustum is: " << *local_frustum << "\n"
	<< "Transform is:\n";
      mat.write(sgraphutil_cat.spam(false), 2);
    }
  }

  LevelState level_state(initial_level_state);
  traverse(root, _initial_render_state, level_state, local_frustum, false);

  _view_frustum = NULL;
  nassertv(_arc_stack.empty());
}


////////////////////////////////////////////////////////////////////
//     Function: FrustumCullTraverser::Traverse
//       Access: Public
//  Description: Walks to the next arc of the graph.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
void FrustumCullTraverser<Visitor, LevelState>::
traverse(NodeRelation *arc, AttributeWrapper render_state, 
	 LevelState level_state, PT(GeometricBoundingVolume) local_frustum,
	 bool all_in) {
  // local_frustum, above, is a PT(GeometricBoundingVolume), instead
  // of just an ordinary GeometricBoundingVolume*, because we might
  // reassign it within this function to a locally-allocated volume
  // which we expect to automatically destruct when the function
  // terminates.
  nassertv(arc->get_child() != (Node *)NULL);

  bool carry_on = true;
  bool pushed_arc_stack = false;

  // First, if we're performing view-frustum culling, check the
  // bounding volume associated with this arc (which bounds all of its
  // children) against the bounding volume of our view frustum.

  if (_view_frustum != (GeometricBoundingVolume *)NULL) {
    // If this arc's child has multiple parents, we need to save the
    // arc on the arc stack, so we can unambiguously wrt().
    if (arc->get_child()->get_num_parents(_graph_type) > 1) {
      _arc_stack.push_back(arc);
      pushed_arc_stack = true;
    }

    if (!all_in) {
      // Now test for intersection with the bounding volume.
      const BoundingVolume &arc_volume = arc->get_bound();
      if (arc_volume.is_of_type(GeometricBoundingVolume::get_class_type())) {
	const GeometricBoundingVolume *arc_gbv =
	  DCAST(GeometricBoundingVolume, &arc_volume);

	nassertv(local_frustum != (GeometricBoundingVolume *)NULL);
	int result = local_frustum->contains(arc_gbv);
	if (result == BoundingVolume::IF_no_intersection) {
	  // No intersection at all.  Cull.
	  if (sgraphutil_cat.is_spam() && !arc_volume.is_empty()) {
	    sgraphutil_cat.spam()
	      << "Culling " << *arc->get_child() << " with volume "
	      << arc_volume << "\n";
	  }

	  /*
	  if (fake_view_frustum_cull) {
	    // In fake mode, we just render everything in red
	    // wireframe instead of actually culling it.

	    ColorTransition *c = 
	      new ColorTransition(1.0, 0.0, 0.0, 1.0);
	    RenderModeTransition *w = 
	      new RenderModeTransition(RenderModeProperty::M_wireframe);
	    TextureTransition *t =
	      new TextureTransition(TextureTransition::off());

	    c->set_priority(100);
	    w->set_priority(100);
	    t->set_priority(100);
	    AllTransitionsWrapper trans;
	    trans.set_transition(c);
	    trans.set_transition(w);
	    trans.set_transition(t);

	    post.apply_in_place(trans);
	    all_in = true;
	  
	    } else */ {
	    carry_on = false;
	  }
	  
	} else if ((result & BoundingVolume::IF_all) != 0) {
	  // The arc and its descendants are completely enclosed within
	  // the frustum.  No need to cull further.
	  all_in = true;
	}
      }

      if (carry_on &&
	  arc->has_transition(TransformTransition::get_class_type())) {
	// Now apply the transform associated with this arc.  We do
	// this after the bounding volume test, because the bounding
	// volume has already been transformed.

	local_frustum =
	  DCAST(GeometricBoundingVolume, _view_frustum->make_copy());

	NodeTransitionWrapper ntw(TransformTransition::get_class_type());
	wrt(_gsg->get_current_projection_node(),
	    arc->get_child(), _arc_stack.begin(), _arc_stack.end(),
	    ntw, _graph_type);
	
	const TransformTransition *tt;
	if (get_transition_into(tt, ntw)) {
	  // This frustum is transformed from the camera.  Most will
	  // be.
	  nassertv(local_frustum != (GeometricBoundingVolume *)NULL);
	  local_frustum->xform(tt->get_matrix());

	  if (sgraphutil_cat.is_spam()) {
	    sgraphutil_cat.spam()
	      << "Transforming frustum into local space of " << *arc << ": " << *local_frustum << "\n"
	      << "Transform is:\n";
	    tt->get_matrix().write(sgraphutil_cat.spam(false), 2);
	  }
	}
      }
    }
  }

  if (carry_on) {
    // Now give the visitor a chance to veto this arc.
    TransitionWrapper trans = 
      TransitionWrapper::init_from(_initial_render_state);
    trans.extract_from(arc);
    
    AttributeWrapper post_state(render_state);
    post_state.apply_in_place(trans);
    if (_visitor.forward_arc(arc, trans, render_state, post_state, 
			     level_state)) {
      
      traverse(arc->get_child(), post_state, level_state, 
	       local_frustum, all_in);
      _visitor.backward_arc(arc, trans, render_state, post_state,
			    level_state);
    }
  }

  if (pushed_arc_stack) {
    _arc_stack.pop_back();
  }
}



////////////////////////////////////////////////////////////////////
//     Function: FrustumCullTraverser::Traverse
//       Access: Public
//  Description: Walks to the next node of the graph.
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
void FrustumCullTraverser<Visitor, LevelState>::
traverse(Node *node, AttributeWrapper &render_state, LevelState &level_state,
	 GeometricBoundingVolume *local_frustum, bool all_in) {
  // Tell the visitor we reached the node, and give it a chance to
  // veto our further progress.
  if (_visitor.reached_node(node, render_state, level_state)) {

    // Look for further children of the given NodeRelation.
    DownRelations::const_iterator dri;
    dri = node->_children.find(_graph_type);
    if (dri != node->_children.end()) {
      // Here are some!
      const DownRelationPointers &drp = (*dri).second;

      // Now visit each of the children in turn.
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
	traverse(*drpi, render_state, level_state, local_frustum, all_in);
      }
    }
  }
}

