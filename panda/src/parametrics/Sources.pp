#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define TARGET parametrics
  #define LOCAL_LIBS \
    pgraph linmath express putil pandabase
    
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
    parametrics

  #define SOURCES \
    test_parametrics.cxx

#end test_bin_target

