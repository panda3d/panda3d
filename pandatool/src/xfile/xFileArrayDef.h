// Filename: xFileArrayDef.h
// Created by:  drose (03Oct04)
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

#ifndef XFILEARRAYDEF_H
#define XFILEARRAYDEF_H

#include "pandatoolbase.h"
#include "notify.h"
#include "xFileNode.h"

class XFileDataDef;

////////////////////////////////////////////////////////////////////
//       Class : XFileArrayDef
// Description : Defines one level of array bounds for an associated
//               XFileDataDef element.
////////////////////////////////////////////////////////////////////
class XFileArrayDef {
public:
  INLINE XFileArrayDef(int fixed_size);
  INLINE XFileArrayDef(XFileDataDef *dynamic_size);

  INLINE bool is_fixed_size() const;
  INLINE int get_fixed_size() const;
  INLINE XFileDataDef *get_dynamic_size() const;

  int get_size(const XFileNode::PrevData &prev_data) const;

  void output(ostream &out) const;

private:
  int _fixed_size;
  XFileDataDef *_dynamic_size;
};

#include "xFileArrayDef.I"

#endif
  


