// Filename: stitchPoint.h
// Created by:  drose (04Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHPOINT_H
#define STITCHPOINT_H

#include <luse.h>

#include <string>

class StitchImage;

class StitchPoint {
public:
  StitchPoint(const string &name);

  void set_space(const LVector3d &space);

  string _name;
  bool _space_known;
  LVector3d _space;

  typedef set<StitchImage *> Images;
  Images _images;
};

#endif

