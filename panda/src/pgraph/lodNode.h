// Filename: lodNode.h
// Created by:  drose (06Mar02)
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

#ifndef LODNODE_H
#define LODNODE_H

#include "pandabase.h"

#include "selectiveChildNode.h"

#include "LOD.h"

////////////////////////////////////////////////////////////////////
//       Class : LODNode
// Description : A Level-of-Detail node.  This selects only one of its
//               children for rendering, according to the distance
//               from the camera and the table indicated in the
//               associated LOD object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LODNode : public SelectiveChildNode {
PUBLISHED:
  INLINE LODNode(const string &name);

protected:
  INLINE LODNode(const LODNode &copy);
public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;
  virtual void xform(const LMatrix4f &mat);
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void output(ostream &out) const;

PUBLISHED:
  // The sense of in vs. out distances is as if the object were coming
  // towards you from far away: it switches "in" at the far distance,
  // and switches "out" at the close distance.  Thus, "in" should be
  // larger than "out".

  INLINE void add_switch(float in, float out);
  INLINE bool set_switch(int index, float in, float out);
  INLINE void clear_switches(void);

  INLINE int get_num_switches() const;
  INLINE float get_in(int index) const;
  INLINE float get_out(int index) const;

  INLINE void set_center(const LPoint3f &center);
  INLINE const LPoint3f &get_center() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

private:
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    LOD _lod;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    SelectiveChildNode::init_type();
    register_type(_type_handle, "LODNode",
                  SelectiveChildNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lodNode.I"

#endif
