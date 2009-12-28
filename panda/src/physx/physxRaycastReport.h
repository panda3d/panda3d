// Filename: physxRaycastReport.h
// Created by:  enn0x (21Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef PHYSXRAYCASTREPORT_H
#define PHYSXRAYCASTREPORT_H

#include "pandabase.h"
#include "pvector.h"

#include "config_physx.h"

class PhysxRaycastHit;

////////////////////////////////////////////////////////////////////
//       Class : PhysxRaycastReport
// Description : Objects of this class are returned by the 'raycast
//               all' methods. They contain an iterable list of all
//               hits that the raycast query produced.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxRaycastReport : public NxUserRaycastReport {

PUBLISHED:
  unsigned int get_num_hits() const;
  PhysxRaycastHit get_first_hit();
  PhysxRaycastHit get_next_hit();
  PhysxRaycastHit get_hit(unsigned int idx);
  MAKE_SEQ(get_hits, get_num_hits, get_hit);

public:
  INLINE PhysxRaycastReport();
  INLINE ~PhysxRaycastReport();

  virtual bool onHit(const NxRaycastHit& hit);

private:
  typedef pvector<PhysxRaycastHit> Hits;
  Hits _hits;

  typedef Hits::const_iterator const_iterator;
  const_iterator _iterator;
};

#include "physxRaycastReport.I"

#endif // PHYSXRAYCASTREPORT_H
