// Filename: config_flt.cxx
// Created by:  drose (24Aug00)
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

#include "config_flt.h"
#include "fltRecord.h"
#include "fltBead.h"
#include "fltBeadID.h"
#include "fltGroup.h"
#include "fltObject.h"
#include "fltGeometry.h"
#include "fltFace.h"
#include "fltCurve.h"
#include "fltMesh.h"
#include "fltLocalVertexPool.h"
#include "fltMeshPrimitive.h"
#include "fltVectorRecord.h"
#include "fltVertexList.h"
#include "fltLOD.h"
#include "fltInstanceDefinition.h"
#include "fltInstanceRef.h"
#include "fltHeader.h"
#include "fltVertex.h"
#include "fltMaterial.h"
#include "fltTexture.h"
#include "fltLightSourceDefinition.h"
#include "fltUnsupportedRecord.h"
#include "fltTransformRecord.h"
#include "fltTransformGeneralMatrix.h"
#include "fltTransformPut.h"
#include "fltTransformRotateAboutEdge.h"
#include "fltTransformRotateAboutPoint.h"
#include "fltTransformScale.h"
#include "fltTransformTranslate.h"
#include "fltTransformRotateScale.h"
#include "fltExternalReference.h"

#include "dconfig.h"

Configure(config_flt);
NotifyCategoryDef(flt, "");


ConfigVariableBool flt_error_abort
("flt-error-abort", false,
 PRC_DESC("Set this true to trigger an assertion failure (and core dump) "
          "immediately when an error is detected on reading or writing a flt "
          "file.  This is primarily useful for debugging the flt reader itself, "
          "to generate a stack trace to determine precisely at what point a flt "

          "file failed."));


ConfigureFn(config_flt) {
  init_libflt();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libflt
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libflt() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  FltRecord::init_type();
  FltBead::init_type();
  FltBeadID::init_type();
  FltGroup::init_type();
  FltObject::init_type();
  FltGeometry::init_type();
  FltFace::init_type();
  FltCurve::init_type();
  FltMesh::init_type();
  FltLocalVertexPool::init_type();
  FltMeshPrimitive::init_type();
  FltVectorRecord::init_type();
  FltVertexList::init_type();
  FltLOD::init_type();
  FltInstanceDefinition::init_type();
  FltInstanceRef::init_type();
  FltHeader::init_type();
  FltVertex::init_type();
  FltMaterial::init_type();
  FltTexture::init_type();
  FltLightSourceDefinition::init_type();
  FltUnsupportedRecord::init_type();
  FltTransformRecord::init_type();
  FltTransformGeneralMatrix::init_type();
  FltTransformPut::init_type();
  FltTransformRotateAboutEdge::init_type();
  FltTransformRotateAboutPoint::init_type();
  FltTransformScale::init_type();
  FltTransformTranslate::init_type();
  FltTransformRotateScale::init_type();
  FltExternalReference::init_type();
}

