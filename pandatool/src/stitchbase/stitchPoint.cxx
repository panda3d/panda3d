// Filename: stitchPoint.cxx
// Created by:  drose (04Nov99)
// 
////////////////////////////////////////////////////////////////////

#include "stitchPoint.h"

StitchPoint::
StitchPoint(const string &name) :
  _name(name)
{
  _space_known = false;
}

void StitchPoint::
set_space(const LVector3d &space) {
  _space_known = true;
  _space = space;
}

