// Filename: config_parametrics.cxx
// Created by:  drose (19Mar00)
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

#include "classicNurbsCurve.h"
#include "config_parametrics.h"
#include "cubicCurveseg.h"
#include "curveFitter.h"
#include "hermiteCurve.h"
#include "nurbsCurveDrawer.h"
#include "nurbsCurveInterface.h"
#include "parametricCurve.h"
#include "parametricCurveDrawer.h"
#include "piecewiseCurve.h"
#include "ropeNode.h"
#include "sheetNode.h"

#ifdef HAVE_NURBSPP
#include "nurbsPPCurve.h"
#endif

#include "get_config_path.h"

Configure(config_parametrics);
NotifyCategoryDef(parametrics, "");

ConfigureFn(config_parametrics) {
  ClassicNurbsCurve::init_type();
  CubicCurveseg::init_type();
  CurveFitter::init_type();
  HermiteCurve::init_type();
  NurbsCurveDrawer::init_type();
  NurbsCurveInterface::init_type();
  ParametricCurve::init_type();
  ParametricCurveDrawer::init_type();
  PiecewiseCurve::init_type();
  RopeNode::init_type();
  SheetNode::init_type();

#ifdef HAVE_NURBSPP
  NurbsPPCurve::init_type();
  NurbsPPCurve::register_with_read_factory();
#endif

  ClassicNurbsCurve::register_with_read_factory();
  CubicCurveseg::register_with_read_factory();
  HermiteCurve::register_with_read_factory();
  RopeNode::register_with_read_factory();
  SheetNode::register_with_read_factory();
}

const DSearchPath &
get_parametrics_path() {
  static DSearchPath *parametrics_path = NULL;
  return get_config_path("parametrics-path", parametrics_path);
}
