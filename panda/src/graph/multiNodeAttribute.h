// Filename: multiNodeAttribute.h
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

#ifndef MULTINODEATTRIBUTE_H
#define MULTINODEATTRIBUTE_H

#include <pandabase.h>

#include "multiAttribute.h"
#include "pointerNameClass.h"
#include "node.h"
#include "pt_Node.h"
#include "vector_PT_Node.h"

#include <pointerTo.h>

class MultiNodeTransition;

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define MULTIATTRIBUTE_NODE MultiAttribute<PT_Node, PointerNameClass>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, MULTIATTRIBUTE_NODE);

////////////////////////////////////////////////////////////////////
//       Class : MultiNodeAttribute
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MultiNodeAttribute :
  public MultiAttribute<PT_Node, PointerNameClass> {
protected:
  INLINE_GRAPH MultiNodeAttribute() {};
  INLINE_GRAPH MultiNodeAttribute(const MultiNodeAttribute &copy) :
          MultiAttribute<PT_Node, PointerNameClass>(copy) {};

  INLINE_GRAPH void operator = (const MultiNodeAttribute &copy)
          {MultiAttribute<PT_Node, PointerNameClass>::operator = (copy);}

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    MultiAttribute<PT_Node, PointerNameClass>::init_type();
    register_type(_type_handle, "MultiNodeAttribute",
                  MultiAttribute<PT_Node, PointerNameClass>::
                  get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
