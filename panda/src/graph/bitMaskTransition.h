// Filename: bitMaskTransition.h
// Created by:  drose (08Jun00)
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

#ifndef BITMASKTRANSITION_H
#define BITMASKTRANSITION_H

#include <pandabase.h>

#include "nodeTransition.h"

class NodeRelation;

////////////////////////////////////////////////////////////////////
//       Class : BitMaskTransition
// Description : This is an abstract template class that encapsulates
//               transitions on a bitmask.  Each transition may add
//               and/or remove bits from the accumulated bitmask.
//
//               It's the base class for a number of scene graph
//               transitions like DrawMaskTransition and
//               CollideMaskTransition.
////////////////////////////////////////////////////////////////////
template<class MaskType>
class BitMaskTransition : public NodeTransition {
protected:
  INLINE_GRAPH BitMaskTransition();
  INLINE_GRAPH BitMaskTransition(const MaskType &and, const MaskType &or);
  INLINE_GRAPH BitMaskTransition(const BitMaskTransition &copy);
  INLINE_GRAPH void operator = (const BitMaskTransition &copy);

public:
  INLINE_GRAPH void set_and(const MaskType &and);
  INLINE_GRAPH const MaskType &get_and() const;
  INLINE_GRAPH void set_or(const MaskType &or);
  INLINE_GRAPH const MaskType &get_or() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;

  virtual void output(ostream &out) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;
  virtual void internal_generate_hash(GraphHashGenerator &hashgen) const;

protected:
  virtual BitMaskTransition<MaskType> *
  make_with_masks(const MaskType &and, const MaskType &or) const=0;

private:
  MaskType _and, _or;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    MaskType::init_type();
    register_type(_type_handle,
                  string("BitMaskTransition<") +
                  MaskType::get_class_type().get_name() + ">",
                  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "bitMaskTransition.T"

#endif
