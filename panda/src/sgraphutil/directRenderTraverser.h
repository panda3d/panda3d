// Filename: directRenderTraverser.h
// Created by:  drose (18Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef DIRECTRENDERTRAVERSER_H
#define DIRECTRENDERTRAVERSER_H

#include <pandabase.h>

#include "directRenderLevelState.h"

#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nodeRelation.h>
#include <allTransitionsWrapper.h>
#include <allAttributesWrapper.h>
#include <geometricBoundingVolume.h>
#include <lmatrix.h>
#include <pointerTo.h>

#ifdef DO_PSTATS
#include <pStatCollector.h>
#endif

class Node;
class GraphicsStateGuardian;
class GeometricBoundingVolume;
class NodeAttributes;

////////////////////////////////////////////////////////////////////
//       Class : DirectRenderTraverser
// Description : A kind of RenderTraverser that renders each GeomNode
//               it encounters immediately as it is encountered.  No
//               attempt is made to perform state-sorting or binning;
//               however, view-frustum culling is performed.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectRenderTraverser :
  public RenderTraverser, 
  public TraverserVisitor<AllTransitionsWrapper, DirectRenderLevelState> {
public:
  DirectRenderTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
			const ArcChain &arc_chain = ArcChain());
  virtual ~DirectRenderTraverser();

  virtual void traverse(Node *root, 
			const AllAttributesWrapper &initial_state,
			const AllTransitionsWrapper &net_trans);

public:  
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the DirectRenderTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool reached_node(Node *node, AllAttributesWrapper &render_state,
		    DirectRenderLevelState &level_state);
  
  bool forward_arc(NodeRelation *arc, AllTransitionsWrapper &trans,
		   AllAttributesWrapper &pre, AllAttributesWrapper &post,
		   DirectRenderLevelState &level_state);

  void backward_arc(NodeRelation *arc, AllTransitionsWrapper &trans,
		    AllAttributesWrapper &pre, AllAttributesWrapper &post,
		    const DirectRenderLevelState &level_state);

private:
  #ifdef DO_PSTATS
    // Statistics
    static PStatCollector _draw_pcollector;
  #endif

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderTraverser::init_type();
    register_type(_type_handle, "DirectRenderTraverser",
                  RenderTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#include "directRenderTraverser.I"

#endif

