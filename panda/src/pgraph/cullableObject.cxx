// Filename: cullableObject.cxx
// Created by:  drose (04Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "cullableObject.h"
#include "textureAttrib.h"
#include "renderState.h"
#include "clockObject.h"


CullableObject *CullableObject::_deleted_chain = (CullableObject *)NULL;
int CullableObject::_num_ever_allocated = 0;
TypeHandle CullableObject::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::munge_geom
//       Access: Public
//  Description: Uses the indicated GeomMunger to transform the geom
//               and/or its vertices.
////////////////////////////////////////////////////////////////////
void CullableObject::
munge_geom(const qpGeomMunger *munger) {
  if (_geom != (Geom *)NULL) {
    // Temporary test and dcast until the experimental Geom rewrite
    // becomes the actual Geom rewrite.
    if (_geom->is_exact_type(qpGeom::get_class_type())) {
      _munger = munger;
      CPT(qpGeom) qpgeom = DCAST(qpGeom, _geom);
      qpgeom->munge_geom(munger, qpgeom, _munged_data);
      CPT(qpGeomVertexData) animated_vertices = 
        _munged_data->animate_vertices_cull();
#ifndef NDEBUG
      if (show_cpu_animation && animated_vertices != _munged_data) {
        // These vertices were CPU-animated, so flash them.
        static const double flash_rate = 1.0;  // 1 state change per second
        int cycle = (int)(ClockObject::get_global_clock()->get_frame_time() * flash_rate);
        if ((cycle & 3) == 0) {
          animated_vertices = animated_vertices->set_color(Colorf(0.8f, 0.2f, 0.2f, 1.0f));
          _state = _state->remove_attrib(TextureAttrib::get_class_type());
        } else if ((cycle & 3) == 2) {
          animated_vertices = animated_vertices->set_color(Colorf(0.1f, 0.2f, 0.8f, 1.0f));
          _state = _state->remove_attrib(TextureAttrib::get_class_type());
        }
      }
#endif
      _munged_data = animated_vertices;
      _geom = qpgeom;
    }
  }
  if (_next != (CullableObject *)NULL) {
    _next->munge_geom(munger);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Destructor
//       Access: Public
//  Description: Automatically deletes the whole chain of these things.
////////////////////////////////////////////////////////////////////
CullableObject::
~CullableObject() {
  if (_next != (CullableObject *)NULL) {
    delete _next;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void CullableObject::
output(ostream &out) const {
  if (_geom != (Geom *)NULL) {
    out << *_geom;
  } else {
    out << "(null)";
  }
}
