// Filename: bitMaskAttribute.h
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

#ifndef BITMASKATTRIBUTE_H
#define BITMASKATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

template<class MaskType>
class BitMaskTransition;

////////////////////////////////////////////////////////////////////
//       Class : BitMaskAttribute
// Description :
////////////////////////////////////////////////////////////////////
template<class MaskType>
class BitMaskAttribute : public NodeAttribute {
protected:
  INLINE_GRAPH BitMaskAttribute();
  INLINE_GRAPH BitMaskAttribute(const MaskType &mask);
  INLINE_GRAPH BitMaskAttribute(const BitMaskAttribute &copy);
  INLINE_GRAPH void operator = (const BitMaskAttribute &copy);

public:
  INLINE_GRAPH void set_mask(const MaskType &mask);
  INLINE_GRAPH const MaskType &get_mask() const;

  virtual void output(ostream &out) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

private:
  MaskType _mask;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    MaskType::init_type();
    register_type(_type_handle,
                  string("BitMaskAttribute<") +
                  MaskType::get_class_type().get_name() + ">",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class BitMaskTransition<MaskType>;
};

#include "bitMaskAttribute.T"

#endif
