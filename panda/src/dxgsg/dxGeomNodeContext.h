// Filename: dxGeomNodeContext.h
// Created by:  drose (12Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef DXGEOMNODECONTEXT_H
#define DXGEOMNODECONTEXT_H

#include <pandabase.h>

#ifdef WIN32_VC
// Must include windows.h before dx.h on NT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#endif

#include <geomNodeContext.h>
#include <geomNode.h>
#include "pvector.h"

#define D3D_OVERLOADS   //  get D3DVECTOR '+' operator, etc from d3dtypes.h
#include <d3d.h>

typedef struct {
     DWORD nVerts;
     D3DPRIMITIVETYPE primtype;
} DPInfo;

////////////////////////////////////////////////////////////////////
//       Class : DXGeomNodeContext
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX DXGeomNodeContext : public GeomNodeContext {
public:
  INLINE DXGeomNodeContext(GeomNode *node);
  ~DXGeomNodeContext();

  // A list of the dynamic Geoms within the GeomNode; these aren't
  // part of the above display list and must be drawn separately
  typedef pvector<PT(dDrawable) > Geoms;
  Geoms _other_geoms;

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
    register_type(_type_handle, "DXGeomNodeContext",
                  GeomNodeContext::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "dxGeomNodeContext.I"

#endif

