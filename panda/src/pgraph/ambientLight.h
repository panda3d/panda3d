// Filename: ambientLight.h
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

#ifndef AMBIENTLIGHT_H
#define AMBIENTLIGHT_H

#include "pandabase.h"

#include "lightNode.h"

////////////////////////////////////////////////////////////////////
//       Class : AmbientLight
// Description : A light source that seems to illuminate all points in
//               space at once.  This kind of light need not actually
//               be part of the scene graph, since it has no meaningful
//               position.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AmbientLight : public LightNode {
PUBLISHED:
  AmbientLight(const string &name);

protected:
  AmbientLight(const AmbientLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void write(ostream &out, int indent_level) const;
  
public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

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
    LightNode::init_type();
    register_type(_type_handle, "AmbientLight",
                  LightNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const AmbientLight &light) {
  light.output(out);
  return out;
}

#include "ambientLight.I"

#endif
