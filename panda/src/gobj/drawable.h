// Filename: drawable.h
// Created by:  mike (09Jan97)
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

#ifndef DDRAWABLE_H
#define DDRAWABLE_H

#include "pandabase.h"

#include "boundedObject.h"
#include "writableConfigurable.h"
#include "referenceCount.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

class GraphicsStateGuardianBase;
class Datagram;
class DatagramIterator;
class BamReader;
class BamWriter;
class qpGeomVertexData;

////////////////////////////////////////////////////////////////////
//       Class : Drawable
// Description : Object that can be drawn (i.e. issues graphics
//               commands).
//               NOTE: We had to change the name to dDrawable because
//               the stupid bastards who wrote X didn't add a prefix
//               to their variable names
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA dDrawable : public ReferenceCount, public WritableConfigurable,
                              public BoundedObject {
public:

  dDrawable();
  virtual ~dDrawable();

  virtual void draw(GraphicsStateGuardianBase *gsg, 
                    const qpGeomVertexData *vertex_data) const;
  virtual bool is_dynamic() const;

protected:
  virtual void propagate_stale_bound();

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }

public:
  static void init_type() {
    ReferenceCount::init_type();
    WritableConfigurable::init_type();
    BoundedObject::init_type();
    register_type(_type_handle, "dDrawable",
                  ReferenceCount::get_class_type(),
                  WritableConfigurable::get_class_type(),
                  BoundedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

public:
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}


private:
  static TypeHandle _type_handle;
};

#endif

