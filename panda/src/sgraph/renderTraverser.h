// Filename: renderTraverser.h
// Created by:  drose (12Apr00)
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

#ifndef RENDERTRAVERSER_H
#define RENDERTRAVERSER_H

#include <pandabase.h>

#include <arcChain.h>
#include <typedObject.h>
#include <typedReferenceCount.h>
#include <notify.h>

class GraphicsStateGuardian;
class Node;
class NodeRelation;
class AllTransitionsWrapper;

////////////////////////////////////////////////////////////////////
//       Class : RenderTraverser
// Description : This is the abstract base class that defines the
//               interface for any number of different kinds of
//               specialized traversers that walk over the scene graph
//               and render it with a given GraphicsStateGuardian,
//               such as DirectRenderTraverser and CullTraverser.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderTraverser : public TypedReferenceCount {
public:
  INLINE RenderTraverser(GraphicsStateGuardian *gsg,
                         TypeHandle graph_type,
                         const ArcChain &arc_chain);
  INLINE ~RenderTraverser();

  typedef ArcChain::iterator iterator;
  typedef ArcChain::const_iterator const_iterator;

  INLINE const ArcChain &get_arc_chain() const;
  INLINE const_iterator begin() const;
  INLINE const_iterator end() const;
  INLINE bool empty() const;

PUBLISHED:
  INLINE GraphicsStateGuardian *get_gsg() const;
  INLINE TypeHandle get_graph_type() const;

public:
  virtual void traverse(Node *root,
                        const AllTransitionsWrapper &initial_state)=0;

PUBLISHED:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  // These methods are to be called by derived classes as we traverse
  // each arc.  They update the arc list returned by begin()/end().
  INLINE void mark_forward_arc(NodeRelation *arc);
  INLINE void mark_backward_arc(NodeRelation *arc);

protected:
  GraphicsStateGuardian *_gsg;
  TypeHandle _graph_type;
  ArcChain _arc_chain;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "RenderTraverser",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const RenderTraverser &rt) {
  rt.output(out);
  return out;
}

#include "renderTraverser.I"

#endif
