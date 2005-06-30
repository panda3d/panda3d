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
//               returns the transform necessary to move vertices as
//               if they were assigned to the indicated joint.  The
//               geometry itself should be parented to the scene graph
//               at the level of the character's root joint; that is,
//               it should not be parented under a node directly
//               animated by any joints.
//
//               Multiple combinations of these with different weights
//               are used to implement soft-skinned vertices for an
//               animated character.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA JointVertexTransform : public VertexTransform {
private:
  JointVertexTransform();

PUBLISHED:
  JointVertexTransform(CharacterJoint *joint);
  virtual ~JointVertexTransform();

  INLINE const CharacterJoint *get_joint() const;

  virtual void get_matrix(LMatrix4f &matrix) const;
  virtual void mult_matrix(LMatrix4f &result, const LMatrix4f &previous) const;
  virtual void accumulate_matrix(LMatrix4f &accum, float weight) const;

  virtual void output(ostream &out) const;

private:
  INLINE void check_matrix() const;

  PT(CharacterJoint) _joint;

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
