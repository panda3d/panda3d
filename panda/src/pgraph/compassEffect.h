// Filename: compassEffect.h
// Created by:  drose (16Jul02)
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

#ifndef COMPASSEFFECT_H
#define COMPASSEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : CompassEffect
// Description : Indicates that geometry at this node should
//               automatically rotate to face the camera, or any other
//               arbitrary node.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CompassEffect : public RenderEffect {
private:
  INLINE CompassEffect();

PUBLISHED:
  static CPT(RenderEffect) make(const NodePath &reference);

public:
  virtual bool safe_to_transform() const;
  virtual void output(ostream &out) const;

  CPT(TransformState) CompassEffect::
  do_compass(const TransformState *net_transform,
             const TransformState *node_transform) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;
  virtual RenderEffect *make_default_impl() const;

private:
  NodePath _reference;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "CompassEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "compassEffect.I"

#endif

