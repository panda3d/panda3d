// Filename: config_flt.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "config_flt.h"
#include "fltRecord.h"
#include "fltBead.h"
#include "fltBeadID.h"
#include "fltGroup.h"
#include "fltObject.h"
#include "fltFace.h"
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

#include <dconfig.h>

Configure(config_flt);
NotifyCategoryDef(flt, "");

// Set this true to trigger an assertion failure (and core dump)
// immediately when an error is detected on reading or writing a flt
// file.  This is primarily useful for debugging the flt reader
// itself, to generate a stack trace to determine precisely at what
// point a flt file failed.
const bool flt_error_abort = config_flt.GetBool("flt-error-abort", false);

ConfigureFn(config_flt) {
  FltRecord::init_type();
  FltBead::init_type();
  FltBeadID::init_type();
  FltGroup::init_type();
  FltObject::init_type();
  FltFace::init_type();
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

