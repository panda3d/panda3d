// Filename: fltEyepoint.h
// Created by:  drose (26Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FLTEYEPOINT_H
#define FLTEYEPOINT_H

#include "pandatoolbase.h"

#include "luse.h"

class FltRecordReader;
class FltRecordWriter;

////////////////////////////////////////////////////////////////////
//       Class : FltEyepoint
// Description : A single eyepoint entry in the eyepoint/trackplane
//               palette.
////////////////////////////////////////////////////////////////////
class FltEyepoint {
public:
  FltEyepoint();

  bool extract_record(FltRecordReader &reader);
  bool build_record(FltRecordWriter &writer) const;

public:
  LPoint3d _rotation_center;
  LVecBase3f _hpr;
  LMatrix4f _rotation;
  float _fov;
  float _scale;
  float _near_clip;
  float _far_clip;
  LMatrix4f _fly_through;
  LPoint3f _eyepoint;
  float _fly_through_yaw;
  float _fly_through_pitch;
  LVector3f _eyepoint_direction;
  bool _no_fly_through;
  bool _ortho_mode;
  bool _is_valid;
  int _image_offset_x;
  int _image_offset_y;
  int _image_zoom;
};

#endif



