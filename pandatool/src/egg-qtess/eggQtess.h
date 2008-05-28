// Filename: eggQtess.h
// Created by:  drose (13Oct03)
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

#ifndef EGGQTESS_H
#define EGGQTESS_H

#include "pandatoolbase.h"
#include "eggFilter.h"
#include "qtessInputFile.h"
#include "qtessSurface.h"
#include "pointerTo.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : EggQtess
// Description : A program to tesselate NURBS surfaces appearing
//               within an egg file into polygons, using variations on
//               a quick uniform tesselation.
////////////////////////////////////////////////////////////////////
class EggQtess : public EggFilter {
public:
  EggQtess();

  void run();

protected:
  virtual bool handle_args(ProgramBase::Args &args);

private:
  void describe_qtess_format();
  void find_surfaces(EggNode *egg_node);

  Filename _qtess_filename;
  double _uniform_per_isoparam;
  int _uniform_per_surface;
  int _total_tris;
  bool _qtess_output;
  bool _describe_qtess;

  QtessInputFile _qtess_file;
  
  typedef pvector< PT(QtessSurface) > Surfaces;
  Surfaces _surfaces;
};

#endif


