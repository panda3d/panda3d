// Filename: renderRelation.h
// Created by:  drose (26Oct98)
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

#ifndef RENDERRELATION_H
#define RENDERRELATION_H

#include <pandabase.h>

#include <nodeRelation.h>
#include <luse.h>

///////////////////////////////////////////////////////////////////
//       Class : RenderRelation
// Description : The arc type specific to renderable scene graphs.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderRelation : public NodeRelation {
public:
  INLINE RenderRelation(Node *from, Node *to, int sort = 0);

protected:
  // Normally, this should only be used for passing to the factory.
  // Don't attempt to create an unattached arc directly.
  INLINE RenderRelation();

public:
  virtual void changed_transition(TypeHandle transition_type);

protected:
  virtual BoundingVolume *recompute_bound();

public:
  // This is just to be called at initialization time; don't try to
  // call this directly.
  INLINE static void register_with_factory();

private:
  static NodeRelation *make_arc(const FactoryParams &params);

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_RenderRelation(const FactoryParams &params);

PUBLISHED:
  INLINE static TypeHandle get_class_type();

public:
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type();

private:
  static TypeHandle _type_handle;
};

#include "renderRelation.I"

#endif
