// Filename: config_parametrics.cxx
// Created by:  drose (19Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_parametrics.h"
#include "curve.h"
#include "curveDrawer.h"
#include "curveFitter.h"
#include "hermiteCurve.h"
#include "nurbsCurve.h"
#include "nurbsCurveDrawer.h"

#include "get_config_path.h"

Configure(config_parametrics);
NotifyCategoryDef(parametrics, "");

ConfigureFn(config_parametrics) {
  ParametricCurve::init_type();
  PiecewiseCurve::init_type();
  CubicCurveseg::init_type();
  ParametricCurveDrawer::init_type();
  CurveFitter::init_type();
  HermiteCurve::init_type();
  NurbsCurve::init_type();
  NurbsCurveDrawer::init_type();
}

const DSearchPath &
get_parametrics_path() {
  static DSearchPath *parametrics_path = NULL;
  return get_config_path("parametrics-path", parametrics_path);
}
