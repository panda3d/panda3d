// Filename: nodePathComponent.h
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

#ifndef NODEPATHCOMPONENT_H
#define NODEPATHCOMPONENT_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : NodePathComponent
// Description : This is one component of a NodePath.  These are
//               stored on each PandaNode, as many as one for each of
//               the possible instances of the node (but they only
//               exist when they are requested, to minimize memory
//               waste).  A NodePath represents a singly-linked list
//               of these from an arbitrary component in the graph to
//               the root.
//
//               This whole NodePath system is used to disambiguate
//               instances in the scene graph, and the
//               NodePathComponents are stored in the nodes themselves
//               to allow the nodes to keep these up to date as the
//               scene graph is manipulated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePathComponent : public ReferenceCount {
private:
  INLINE NodePathComponent(PandaNode *node, NodePathComponent *next = NULL);
  INLINE NodePathComponent(const NodePathComponent &copy);
  INLINE void operator = (const NodePathComponent &copy);

public:
  INLINE ~NodePathComponent();
  
  INLINE PandaNode *get_node() const;
  int get_key() const;
  bool is_top_node() const;
  
  NodePathComponent *get_next() const;
  int get_length() const;

  bool fix_length();

  void output(ostream &out) const;
  
private:
  void set_next(NodePathComponent *next);
  void set_top_node();

  // We don't have to cycle the _node and _key elements, since these
  // are permanent properties of this object.  (Well, the _key is
  // semi-permanent: it becomes permanent after it has been set the
  // first time.)
  PT(PandaNode) _node;
  int _key;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;

    PT(NodePathComponent) _next;
    int _length;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  static int _next_key;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "NodePathComponent",
                  ReferenceCount::get_class_type());
  }
  
private:
  static TypeHandle _type_handle;
  friend class PandaNode;
};

INLINE ostream &operator << (ostream &out, const NodePathComponent &comp);

#include "nodePathComponent.I"

#endif
