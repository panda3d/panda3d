// Filename: frustumCullTraverser.I
// Created by:  drose (14Apr00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: FrustumCullTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class Visitor, class LevelState>
FrustumCullTraverser<Visitor, LevelState>::
FrustumCullTraverser(ArcChain &arc_chain, Node *root, 
		     const LMatrix4f &rel_from_camera, Visitor &visitor,
		     const AttributeWrapper &initial_render_state,
		     const LevelState &initial_level_state,
		     GraphicsStateGuardian *gsg, 
		     TypeHandle graph_type) :
  _arc_chain(arc_chain),
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

    ProjectionNode *camera = _gsg->get_current_projection_node();
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

    local_frustum = DCAST(GeometricBoundingVolume, _view_frustum->make_copy());
    local_frustum->xform(rel_from_camera);

#ifndef NDEBUG
    if (sgraphutil_cat.is_spam()) {
      sgraphutil_cat.spam()
	<< "Beginning frustum cull, frustum is: " << *local_frustum << "\n"
	<< "Transform is:\n";
      rel_from_camera.write(sgraphutil_cat.spam(false), 2);
    }
#endif
  }

  bool needs_top_node = !_arc_chain.has_node();
  if (needs_top_node) {
    // If the ArcChain supplied in is initially empty, put the root
    // node in it.
    _arc_chain = ArcChain(root);
  }

  LevelState level_state(initial_level_state);
  traverse(root, _initial_render_state, level_state, local_frustum, false);

  _view_frustum = NULL;

  if (needs_top_node) {
    // Restore the ArcChain to its initial empty state (if that's what
    // it was).
    _arc_chain = ArcChain();
  }
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

  _arc_chain.push_back(arc);

  // First, if we're performing view-frustum culling, check the
  // bounding volume associated with this arc (which bounds all of its
  // children) against the bounding volume of our view frustum.

  if (_view_frustum != (GeometricBoundingVolume *)NULL) {
    // If this arc's child has multiple parents, we need to save the
    // arc on the arc stack, so we can unambiguously wrt().

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
#ifndef NDEBUG	
	  if (sgraphutil_cat.is_spam() && !arc_volume.is_empty()) {
	    sgraphutil_cat.spam()
	      << "Culling " << *arc->get_child() << " with volume "
	      << arc_volume << "\n";
	  }
#endif

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

      if (carry_on) {
	if (arc->has_transition(BillboardTransition::get_class_type())) {
	  // We can't reliably cull within a billboard, because the
	  // geometry might get rotated out of its bounding volume.
	  // So once we get within a billboard, we consider it all
	  // visible.
	  all_in = true;
	}

	if (!all_in && 
	    arc->has_transition(TransformTransition::get_class_type())) {
	  // Now apply the transform associated with this arc.  We do
	  // this after the bounding volume test, because the bounding
	  // volume has already been transformed.

	  local_frustum =
	    DCAST(GeometricBoundingVolume, _view_frustum->make_copy());

	  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
	  wrt(_gsg->get_current_projection_node(),
	      arc->get_child(), _arc_chain.begin(), _arc_chain.end(),
	      ntw, _graph_type);
	  
	  const TransformTransition *tt;
	  if (get_transition_into(tt, ntw)) {
	    // This frustum is transformed from the camera.  Most will
	    // be.
	    nassertv(local_frustum != (GeometricBoundingVolume *)NULL);
	    local_frustum->xform(tt->get_matrix());

#ifndef NDEBUG	    
	    if (sgraphutil_cat.is_spam()) {
	      sgraphutil_cat.spam()
		<< "Transforming frustum into local space of " << *arc << ": " << *local_frustum << "\n"
		<< "Transform is:\n";
	      tt->get_matrix().write(sgraphutil_cat.spam(false), 2);
	    }
#endif
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

  _arc_chain.pop_back();
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
    const DownRelationPointers &drp = 
      node->find_connection(_graph_type).get_down();

    // Now visit each of the children in turn.
    if (node->is_of_type(SwitchNode::get_class_type())) {
      SwitchNode *swnode = DCAST(SwitchNode, node);
      // Presumably, the visitor called compute_switch() on the
      // SwitchNode during the reached_node() call, above.  We can't
      // call it directly since we don't have enough information.
      size_t i = 0;
      for (i = 0; i < drp.size(); i++) {
	if (swnode->is_child_visible(_graph_type, i)) {
	  traverse(drp[i], render_state, level_state, local_frustum, all_in);
	}
      }
    } else {
      DownRelationPointers::const_iterator drpi;
      for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
	traverse(*drpi, render_state, level_state, local_frustum, all_in);
      }
    }
  }
}

