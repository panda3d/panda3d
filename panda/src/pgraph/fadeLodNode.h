// Filename: fadeLodNode.h
// Created by:  sshodhan (14Jun04)
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

#ifndef FADELODNODE_H
#define FADELODNODE_H

#include "pandabase.h"

#include "pandaNode.h"

#include "LOD.h"

////////////////////////////////////////////////////////////////////
//       Class : FadeLODNode
// Description : A Level-of-Detail node with alpha based switching.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FadeLODNode : public PandaNode {
PUBLISHED:
  INLINE FadeLODNode(const string &name);


protected:
  INLINE FadeLODNode(const FadeLODNode &copy);
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

  INLINE void set_fade_time(float t);
  INLINE float get_fade_time() const;

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
    float _fade_time;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  bool _fade_mode;
  float _fade_timer;
  int _fade_out;
  int _fade_in;
  int _previous_child;
  


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "FadeLODNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "fadeLodNode.I"

#endif
