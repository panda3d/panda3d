// Filename: cullTraverser.h
// Created by:  drose (07Apr00)
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

#ifndef CULLTRAVERSER_H
#define CULLTRAVERSER_H

#include <pandabase.h>

#include "cullLevelState.h"
#include "geomBin.h"

#include <pt_Node.h>
#include <renderTraverser.h>
#include <traverserVisitor.h>
#include <nullTransitionWrapper.h>
#include <pStatCollector.h>

#include "plist.h"
#include "pset.h"

class GraphicsStateGuardian;
class CullStateLookup;
class CullState;
class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
//       Class : CullTraverser
// Description : A special kind of RenderTraverser that makes a
//               complete "cull" pass over the scene graph, grouping
//               the geometry by state and adding it to individual
//               GeomBins, and then renders the GeomBins in sequence.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullTraverser :
  public RenderTraverser,
  public TraverserVisitor<NullTransitionWrapper, CullLevelState> {
public:
  CullTraverser(GraphicsStateGuardian *gsg, TypeHandle graph_type,
                const ArcChain &arc_chain = ArcChain());
  virtual ~CullTraverser();

PUBLISHED:
  bool has_bin(const string &name) const;
  GeomBin *get_bin(const string &name) const;
  void clear_bins();

  void clear_state();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:

  virtual void traverse(Node *root,
                        const AllTransitionsWrapper &initial_state);

  INLINE void draw_geom(GeomNode *geom_node,
                        const AllTransitionsWrapper &initial_state);
  INLINE void draw_geom(const ArcChain &arc_chain,
                        const AllTransitionsWrapper &initial_state);
  INLINE void draw_direct(const ArcChain &arc_chain,
                          const AllTransitionsWrapper &initial_state);

private:
  void setup_initial_bins();

  void draw();
  void clean_out_old_states();

  void add_geom_node(GeomNode *node,
                     const AllTransitionsWrapper &trans,
                     const CullLevelState &level_state);
  void add_direct_node(Node *node,
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
                   NullTransitionWrapper &pre, NullTransitionWrapper &post,
                   CullLevelState &level_state);

  INLINE void
  backward_arc(NodeRelation *arc, NullTransitionWrapper &trans,
               NullTransitionWrapper &pre, NullTransitionWrapper &post,
               const CullLevelState &level_state);

private:
  void attach_toplevel_bin(GeomBin *bin);
  void attach_sub_bin(GeomBin *bin);
  void detach_toplevel_bin(GeomBin *bin);
  void detach_sub_bin(GeomBin *bin);

  AllTransitionsWrapper _initial_state;

  typedef pmap<string,  PT(GeomBin) > ToplevelBins;
  typedef pmultimap<int,  PT(GeomBin) > SubBins;
  ToplevelBins _toplevel_bins;
  SubBins _sub_bins;
  PT(GeomBin) _default_bin;

  typedef pset<PT(CullState), IndirectCompareTo<CullState> > States;
  States _states;

  CullStateLookup _lookup;
  int _nested_count;
  UpdateSeq _as_of;
  UpdateSeq _now;

public:
  // Statistics
  static PStatCollector _cull_pcollector;
  static PStatCollector _draw_pcollector;
  static PStatCollector _cull_traverse_pcollector;
  static PStatCollector _cull_geom_node_pcollector;
  static PStatCollector _cull_direct_node_pcollector;
  static PStatCollector _cull_draw_get_bin_pcollector;
  static PStatCollector _cull_draw_pcollector;
  static PStatCollector _cull_clean_pcollector;
  static PStatCollector _cull_bins_unsorted_pcollector;
  static PStatCollector _cull_bins_fixed_pcollector;
  static PStatCollector _cull_bins_btf_pcollector;

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
  friend class GeomBinGroup;
};

#include "cullTraverser.I"

#endif

