// Filename: isoPlacer.h
// Created by:  drose (13Oct03)
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

