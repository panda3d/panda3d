// Filename: dynamicVertices.h
// Created by:  drose (01Mar99)
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

#ifndef DYNAMICVERTICES_H
#define DYNAMICVERTICES_H

#include "pandabase.h"

#include "pointerToArray.h"
#include "typedObject.h"
#include "luse.h"
#include "pta_Vertexf.h"
#include "pta_Normalf.h"
#include "pta_Colorf.h"
#include "pta_TexCoordf.h"
#include "typedWritable.h"

class BamReader;

////////////////////////////////////////////////////////////////////
//       Class : DynamicVertices
// Description : A table of vertices associated with a Character that
//               must be computed dynamically each frame.  This is the
//               actual table of vertices; see ComputedVertices for
//               the code to compute their values.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DynamicVertices : public TypedWritable {
public:
  DynamicVertices();
  DynamicVertices(const DynamicVertices &copy);
  static DynamicVertices deep_copy(const DynamicVertices &copy);

  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_TexCoordf _texcoords;

public:
  virtual void write_datagram(BamWriter* manager, Datagram &me);
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
    TypedWritable::init_type();
    register_type(_type_handle, "DynamicVertices",
                  TypedWritable::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif

