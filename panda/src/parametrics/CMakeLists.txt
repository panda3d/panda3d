set(P3PARAMETRICS_HEADERS
  config_parametrics.h
  cubicCurveseg.h curveFitter.I curveFitter.h
  hermiteCurve.h
  nurbsCurve.I nurbsCurve.h
  nurbsCurveEvaluator.I nurbsCurveEvaluator.h
  nurbsCurveInterface.I nurbsCurveInterface.h
  nurbsCurveResult.I nurbsCurveResult.h
  nurbsBasisVector.I nurbsBasisVector.h
  nurbsSurfaceEvaluator.I nurbsSurfaceEvaluator.h
  nurbsSurfaceResult.I nurbsSurfaceResult.h
  nurbsVertex.h nurbsVertex.I
  parametricCurve.h
  parametricCurveCollection.I parametricCurveCollection.h
  piecewiseCurve.h
  ropeNode.I ropeNode.h
  sheetNode.I sheetNode.h
)
set(P3PARAMETRICS_SOURCES
  config_parametrics.cxx cubicCurveseg.cxx
  curveFitter.cxx hermiteCurve.cxx
  nurbsCurve.cxx
  nurbsCurveEvaluator.cxx
  nurbsCurveInterface.cxx
  nurbsCurveResult.cxx
  nurbsBasisVector.cxx
  nurbsSurfaceEvaluator.cxx
  nurbsSurfaceResult.cxx
  nurbsVertex.cxx
  parametricCurve.cxx parametricCurveCollection.cxx
  piecewiseCurve.cxx
  ropeNode.cxx
  sheetNode.cxx
)

composite_sources(p3parametrics P3PARAMETRICS_SOURCES)
add_library(p3parametrics ${P3PARAMETRICS_HEADERS} ${P3PARAMETRICS_SOURCES})
target_link_libraries(p3parametrics p3pgraph)
target_interrogate(p3parametrics ALL)

install(TARGETS p3parametrics DESTINATION lib)
install(FILES ${P3PARAMETRICS_HEADERS} DESTINATION include/panda3d)

#begin test_bin_target
  #define TARGET test_parametrics
  #define LOCAL_LIBS
#    p3parametrics

  #define SOURCES
#    test_parametrics.cxx

#end test_bin_target
