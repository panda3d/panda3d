// Filename: dxGeomNodeContext7.h
// Created by:  drose (12Jun01)
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

#ifndef DXGEOMNODECONTEXT7_H
#define DXGEOMNODECONTEXT7_H

#include "dxgsg7base.h"

#include "geomNodeContext.h"
#include "geomNode.h"
#include "pvector.h"

typedef struct {
     DWORD nVerts;
     D3DPRIMITIVETYPE primtype;
} DPInfo;

////////////////////////////////////////////////////////////////////
//       Class : DXGeomNodeContext7
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGeomNodeContext7 : public GeomNodeContext {
public:
  INLINE DXGeomNodeContext7(GeomNode *node);
  ~DXGeomNodeContext7();

  // A list of the dynamic Geoms within the GeomNode; these aren't
  // part of the above display list and must be drawn separately
  typedef pvector< PT(Geom) > Geoms;
  Geoms _cached_geoms,_other_geoms;

  // VB's are already reference-counted by D3D, no need to make separate panda object to do that
  // but we will want a way to know if VB has already been xformed by ProcessVerts this frame
  // if multiple geomnodes share VBs

  LPDIRECT3DVERTEXBUFFER7 _pVB;
  LPDIRECT3DVERTEXBUFFER7 _pXformed_VB;

  int _start_index;   // starting offset of this geom's verts within the VB
  int _num_verts;     // number of verts used by this geomcontext within the VB
  
  BYTE *_pEndofVertData;  // ptr to end of current vert data in VB (note: only used/valid during setup)

  // for multiple geoms per VB, either will have to regen lengths based on per* flags, or store
  // per geom vector of perprim vectors lengths
  vector<DPInfo> _PrimInfo;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNodeContext::init_type();
    register_type(_type_handle, "DXGeomNodeContext7",
                  GeomNodeContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGeomNodeContext7.I"

#endif

