// Filename: dxDataContext8.cxx
// Created by:  drose (18Mar05)
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

#include "dxDataContext8.h"
#include "qpgeomVertexArrayData.h"
#include "qpgeomVertexArrayFormat.h"
#include "internalName.h"
#include "config_dxgsg8.h"
#include <d3dx8.h>

TypeHandle DXDataContext8::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXDataContext8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXDataContext8::
DXDataContext8(qpGeomVertexArrayData *data) :
  DataContext(data),
  _vbuffer(NULL)
{
  // Now fill in the FVF code.
  const qpGeomVertexArrayFormat *array_format = data->get_array_format();

  // We have to start with the vertex data, and work up from there in
  // order, since that's the way the FVF is defined.
  int n = 0;
  int num_data_types = array_format->get_num_data_types();

  _fvf = 0;
  
  if (n < num_data_types && 
      array_format->get_data_type(n)->get_name() == InternalName::get_vertex()) {
    _fvf |= D3DFVF_XYZ;
    ++n;
  }
  if (n < num_data_types && 
      array_format->get_data_type(n)->get_name() == InternalName::get_normal()) {
    _fvf |= D3DFVF_NORMAL;
    ++n;
  }
  if (n < num_data_types && 
      array_format->get_data_type(n)->get_name() == InternalName::get_color()) {
    _fvf |= D3DFVF_DIFFUSE;
    ++n;
  }

  // For multitexture support, we will need to look for all of the
  // texcoord names and enable them in order.
  if (n < num_data_types && 
      array_format->get_data_type(n)->get_name() == InternalName::get_texcoord()) {
    const qpGeomVertexDataType *data_type = array_format->get_data_type(n);
    switch (data_type->get_num_values()) {
    case 1:
      _fvf |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE1(0);
      ++n;
      break;
    case 2:
      _fvf |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0);
      ++n;
      break;
    case 3:
      _fvf |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
      ++n;
      break;
    case 4:
      _fvf |= D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE4(0);
      ++n;
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXDataContext8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXDataContext8::
~DXDataContext8() {
  if (_vbuffer != NULL) {
    RELEASE(_vbuffer, dxgsg8, "vertex buffer", RELEASE_ONCE);
    _vbuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXDataContext8::create_vbuffer
//       Access: Public
//  Description: Creates a new vertex buffer and uploads data to it.
////////////////////////////////////////////////////////////////////
void DXDataContext8::
create_vbuffer(DXScreenData &scrn) {
  if (_vbuffer != NULL) {
    RELEASE(_vbuffer, dxgsg8, "vertex buffer", RELEASE_ONCE);
    _vbuffer = NULL;
  }

  HRESULT hr = scrn.pD3DDevice->CreateVertexBuffer
    (get_data()->get_num_bytes(), D3DUSAGE_WRITEONLY,
     _fvf, D3DPOOL_MANAGED, &_vbuffer);
  if (FAILED(hr)) {
    dxgsg8_cat.warning()
      << "CreateVertexBuffer failed" << D3DERRORSTRING(hr);
    _vbuffer = NULL;
    
  } else {
    upload_data();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXDataContext8::upload_data
//       Access: Public
//  Description: Copies the latest data from the client store to
//               DirectX.
////////////////////////////////////////////////////////////////////
void DXDataContext8::
upload_data() {
  nassertv(_vbuffer != NULL);

  BYTE *local_pointer;
  HRESULT hr = _vbuffer->Lock(0, 0, &local_pointer, 0);
  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "VertexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return;
  }

  memcpy(local_pointer, get_data()->get_data(), get_data()->get_num_bytes());

  _vbuffer->Unlock();
}

