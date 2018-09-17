/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file jointVertexTransform.h
 * @author drose
 * @date 2005-03-24
 */

#ifndef JOINTVERTEXTRANSFORM_H
#define JOINTVERTEXTRANSFORM_H

#include "pandabase.h"
#include "characterJoint.h"
#include "vertexTransform.h"
#include "pointerTo.h"
#include "lightMutex.h"

/**
 * This is a specialization on VertexTransform that returns the transform
 * necessary to move vertices as if they were assigned to the indicated joint.
 * The geometry itself should be parented to the scene graph at the level of
 * the character's root joint; that is, it should not be parented under a node
 * directly animated by any joints.
 *
 * Multiple combinations of these with different weights are used to implement
 * soft-skinned vertices for an animated character.
 */
class EXPCL_PANDA_CHAR JointVertexTransform : public VertexTransform {
private:
  JointVertexTransform();

PUBLISHED:
  JointVertexTransform(CharacterJoint *joint);
  virtual ~JointVertexTransform();

  INLINE const CharacterJoint *get_joint() const;

  virtual void get_matrix(LMatrix4 &matrix) const;
  virtual void mult_matrix(LMatrix4 &result, const LMatrix4 &previous) const;
  virtual void accumulate_matrix(LMatrix4 &accum, PN_stdfloat weight) const;

  virtual void output(std::ostream &out) const;

private:
  PT(CharacterJoint) _joint;

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
