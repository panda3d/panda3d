/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltToEggLevelState.h
 * @author drose
 * @date 2001-04-18
 */

#ifndef FLTTOEGGLEVELSTATE_H
#define FLTTOEGGLEVELSTATE_H

#include "pandatoolbase.h"
#include "fltGeometry.h"

class FltObject;
class FltBead;
class EggGroupNode;
class EggGroup;
class FltToEggConverter;

/**
 * This keeps track of relevant things about the traversal as we walk through
 * the flt hierarchy.
 */
class FltToEggLevelState {
public:
  INLINE FltToEggLevelState(FltToEggConverter *converter);
  INLINE FltToEggLevelState(const FltToEggLevelState &copy);
  INLINE void operator = (const FltToEggLevelState &copy);
  ~FltToEggLevelState();

  EggGroupNode *get_synthetic_group(const std::string &name,
                                    const FltBead *transform_bead,
                                    FltGeometry::BillboardType type = FltGeometry::BT_none);

  void set_transform(const FltBead *flt_bead, EggGroup *egg_group);

  const FltObject *_flt_object;
  EggGroupNode *_egg_parent;

private:
  class ParentNodes {
  public:
    ParentNodes();

    EggGroup *_axial_billboard;
    EggGroup *_point_billboard;
    EggGroup *_plain;
  };

  typedef pmap<LMatrix4d, ParentNodes *> Parents;
  Parents _parents;

  FltToEggConverter *_converter;
};

#include "fltToEggLevelState.I"

#endif
