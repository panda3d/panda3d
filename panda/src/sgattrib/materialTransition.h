// Filename: materialTransition.h
// Created by:  mike (19Jan99)
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

#ifndef MATERIALTRANSITION_H
#define MATERIALTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>
#include <material.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
//       Class : MaterialTransition
// Description : Applies a material, controlling subtle lighting
//               parameters, to geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MaterialTransition : public OnOffTransition {
public:
  INLINE MaterialTransition();
  INLINE MaterialTransition(const Material *material);
  INLINE static MaterialTransition off();

  INLINE void set_on(const Material *material);
  INLINE const Material *get_material() const;

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeTransition *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  CPT(Material) _value;
  static PT(NodeTransition) _initial;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_MaterialTransition(const FactoryParams &params);
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
    OnOffTransition::init_type();
    register_type(_type_handle, "MaterialTransition",
                  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialTransition.I"

#endif
