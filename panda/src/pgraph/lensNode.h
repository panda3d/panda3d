// Filename: lensNode.h
// Created by:  drose (26Feb02)
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

#ifndef LENSNODE_H
#define LENSNODE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "lens.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : LensNode
// Description : A node that contains a Lens.  The most important
//               example of this kind of node is a Camera, but other
//               kinds of nodes also contain a lens (for instance, a
//               Spotlight).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LensNode : public PandaNode {
PUBLISHED:
  LensNode(const string &name);

protected:
  LensNode(const LensNode &copy);
public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *make_copy() const;

PUBLISHED:
  INLINE void copy_lens(const Lens &lens);
  INLINE void set_lens(Lens *lens);
  INLINE Lens *get_lens() const;

  bool is_in_view(const LPoint3f &pos);

protected:
  PT(Lens) _lens;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "LensNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lensNode.I"

#endif
