set(P3FLT_HEADERS
  config_flt.h
  fltBead.h
  fltBeadID.h
  fltCurve.h fltCurve.I
  fltError.h
  fltExternalReference.h
  fltEyepoint.h
  fltFace.h fltFace.I
  fltGeometry.h fltGeometry.I
  fltGroup.h
  fltHeader.h
  fltInstanceDefinition.h
  fltInstanceRef.h
  fltLightSourceDefinition.h
  fltLocalVertexPool.h fltLocalVertexPool.I
  fltLOD.h
  fltMaterial.h
  fltMesh.h fltMesh.I
  fltMeshPrimitive.h fltMeshPrimitive.I
  fltObject.h
  fltOpcode.h
  fltPackedColor.h fltPackedColor.I
  fltRecord.h fltRecord.I
  fltRecordReader.h
  fltRecordWriter.h
  fltTexture.h
  fltTrackplane.h
  fltTransformGeneralMatrix.h
  fltTransformPut.h
  fltTransformRecord.h
  fltTransformRotateAboutEdge.h
  fltTransformRotateAboutPoint.h
  fltTransformRotateScale.h
  fltTransformScale.h
  fltTransformTranslate.h
  fltUnsupportedRecord.h
  fltVectorRecord.h
  fltVertex.h fltVertex.I
  fltVertexList.h
)

set(P3FLT_SOURCES
  config_flt.cxx
  fltBead.cxx
  fltBeadID.cxx
  fltCurve.cxx
  fltError.cxx
  fltExternalReference.cxx
  fltEyepoint.cxx
  fltFace.cxx
  fltGeometry.cxx
  fltGroup.cxx
  fltHeader.cxx
  fltInstanceDefinition.cxx
  fltInstanceRef.cxx
  fltLightSourceDefinition.cxx
  fltLocalVertexPool.cxx
  fltLOD.cxx
  fltMaterial.cxx
  fltMesh.cxx
  fltMeshPrimitive.cxx
  fltObject.cxx
  fltOpcode.cxx
  fltPackedColor.cxx
  fltRecord.cxx
  fltRecordReader.cxx
  fltRecordWriter.cxx
  fltTexture.cxx
  fltTrackplane.cxx
  fltTransformGeneralMatrix.cxx
  fltTransformPut.cxx
  fltTransformRecord.cxx
  fltTransformRotateAboutEdge.cxx
  fltTransformRotateAboutPoint.cxx
  fltTransformRotateScale.cxx
  fltTransformScale.cxx
  fltTransformTranslate.cxx
  fltUnsupportedRecord.cxx
  fltVectorRecord.cxx
  fltVertex.cxx
  fltVertexList.cxx
)

composite_sources(p3flt P3FLT_SOURCES)
add_library(p3flt STATIC ${P3FLT_HEADERS} ${P3FLT_SOURCES})
target_link_libraries(p3flt p3pandatoolbase panda)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
