/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileArrayDef.h
 * @author drose
 * @date 2004-10-03
 */

#ifndef XFILEARRAYDEF_H
#define XFILEARRAYDEF_H

#include "pandatoolbase.h"
#include "pnotify.h"
#include "xFileNode.h"

class XFileDataDef;

/**
 * Defines one level of array bounds for an associated XFileDataDef element.
 */
class XFileArrayDef {
public:
  INLINE XFileArrayDef(int fixed_size);
  INLINE XFileArrayDef(XFileDataDef *dynamic_size);

  INLINE bool is_fixed_size() const;
  INLINE int get_fixed_size() const;
  INLINE XFileDataDef *get_dynamic_size() const;

  int get_size(const XFileNode::PrevData &prev_data) const;

  void output(std::ostream &out) const;

  bool matches(const XFileArrayDef &other, const XFileDataDef *parent,
               const XFileDataDef *other_parent) const;

private:
  int _fixed_size;
  XFileDataDef *_dynamic_size;
};

#include "xFileArrayDef.I"

#endif
