// Filename: planeNode.h
// Created by:  drose (11Jul02)
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

#ifndef PLANENODE_H
#define PLANENODE_H

#include "pandabase.h"

#include "plane.h"
#include "pandaNode.h"
#include "updateSeq.h"
#include "geom.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "cycleDataStageReader.h"
#include "cycleDataStageWriter.h"
#include "pipelineCycler.h"

////////////////////////////////////////////////////////////////////
//       Class : PlaneNode
// Description : A node that contains a plane.  This is most often
//               used as a clipping plane, but it can serve other
//               purposes as well; whenever a plane is needed to be
//               defined in some coordinate space in the world.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PlaneNode : public PandaNode {
PUBLISHED:
  PlaneNode(const string &name, const Planef &plane = Planef());

protected:
  PlaneNode(const PlaneNode &copy);
public:
  virtual void output(ostream &out) const;

  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4f &mat);

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

PUBLISHED:
  INLINE void set_plane(const Planef &plane);
  INLINE const Planef &get_plane() const;

  INLINE void set_viz_scale(float viz_scale);
  INLINE float get_viz_scale() const;

  INLINE void set_priority(int priority);
  INLINE int get_priority() const;

public:
  INLINE static UpdateSeq get_sort_seq();

protected:
  virtual PT(BoundingVolume) compute_internal_bounds(int pipeline_stage, Thread *current_thread) const;
  PT(Geom) get_viz(CullTraverser *trav, CullTraverserData &data);
  
private:
  // The priority is not cycled, because there's no real reason to do
  // so, and cycling it makes it difficult to synchronize with the
  // ClipPlaneAttribs.
  int _priority;
  static UpdateSeq _sort_seq;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return PlaneNode::get_class_type();
    }

    Planef _plane;
    PT(Geom) _front_viz, _back_viz;
    float _viz_scale;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

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
    register_type(_type_handle, "PlaneNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "planeNode.I"

#endif
