// Filename: jointVertexTransform.h
// Created by:  drose (24Mar05)
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

#ifndef JOINTVERTEXTRANSFORM_H
#define JOINTVERTEXTRANSFORM_H

#include "pandabase.h"
#include "characterJoint.h"
#include "vertexTransform.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : JointVertexTransform
// Description : This is a specialization on VertexTransform that
//               returns the relative transform from one joint's
//               initial position to another joint's (or possibly the
//               same joint's) current position.  It is used to
//               implement soft-skinned vertices for an animated
//               character.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA JointVertexTransform : public VertexTransform {
private:
  JointVertexTransform();

PUBLISHED:
  JointVertexTransform(CharacterJoint *from, CharacterJoint *to);
  virtual ~JointVertexTransform();

  INLINE const CharacterJoint *get_from() const;
  INLINE const CharacterJoint *get_to() const;

  virtual void get_matrix(LMatrix4f &matrix) const;

  virtual void output(ostream &out) const;

private:
  PT(CharacterJoint) _from;
  PT(CharacterJoint) _to;

  LMatrix4f _matrix;
  bool _matrix_stale;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    VertexTransform::init_type();
    register_type(_type_handle, "JointVertexTransform",
                  VertexTransform::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CharacterJoint;
};

#include "jointVertexTransform.I"

#endif
