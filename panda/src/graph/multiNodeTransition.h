// Filename: multiNodeTransition.h
// Created by:  drose (23Mar00)
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

#ifndef MULTINODETRANSITION_H
#define MULTINODETRANSITION_H

#include <pandabase.h>

#include "multiTransition.h"
#include "pointerNameClass.h"
#include "node.h"
#include "pt_Node.h"

#include <pointerTo.h>

class NodeRelation;

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTITRANSITION_NODE MultiTransition<PT_Node, PointerNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTITRANSITION_NODE);

////////////////////////////////////////////////////////////////////
//       Class : MultiNodeTransition
// Description : This is a particular instantiation of MultiTransition
//               on PT_Node.  It is its own class in an attempt to
//               cut down on code bloat; if a particular transition
//               wants to be a MultiTransition on some kind of
//               Node, it should inherit from MultiNodeTransition
//               (rather than instantiating a whole new kind of
//               MultiTransition).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MultiNodeTransition :
  public MultiTransition<PT_Node, PointerNameClass> {
protected:
  INLINE_GRAPH MultiNodeTransition() {};
  INLINE_GRAPH MultiNodeTransition(const MultiNodeTransition &copy) :
          MultiTransition<PT_Node, PointerNameClass>(copy) {};

  INLINE_GRAPH void operator = (const MultiNodeTransition &copy)
          {MultiTransition<PT_Node, PointerNameClass>::operator = (copy);};

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiTransition<PT_Node, PointerNameClass>::init_type();
    register_type(_type_handle, "MultiNodeTransition",
                  MultiTransition<PT_Node, PointerNameClass>::
                  get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
