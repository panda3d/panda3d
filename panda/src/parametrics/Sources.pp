#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define TARGET p3parametrics
  #define LOCAL_LIBS \
    p3pgraph p3linmath p3express p3putil p3pandabase
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    config_parametrics.h  \
    cubicCurveseg.h curveFitter.I curveFitter.h  \
    hermiteCurve.h \
    nurbsCurve.I nurbsCurve.h \
    nurbsCurveEvaluator.I nurbsCurveEvaluator.h \
    nurbsCurveInterface.I nurbsCurveInterface.h \
    nurbsCurveResult.I nurbsCurveResult.h \
    nurbsBasisVector.I nurbsBasisVector.h \
    nurbsSurfaceEvaluator.I nurbsSurfaceEvaluator.h \
    nurbsSurfaceResult.I nurbsSurfaceResult.h \
    nurbsVertex.h nurbsVertex.I \
    parametricCurve.h  \
    parametricCurveCollection.I parametricCurveCollection.h  \
    piecewiseCurve.h \
    ropeNode.I ropeNode.h \
    sheetNode.I sheetNode.h

  #define INCLUDED_SOURCES \
    config_parametrics.cxx cubicCurveseg.cxx  \
    curveFitter.cxx hermiteCurve.cxx  \
    nurbsCurve.cxx \
    nurbsCurveEvaluator.cxx \
    nurbsCurveInterface.cxx  \
    nurbsCurveResult.cxx \
    nurbsBasisVector.cxx \
    nurbsSurfaceEvaluator.cxx \
    nurbsSurfaceResult.cxx \
    nurbsVertex.cxx \
    parametricCurve.cxx parametricCurveCollection.cxx  \
    piecewiseCurve.cxx \
    ropeNode.cxx \
    sheetNode.cxx

  #define INSTALL_HEADERS \
    config_parametrics.h \
    cubicCurveseg.h \
    curveFitter.I curveFitter.h \
    hermiteCurve.h \
    nurbsCurve.I nurbsCurve.h \
    nurbsCurveEvaluator.I nurbsCurveEvaluator.h \
    nurbsCurveInterface.I nurbsCurveInterface.h \
    nurbsCurveResult.I nurbsCurveResult.h \
    nurbsBasisVector.I nurbsBasisVector.h \
    nurbsSurfaceEvaluator.I nurbsSurfaceEvaluator.h \
    nurbsSurfaceResult.I nurbsSurfaceResult.h \
    nurbsVertex.h nurbsVertex.I \
    parametricCurve.h \
    parametricCurveCollection.I parametricCurveCollection.h \
    piecewiseCurve.h \
    ropeNode.I ropeNode.h \
    sheetNode.I sheetNode.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_parametrics
  #define LOCAL_LIBS \
    p3parametrics

  #define SOURCES \
    test_parametrics.cxx

#end test_bin_target

