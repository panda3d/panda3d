// Filename: fltToEggLevelState.h
// Created by:  drose (18Apr01)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTTOEGGLEVELSTATE_H
#define FLTTOEGGLEVELSTATE_H

#include <pandatoolbase.h>

#include <fltGeometry.h>

class FltObject;
class EggGroupNode;
class EggGroup;

////////////////////////////////////////////////////////////////////
//       Class : FltToEggLevelState
// Description : This keeps track of relevant things about the
//               traversal as we walk through the flt hierarchy.
////////////////////////////////////////////////////////////////////
class FltToEggLevelState {
public:
  INLINE FltToEggLevelState();
  INLINE FltToEggLevelState(const FltToEggLevelState &copy);
  INLINE void operator = (const FltToEggLevelState &copy);
  ~FltToEggLevelState();

  EggGroupNode *get_synthetic_group(const string &name,
                                    const LMatrix4d &transform,
                                    FltGeometry::BillboardType type = FltGeometry::BT_none);

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

  typedef map<LMatrix4d, ParentNodes *> Parents;
  Parents _parents;
};

#include "fltToEggLevelState.I"
  
#endif
