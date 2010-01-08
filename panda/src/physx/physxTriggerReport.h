// Filename: physxTriggerReport.h
// Created by:  enn0x (19Sep09)
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

#ifndef PHYSXTRIGGERREPORT_H
#define PHYSXTRIGGERREPORT_H

#include "pandabase.h"
#include "pStatCollector.h"

#include "physx_includes.h"

////////////////////////////////////////////////////////////////////
//       Class : PhysxTriggerReport
// Description : Implementation of the NxUserTriggerReport
//               interface.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxTriggerReport : public NxUserTriggerReport {

public:
  INLINE PhysxTriggerReport();
  INLINE ~PhysxTriggerReport();

  void enable();
  void disable();
  bool is_enabled() const;

  void onTrigger(NxShape &triggerShape, NxShape &otherShape, NxTriggerFlag status);

private:
  bool _enabled;
  static PStatCollector _pcollector;
};

#include "physxTriggerReport.I"

#endif // PHYSXTRIGGERREPORT_H
