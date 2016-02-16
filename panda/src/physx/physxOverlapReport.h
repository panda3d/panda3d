/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxOverlapReport.h
 * @author enn0x
 * @date 2009-10-21
 */

#ifndef PHYSXOVERLAPREPORT_H
#define PHYSXOVERLAPREPORT_H

#include "pandabase.h"
#include "pvector.h"
#include "pointerTo.h"

#include "config_physx.h"
#include "physx_includes.h"

class PhysxShape;

class PhysxUserEntityReport : public NxUserEntityReport<NxShape *> {};

/**
 * Objects of this class are returned by the 'overlap shape' methods, for
 * example overlapSphereShapes.  They contain an iterable list of all sshapes
 * that the raycast query produced.
 */
class EXPCL_PANDAPHYSX PhysxOverlapReport : public PhysxUserEntityReport {

PUBLISHED:
  unsigned int get_num_overlaps() const;
  PhysxShape *get_first_overlap();
  PhysxShape *get_next_overlap();
  PhysxShape *get_overlap(unsigned int idx);
  MAKE_SEQ(get_overlaps, get_num_overlaps, get_overlap);

public:
  INLINE PhysxOverlapReport();
  INLINE ~PhysxOverlapReport();

  virtual bool onEvent(NxU32 nbEntities, NxShape **entities);

private:
  typedef pvector<PT(PhysxShape)> Overlaps;
  Overlaps _overlaps;

  typedef Overlaps::const_iterator const_iterator;
  const_iterator _iterator;
};

#include "physxOverlapReport.I"

#endif // PHYSXOVERLAPREPORT_H
