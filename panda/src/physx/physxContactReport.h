/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxContactReport.h
 * @author enn0x
 * @date 2009-09-19
 */

#ifndef PHYSXCONTACTREPORT_H
#define PHYSXCONTACTREPORT_H

#include "pandabase.h"
#include "pStatCollector.h"
#include "physx_includes.h"

/**
 * Implementation of the NxUserContactReport interface.
 */
class EXPCL_PANDAPHYSX PhysxContactReport : public NxUserContactReport {

public:
  INLINE PhysxContactReport();
  INLINE ~PhysxContactReport();

  void enable();
  void disable();
  bool is_enabled() const;

  void onContactNotify(NxContactPair& pair, NxU32 flags);

private:
  bool _enabled;
  static PStatCollector _pcollector;
};

#include "physxContactReport.I"

#endif // PHYSXCONTACTREPORT_H
