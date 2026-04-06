/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ptsToBam.h
 * @author drose
 * @date 2000-06-28
 */

#ifndef PTSTOBAM_H
#define PTSTOBAM_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "withOutputFile.h"
#include "filename.h"
#include "vector_string.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomNode.h"

/**
 *
 */
class PtsToBam : public ProgramBase, public WithOutputFile {
public:
  PtsToBam();

  void run();

protected:
  virtual bool handle_args(Args &args);

private:
  void process_line(const std::string &line);
  void add_point(const vector_string &words);

  void open_vertex_data();
  void close_vertex_data();

private:
  Filename _pts_filename;
  double _decimate_divisor;
  double _decimate_factor;

  int _line_number;
  int _point_number;
  int _num_points_expected;
  int _num_points_found;
  int _num_points_added;
  int _num_vdatas;

  double _decimated_point_number;
  PT(GeomNode) _gnode;
  PT(GeomVertexData) _data;
  GeomVertexWriter _vertex;
};

#endif
