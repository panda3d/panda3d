// Filename: fltTrackplane.h
// Created by:  drose (26Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef FLTTRACKPLANE_H
#define FLTTRACKPLANE_H

#include "pandatoolbase.h"

#include "luse.h"

class FltRecordReader;
class FltRecordWriter;

////////////////////////////////////////////////////////////////////
//       Class : FltTrackplane
// Description : A single trackplane entry in the eyepoint/trackplane
//               palette.
////////////////////////////////////////////////////////////////////
class FltTrackplane {
public:
  FltTrackplane();

  bool extract_record(FltRecordReader &reader);
  bool build_record(FltRecordWriter &writer) const;

public:
  LPoint3d _origin;
  LPoint3d _alignment;
  LVector3d _plane;
  bool _grid_state;
  bool _grid_under;
  float _grid_angle;
  double _grid_spacing_x;
  double _grid_spacing_y;
  bool _snap_to_grid;
  double _grid_size;
  int _grid_spacing_direction;
  int _grid_mask;
};

#endif



