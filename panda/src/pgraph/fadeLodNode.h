// Filename: fadeLodNode.h
// Created by:  sshodhan (14Jun04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef FADELODNODE_H
#define FADELODNODE_H

#include "pandabase.h"

#include "lodNode.h"

////////////////////////////////////////////////////////////////////
//       Class : FadeLODNode
// Description : A Level-of-Detail node with alpha based switching.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH FadeLODNode : public LODNode {
PUBLISHED:
  FadeLODNode(const string &name);

protected:
  INLINE FadeLODNode(const FadeLODNode &copy);
public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void output(ostream &out) const;

PUBLISHED:
  INLINE void set_fade_time(float t);
  INLINE float get_fade_time() const;

private:
  static CPT(RenderState) get_fade_out_state();

private:
  float _fade_time;
  
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
    LODNode::init_type();
    register_type(_type_handle, "FadeLODNode",
                  LODNode::get_class_type());
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
