#begin ss_lib_target
  #define TARGET flt
  #define LOCAL_LIBS converter pandatoolbase
  #define OTHER_LIBS \
    mathutil:c linmath:c putil:c express:c panda:m dtoolconfig dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_flt.cxx config_flt.h fltBead.cxx fltBead.h fltBeadID.cxx \
    fltBeadID.h fltCurve.I fltCurve.cxx fltCurve.h \
    fltError.cxx fltError.h fltExternalReference.cxx \
    fltExternalReference.h fltEyepoint.cxx fltEyepoint.h fltFace.I \
    fltFace.cxx fltFace.h fltGeometry.I fltGeometry.cxx fltGeometry.h \
    fltGroup.cxx fltGroup.h fltHeader.cxx \
    fltHeader.h fltInstanceDefinition.cxx fltInstanceDefinition.h \
    fltInstanceRef.cxx fltInstanceRef.h fltLOD.cxx fltLOD.h \
    fltLightSourceDefinition.cxx fltLightSourceDefinition.h \
    fltLocalVertexPool.I fltLocalVertexPool.cxx fltLocalVertexPool.h \
    fltMaterial.cxx fltMaterial.h fltMesh.I fltMesh.cxx fltMesh.h \
    fltMeshPrimitive.I fltMeshPrimitive.cxx fltMeshPrimitive.h \
    fltObject.cxx fltObject.h \
    fltOpcode.cxx fltOpcode.h fltPackedColor.I fltPackedColor.cxx \
    fltPackedColor.h fltRecord.I fltRecord.cxx fltRecord.h \
    fltRecordReader.cxx fltRecordReader.h fltRecordWriter.cxx \
    fltRecordWriter.h fltTexture.cxx fltTexture.h fltTrackplane.cxx \
    fltTrackplane.h fltTransformGeneralMatrix.cxx \
    fltTransformGeneralMatrix.h fltTransformPut.cxx fltTransformPut.h \
    fltTransformRecord.cxx fltTransformRecord.h \
    fltTransformRotateAboutEdge.cxx fltTransformRotateAboutEdge.h \
    fltTransformRotateAboutPoint.cxx fltTransformRotateAboutPoint.h \
    fltTransformRotateScale.cxx fltTransformRotateScale.h \
    fltTransformScale.cxx fltTransformScale.h fltTransformTranslate.cxx \
    fltTransformTranslate.h fltUnsupportedRecord.cxx \
    fltUnsupportedRecord.h fltVertex.I fltVertex.cxx fltVertex.h \
    fltVertexList.cxx fltVertexList.h

  #define INSTALL_HEADERS \
    fltBead.h fltBeadID.h fltCurve.I fltCurve.h \
    fltError.h fltExternalReference.h \
    fltEyepoint.h fltFace.I fltFace.h fltGeometry.I fltGeometry.h \
    fltGroup.h fltHeader.h \
    fltInstanceDefinition.h fltInstanceRef.h fltLOD.h \
    fltLightSourceDefinition.h fltLocalVertexPool.I fltLocalVertexPool.h \
    fltMaterial.h fltMesh.I fltMesh.h \
    fltMeshPrimitive.I fltMeshPrimitive.h \
    fltObject.h fltOpcode.h \
    fltPackedColor.I fltPackedColor.h fltRecord.I fltRecord.h \
    fltRecordReader.h fltRecordWriter.h fltTexture.h fltTrackplane.h \
    fltTransformGeneralMatrix.h fltTransformPut.h fltTransformRecord.h \
    fltTransformRotateAboutEdge.h fltTransformRotateAboutPoint.h \
    fltTransformRotateScale.h fltTransformScale.h \
    fltTransformTranslate.h fltUnsupportedRecord.h fltVertex.I \
    fltVertex.h fltVertexList.h

#end ss_lib_target
