/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sheetNode.h
 * @author drose
 * @date 2003-10-11
 */

#ifndef SHEETNODE_H
#define SHEETNODE_H

#include "pandabase.h"
#include "nurbsSurfaceEvaluator.h"
#include "pandaNode.h"
#include "pStatCollector.h"

/**
 * This class draws a visible representation of the NURBS surface stored in
 * its NurbsSurfaceEvaluator.  It automatically recomputes the surface every
 * frame.
 *
 * This is not related to NurbsSurface, CubicSurfaceseg or any of the
 * ParametricSurface-derived objects in this module.  It is a completely
 * parallel implementation of NURBS surfaces, and will probably eventually
 * replace the whole ParametricSurface class hierarchy.
 */
class EXPCL_PANDA_PARAMETRICS SheetNode : public PandaNode {
PUBLISHED:
  explicit SheetNode(const std::string &name);

protected:
  SheetNode(const SheetNode &copy);
public:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  virtual PandaNode *make_copy() const;

  virtual bool safe_to_transform() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

PUBLISHED:
  INLINE void set_surface(NurbsSurfaceEvaluator *surface);
  INLINE NurbsSurfaceEvaluator *get_surface() const;

  INLINE void set_use_vertex_color(bool flag);
  INLINE bool get_use_vertex_color() const;

  INLINE void set_num_u_subdiv(int num_u_subdiv);
  INLINE int get_num_u_subdiv() const;
  INLINE void set_num_v_subdiv(int num_v_subdiv);
  INLINE int get_num_v_subdiv() const;

  void reset_bound(const NodePath &rel_to);

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

private:
  PT(BoundingVolume) do_recompute_bounds(const NodePath &rel_to,
                                         int pipeline_stage,
                                         Thread *current_thread) const;
  void render_sheet(CullTraverser *trav, CullTraverserData &data,
                    NurbsSurfaceResult *result);

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PARAMETRICS CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return SheetNode::get_class_type();
    }

    PT(NurbsSurfaceEvaluator) _surface;
    bool _use_vertex_color;
    int _num_u_subdiv;
    int _num_v_subdiv;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  static PStatCollector _sheet_node_pcollector;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "SheetNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sheetNode.I"

#endif
