// Filename: transparencyTransition.h
// Created by:  mike (18Jan99)
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

#ifndef TRANSPARENCYTRANSITION_H
#define TRANSPARENCYTRANSITION_H

#include <pandabase.h>

#include "transparencyProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : TransparencyTransition
// Description : This controls the enabling of transparency.  Simply
//               setting an alpha component to non-1 does not in
//               itself make an object transparent; you must also
//               enable transparency mode with a suitable
//               TransparencyTransition.  Similarly, it is wasteful to
//               render an object with a TransparencyTransition in
//               effect unless you actually want it to be at least
//               partially transparent (and it has alpha components
//               less than 1).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransparencyTransition : public OnTransition {
PUBLISHED:
  INLINE TransparencyTransition(TransparencyProperty::Mode mode = TransparencyProperty::M_none);

  INLINE void set_mode(TransparencyProperty::Mode mode);
  INLINE TransparencyProperty::Mode get_mode() const;

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  TransparencyProperty _value;
  static PT(NodeTransition) _initial;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_TransparencyTransition(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    register_type(_type_handle, "TransparencyTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "transparencyTransition.I"

#endif
