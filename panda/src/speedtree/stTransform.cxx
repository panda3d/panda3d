/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stTransform.cxx
 * @author drose
 * @date 2010-10-06
 */

#include "stTransform.h"

STTransform STTransform::_ident_mat;

/**
 * This constructor accepts a Panda TransformState, for instance as extracted
 * from the scene graph.
 */
STTransform::
STTransform(const TransformState *trans) {
#ifndef NDEBUG
  // Ensure these are initialized to reasonable values in case we fail an
  // assertion below.
  _pos.set(0.0f, 0.0f, 0.0f);
  _rotate = 0.0f;
  _scale = 1.0f;
#endif

  nassertv(trans->has_components());
  _pos = trans->get_pos();

  const LVecBase3 &hpr = trans->get_hpr();
  nassertv(IS_NEARLY_ZERO(hpr[1]) && IS_NEARLY_ZERO(hpr[2]));
  _rotate = hpr[0];

  nassertv(trans->has_uniform_scale());
  _scale = trans->get_uniform_scale();
}

/**
 *
 */
void STTransform::
output(std::ostream &out) const {
  out << "STTransform(" << _pos << ", " << _rotate << ", " << _scale << ")";
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void STTransform::
write_datagram(BamWriter *manager, Datagram &dg) {
  _pos.write_datagram(dg);
  dg.add_stdfloat(_rotate);
  dg.add_stdfloat(_scale);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new SpeedTreeNode.
 */
void STTransform::
fillin(DatagramIterator &scan, BamReader *manager) {
  _pos.read_datagram(scan);
  _rotate = scan.get_stdfloat();
  _scale = scan.get_stdfloat();
}
