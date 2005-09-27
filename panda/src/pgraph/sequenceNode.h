// Filename: sequenceNode.h
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

#ifndef SEQUENCENODE_H
#define SEQUENCENODE_H

#include "pandabase.h"

#include "selectiveChildNode.h"
#include "animInterface.h"
#include "clockObject.h"

////////////////////////////////////////////////////////////////////
//       Class : SequenceNode
// Description : A node that automatically cycles through rendering
//               each one of its children according to its frame rate.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SequenceNode : public SelectiveChildNode, public AnimInterface {
PUBLISHED:
  INLINE SequenceNode(const string &name);

protected:
  SequenceNode(const SequenceNode &copy);

PUBLISHED:
  virtual int get_num_frames() const;
  INLINE void set_frame_rate(double frame_rate);

public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;

  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool has_single_child_visibility() const;

  virtual void output(ostream &out) const;

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
    SelectiveChildNode::init_type();
    AnimInterface::init_type();
    register_type(_type_handle, "SequenceNode",
                  SelectiveChildNode::get_class_type(),
		  AnimInterface::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sequenceNode.I"

#endif
