// Filename: isoPlacer.h
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

#ifndef ISOPLACER_H
#define ISOPLACER_H

#include "pandatoolbase.h"
#include "pvector.h"
#include "vector_double.h"

class NurbsSurfaceResult;

////////////////////////////////////////////////////////////////////
//       Class : IsoPlacer
// Description : Contains the logic used to place isoparams where
//               they'll do the most good on a surface.
////////////////////////////////////////////////////////////////////
class IsoPlacer {
public:
  INLINE IsoPlacer();

  void get_scores(int subdiv, int across, double ratio,
                  NurbsSurfaceResult *surf, bool s);
  void place(int count, pvector<double> &iso_points);

  INLINE double get_total_score() const;

  vector_double _cscore, _sscore, _cint;
  int _maxi;
};

#include "isoPlacer.I"

#endif

