// Filename: nodeChainComponent.h
// Created by:  drose (25Feb02)
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

#ifndef NODECHAINCOMPONENT_H
#define NODECHAINCOMPONENT_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : NodeChainComponent
// Description : This is one component of a NodeChain.  These are
//               stored on each PandaNode, as many as one for each of
//               the possible instances of the node (but they only
//               exist when they are requested, to minimize memory
//               waste).  A NodeChain represents a singly-linked list
//               of these from an arbitrary component in the graph to
//               the root.
//
//               This whole NodeChain system is used to disambiguate
//               instances in the scene graph, and the
//               NodeChainComponents are stored in the nodes
//               themselves to allow the nodes to keep these up to
//               date as the scene graph is manipulated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodeChainComponent : public ReferenceCount {
private:
  INLINE NodeChainComponent(PandaNode *node, NodeChainComponent *next = NULL);
  INLINE NodeChainComponent(const NodeChainComponent &copy);
  INLINE void operator = (const NodeChainComponent &copy);

public:
  INLINE ~NodeChainComponent();
  
  INLINE PandaNode *get_node() const;
  INLINE bool is_top_node() const;
  INLINE bool is_collapsed() const;
  
  NodeChainComponent *get_next() const;
  INLINE int get_length() const;
  INLINE NodeChainComponent *get_collapsed() const;

  bool fix_length();
  NodeChainComponent *uncollapse();
  
private:
  INLINE void set_next(NodeChainComponent *next);
  INLINE void set_top_node();
  INLINE void collapse_with(NodeChainComponent *next);

  PT(PandaNode) _node;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;

    PT(NodeChainComponent) _next;
    int _length;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "NodeChainComponent",
                  ReferenceCount::get_class_type());
  }
  
private:
  static TypeHandle _type_handle;
  friend class PandaNode;
};

#include "nodeChainComponent.I"

#endif
