// Filename: egg_parametrics.h
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

#ifndef EGG_PARAMETRICS_H
#define EGG_PARAMETRICS_H

#include "pandabase.h"

#include "eggNurbsSurface.h"
#include "eggNurbsCurve.h"
#include "nurbsSurfaceEvaluator.h"
#include "nurbsCurveEvaluator.h"

BEGIN_PUBLISH

////////////////////////////////////////////////////////////////////
//     Function: make_nurbs_surface
//  Description: Returns a new NurbsSurfaceEvaluator that's filled in
//               with the values from the given EggSurface (and
//               transformed by the indicated matrix), or NULL if the
//               object is invalid.  If there is vertex color, it will
//               be applied to values 0 - 3 of the extended vertex
//               values.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT(NurbsSurfaceEvaluator)
make_nurbs_surface(EggNurbsSurface *egg_surface, const LMatrix4d &mat);

////////////////////////////////////////////////////////////////////
//     Function: make_nurbs_curve
//  Description: Returns a new NurbsCurveEvaluator that's filled in
//               with the values from the given EggCurve (and
//               transformed by the indicated matrix), or NULL if the
//               object is invalid.  If there is vertex color, it will
//               be applied to values 0 - 3 of the extended vertex
//               values.
////////////////////////////////////////////////////////////////////
EXPCL_PANDAEGG PT(NurbsCurveEvaluator)
make_nurbs_curve(EggNurbsCurve *egg_curve, const LMatrix4d &mat);

END_PUBLISH

#endif
