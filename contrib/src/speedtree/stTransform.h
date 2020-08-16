/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stTransform.h
 * @author drose
 * @date 2010-10-06
 */

#ifndef STTRANSFORM_H
#define STTRANSFORM_H

#include "pandabase.h"
#include "transformState.h"
#include "speedtree_api.h"
#include "deg_2_rad.h"

/**
 * Represents a transform that may be applied to a particular instance of a
 * tree when added to the SpeedTreeNode.
 */
class EXPCL_PANDASPEEDTREE STTransform {
PUBLISHED:
  INLINE STTransform();
  STTransform(const TransformState *trans);
  INLINE STTransform(const LPoint3 &pos, PN_stdfloat rotate = 0.0f, PN_stdfloat scale = 1.0f);
  INLINE STTransform(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z, PN_stdfloat rotate, PN_stdfloat scale);
  INLINE STTransform(const STTransform &copy);
  INLINE void operator = (const STTransform &copy);

public:
  INLINE STTransform(const SpeedTree::CInstance &instance);
  INLINE operator SpeedTree::CInstance () const;
  INLINE operator CPT(TransformState) () const;

PUBLISHED:
  INLINE static const STTransform &ident_mat();

  INLINE void set_pos(const LPoint3 &pos);
  INLINE const LPoint3 &get_pos() const;
  INLINE void set_rotate(PN_stdfloat rotate);
  INLINE PN_stdfloat get_rotate() const;
  INLINE void set_scale(PN_stdfloat scale);
  INLINE PN_stdfloat get_scale() const;

  INLINE void operator *= (const STTransform &other);
  INLINE STTransform operator * (const STTransform &other) const;

  void output(std::ostream &out) const;

public:
  void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  LPoint3 _pos;
  PN_stdfloat _rotate;
  PN_stdfloat _scale;

  static STTransform _ident_mat;
};

INLINE std::ostream &operator << (std::ostream &out, const STTransform &transform) {
  transform.output(out);
  return out;
}

#include "stTransform.I"

#endif
