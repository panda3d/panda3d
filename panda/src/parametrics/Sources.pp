#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define USE_PACKAGES nurbspp

#begin lib_target
  #define TARGET parametrics
  #define LOCAL_LIBS \
    pgraph grutil linmath express putil pandabase
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx    

  #define SOURCES \
    classicNurbsCurve.I classicNurbsCurve.h config_parametrics.h  \
    cubicCurveseg.h parametricCurveDrawer.I  \
    parametricCurveDrawer.h curveFitter.I curveFitter.h  \
    hermiteCurve.h nurbsCurve.h \
    nurbsCurveDrawer.I nurbsCurveDrawer.h \
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
    $[if $[HAVE_NURBSPP], nurbsPPCurve.cxx nurbsPPCurve.h]


  #define INCLUDED_SOURCES \
    classicNurbsCurve.cxx config_parametrics.cxx cubicCurveseg.cxx  \
    parametricCurveDrawer.cxx curveFitter.cxx hermiteCurve.cxx  \
    nurbsCurveDrawer.cxx \
    nurbsCurveEvaluator.cxx \
    nurbsCurveInterface.cxx  \
    nurbsCurveResult.cxx \
    nurbsBasisVector.cxx \
    nurbsSurfaceEvaluator.cxx \
    nurbsSurfaceResult.cxx \
    nurbsVertex.cxx \
    parametricCurve.cxx parametricCurveCollection.cxx  \
    piecewiseCurve.cxx \
    ropeNode.cxx

  #define INSTALL_HEADERS \
    classicNurbsCurve.I classicNurbsCurve.h \
    config_parametrics.h \
    cubicCurveseg.h \
    parametricCurveDrawer.I parametricCurveDrawer.h \
    curveFitter.I curveFitter.h \
    hermiteCurve.h \
    nurbsCurve.h \
    nurbsCurveDrawer.I nurbsCurveDrawer.h \
    nurbsCurveEvaluator.I nurbsCurveEvaluator.h \
    nurbsCurveInterface.I nurbsCurveInterface.h \
    nurbsCurveResult.I nurbsCurveResult.h \
    nurbsBasisVector.I nurbsBasisVector.h \
    nurbsSurfaceEvaluator.I nurbsSurfaceEvaluator.h \
    nurbsSurfaceResult.I nurbsSurfaceResult.h \
    nurbsVertex.h nurbsVertex.I \
    nurbsPPCurve.h \
    parametricCurve.h \
    parametricCurveCollection.I parametricCurveCollection.h \
    piecewiseCurve.h \
    ropeNode.I ropeNode.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_parametrics
  #define LOCAL_LIBS \
    parametrics

  #define SOURCES \
    test_parametrics.cxx

#end test_bin_target

