// Filename: cullTraverser.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CULLTRAVERSER_H
#define CULLTRAVERSER_H

#include <pandabase.h>

#include "cullLevelState.h"
#include "geomBin.h"

#include <pt_Node.h>
#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nullTransitionWrapper.h>
#include <nullAttributeWrapper.h>
#include <pStatCollector.h>

#include <list>
#include <set>

class GraphicsStateGuardian;
class CullStateLookup;
class CullState;
class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
// 	 Class : CullTraverser
// Description : A special kind of RenderTraverser that makes a
//               complete "cull" pass over the scene graph, grouping
//               the geometry by state and adding it to individual
//               GeomBins, and then renders the GeomBins in sequence.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullTraverser : 
  public RenderTraverser, 
  public TraverserVisitor<NullTransitionWrapper, CullLevelState> {
public:
  CullTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type);
  virtual ~CullTraverser();

  INLINE void set_default_bin(GeomBin *bin);
  INLINE GeomBin *get_default_bin() const;

  virtual void traverse(Node *root, 
			const AllAttributesWrapper &initial_state,
			const AllTransitionsWrapper &net_trans);

  INLINE void draw_geom(GeomNode *geom_node,
			const AllAttributesWrapper &initial_state);
  INLINE void draw_direct(Node *node,
			  const AllAttributesWrapper &initial_state);

PUBLISHED:
  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  void draw();
  void clean_out_old_states();

  void add_geom_node(const PT(GeomNode) &node, 
		     const AllTransitionsWrapper &trans,
		     const CullLevelState &level_state);
  void add_direct_node(const PT_Node &node, 
		       const AllTransitionsWrapper &trans,
		       const CullLevelState &level_state);
		     
  INLINE CullStateLookup *add_instance(NodeRelation *arc, 
				       const AllTransitionsWrapper &trans,
				       Node *top_subtree,
				       const CullLevelState &level_state);

  INLINE CullState *find_bin_state(const AllTransitionsWrapper &trans);


public:  
  // These methods, from parent class TraverserVisitor, define the
  // behavior of the RenderTraverser as it traverses the graph.
  // Normally you would never call these directly.
  bool forward_arc(NodeRelation *arc, NullTransitionWrapper &trans,
		   NullAttributeWrapper &pre, NullAttributeWrapper &post,
		   CullLevelState &level_state);

private:
  UpdateSeq _now;
  AllAttributesWrapper _initial_state;

  typedef set<PT(GeomBin)> Bins;
  Bins _bins;
  PT(GeomBin) _default_bin;

  typedef set<PT(CullState), IndirectCompareTo<CullState> > States;
  States _states;

  CullStateLookup _lookup;
  int _nested_count;

public:
  // Statistics
  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderTraverser::init_type();
    register_type(_type_handle, "CullTraverser",
                  RenderTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;

friend class GeomBin;
};

#include "cullTraverser.I"

#endif

