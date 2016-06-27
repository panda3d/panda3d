/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltTrackplane.h
 * @author drose
 * @date 2000-08-26
 */

#ifndef FLTTRACKPLANE_H
#define FLTTRACKPLANE_H

#include "pandatoolbase.h"

#include "luse.h"

class FltRecordReader;
class FltRecordWriter;

/**
 * A single trackplane entry in the eyepoint/trackplane palette.
 */
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
  PN_stdfloat _grid_angle;
  double _grid_spacing_x;
  double _grid_spacing_y;
  bool _snap_to_grid;
  double _grid_size;
  int _grid_spacing_direction;
  int _grid_mask;
};

#endif
