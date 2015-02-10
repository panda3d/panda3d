// Filename: odeGeom_ext.cxx
// Created by:  rdb (11Dec13)
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

#include "odeGeom_ext.h"

#include "odeBoxGeom.h"
//#include "odeConvexGeom.h"
#include "odeGeom.h"
#include "odeHashSpace.h"
#include "odeCappedCylinderGeom.h"
//#include "odeHeightfieldGeom.h"
#include "odePlaneGeom.h"
#include "odeQuadTreeSpace.h"
#include "odeRayGeom.h"
#include "odeSimpleSpace.h"
#include "odeSpace.h"
#include "odeSphereGeom.h"
#include "odeTriMeshGeom.h"

#ifdef HAVE_PYTHON

#ifndef CPPPARSER
extern Dtool_PyTypedObject Dtool_OdeBoxGeom;
//extern Dtool_PyTypedObject Dtool_OdeConvexGeom;
extern Dtool_PyTypedObject Dtool_OdeGeom;
extern Dtool_PyTypedObject Dtool_OdeHashSpace;
extern Dtool_PyTypedObject Dtool_OdeCappedCylinderGeom;
//extern Dtool_PyTypedObject Dtool_OdeHeightfieldGeom;
extern Dtool_PyTypedObject Dtool_OdePlaneGeom;
extern Dtool_PyTypedObject Dtool_OdeQuadTreeSpace;
extern Dtool_PyTypedObject Dtool_OdeRayGeom;
extern Dtool_PyTypedObject Dtool_OdeSimpleSpace;
extern Dtool_PyTypedObject Dtool_OdeSpace;
extern Dtool_PyTypedObject Dtool_OdeSphereGeom;
extern Dtool_PyTypedObject Dtool_OdeTriMeshGeom;
#endif

////////////////////////////////////////////////////////////////////
//     Function: OdeGeom::convert
//       Access: Published
//  Description: Do a sort of pseudo-downcast on this space in
//               order to expose its specialized functions.
////////////////////////////////////////////////////////////////////
PyObject *Extension<OdeGeom>::
convert() const {
  Dtool_PyTypedObject *class_type;
  TypedObject *geom;

  switch (_this->get_class()) {
  case OdeGeom::GC_sphere:
    geom = new OdeSphereGeom(_this->get_id());
    class_type = &Dtool_OdeSphereGeom;
    break;

  case OdeGeom::GC_box:
    geom = new OdeBoxGeom(_this->get_id());
    class_type = &Dtool_OdeBoxGeom;
    break;

  case OdeGeom::GC_capped_cylinder:
    geom = new OdeCappedCylinderGeom(_this->get_id());
    class_type = &Dtool_OdeCappedCylinderGeom;
    break;

  case OdeGeom::GC_plane:
    geom = new OdePlaneGeom(_this->get_id());
    class_type = &Dtool_OdePlaneGeom;
    break;

  case OdeGeom::GC_ray:
    geom = new OdeRayGeom(_this->get_id());
    class_type = &Dtool_OdeRayGeom;
    break;

  //case OdeGeom::GC_convex:
  //  geom = new OdeConvexGeom(_this->get_id());
  //  class_type = &Dtool_OdeConvexGeom;
  //  break;

  case OdeGeom::GC_tri_mesh:
    geom = new OdeTriMeshGeom(_this->get_id());
    class_type = &Dtool_OdeTriMeshGeom;
    break;

  //case OdeGeom::GC_heightfield:
  //  geom = new OdeHeightfieldGeom(_this->get_id());
  //  class_type = &Dtool_OdeHeightfieldGeom;
  //  break;

  case OdeGeom::GC_simple_space:
    geom = new OdeSimpleSpace((dSpaceID) _this->get_id());
    class_type = &Dtool_OdeSimpleSpace;
    break;

  case OdeGeom::GC_hash_space:
    geom = new OdeHashSpace((dSpaceID) _this->get_id());
    class_type = &Dtool_OdeHashSpace;
    break;

  case OdeGeom::GC_quad_tree_space:
    geom = new OdeQuadTreeSpace((dSpaceID) _this->get_id());
    class_type = &Dtool_OdeQuadTreeSpace;
    break;

  default:
    // This shouldn't happen, but if it does, we
    // should just return a regular OdeGeom or OdeSpace.
    if (_this->is_space()) {
      geom = new OdeSpace((dSpaceID) _this->get_id());
      class_type = &Dtool_OdeSpace;

    } else {
      geom = new OdeGeom(_this->get_id());
      class_type = &Dtool_OdeGeom;
    }
  }

  return DTool_CreatePyInstanceTyped((void *)geom, *class_type,
                                     true, false, geom->get_type_index());
}

#endif  // HAVE_PYTHON
