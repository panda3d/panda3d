// Filename: dxVertexBufferContext8.cxx
// Created by:  drose (18Mar05)
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

#include "dxVertexBufferContext8.h"
#include "geomVertexArrayData.h"
#include "geomVertexArrayFormat.h"
#include "graphicsStateGuardian.h"
#include "pStatTimer.h"
#include "internalName.h"
#include "config_dxgsg8.h"
#include "dxgsg8base.h"

TypeHandle DXVertexBufferContext8::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXVertexBufferContext8::
DXVertexBufferContext8(PreparedGraphicsObjects *pgo, GeomVertexArrayData *data) :
  VertexBufferContext(pgo, data),
  _vbuffer(NULL)
{
  // Now fill in the FVF code.
  const GeomVertexArrayFormat *array_format = data->get_array_format();

  // We have to start with the vertex data, and work up from there in
  // order, since that's the way the FVF is defined.
  int n = 0;
  int num_columns = array_format->get_num_columns();

  _fvf = 0;

  if (n < num_columns &&
      array_format->get_column(n)->get_name() == InternalName::get_vertex()) {
    ++n;

    int num_blend_values = 0;

    if (n < num_columns &&
        array_format->get_column(n)->get_name() == InternalName::get_transform_weight()) {
      // We have hardware vertex animation.
      num_blend_values = array_format->get_column(n)->get_num_values();
      ++n;
    }

    if (n < num_columns &&
        array_format->get_column(n)->get_name() == InternalName::get_transform_index()) {
      // Furthermore, it's indexed vertex animation.
      _fvf |= D3DFVF_LASTBETA_UBYTE4;
      ++num_blend_values;
      ++n;
    }

    switch (num_blend_values) {
    case 0:
      _fvf |= D3DFVF_XYZ;
      break;

    case 1:
      _fvf |= D3DFVF_XYZB1;
      break;

    case 2:
      _fvf |= D3DFVF_XYZB2;
      break;

    case 3:
      _fvf |= D3DFVF_XYZB3;
      break;

    case 4:
      _fvf |= D3DFVF_XYZB4;
      break;

    case 5:
      _fvf |= D3DFVF_XYZB5;
      break;
    }
  }

  if (n < num_columns &&
      array_format->get_column(n)->get_name() == InternalName::get_normal()) {
    _fvf |= D3DFVF_NORMAL;
    ++n;
  }
  if (n < num_columns &&
      array_format->get_column(n)->get_name() == InternalName::get_color()) {
    _fvf |= D3DFVF_DIFFUSE;
    ++n;
  }

  // Now look for all of the texcoord names and enable them in the
  // same order they appear in the array.
  int texcoord_index = 0;
  while (n < num_columns &&
         array_format->get_column(n)->get_contents() == Geom::C_texcoord) {
    const GeomVertexColumn *column = array_format->get_column(n);
    switch (column->get_num_values()) {
    case 1:
      _fvf |= D3DFVF_TEXCOORDSIZE1(texcoord_index);
      ++n;
      break;
    case 2:
      _fvf |= D3DFVF_TEXCOORDSIZE2(texcoord_index);
      ++n;
      break;
    case 3:
      _fvf |= D3DFVF_TEXCOORDSIZE3(texcoord_index);
      ++n;
      break;
    case 4:
      _fvf |= D3DFVF_TEXCOORDSIZE4(texcoord_index);
      ++n;
      break;
    }
    ++texcoord_index;
  }

  switch (texcoord_index) {
  case 0:
    break;
  case 1:
    _fvf |= D3DFVF_TEX1;
    break;
  case 2:
    _fvf |= D3DFVF_TEX2;
    break;
  case 3:
    _fvf |= D3DFVF_TEX3;
    break;
  case 4:
    _fvf |= D3DFVF_TEX4;
    break;
  case 5:
    _fvf |= D3DFVF_TEX5;
    break;
  case 6:
    _fvf |= D3DFVF_TEX6;
    break;
  case 7:
    _fvf |= D3DFVF_TEX7;
    break;
  case 8:
    _fvf |= D3DFVF_TEX8;
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXVertexBufferContext8::
~DXVertexBufferContext8() {
  if (_vbuffer != NULL) {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "deleting vertex buffer " << _vbuffer << "\n";
    }

    RELEASE(_vbuffer, dxgsg8, "vertex buffer", RELEASE_ONCE);
    _vbuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext8::evict_lru
//       Access: Public, Virtual
//  Description: Evicts the page from the LRU.  Called internally when
//               the LRU determines that it is full.  May also be
//               called externally when necessary to explicitly evict
//               the page.
//
//               It is legal for this method to either evict the page
//               as requested, do nothing (in which case the eviction
//               will be requested again at the next epoch), or
//               requeue itself on the tail of the queue (in which
//               case the eviction will be requested again much
//               later).
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext8::
evict_lru() {
  dequeue_lru();

  if (_vbuffer != NULL) {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "deleting vertex buffer " << _vbuffer << "\n";
    }

    RELEASE(_vbuffer, dxgsg8, "vertex buffer", RELEASE_ONCE);
    _vbuffer = NULL;
  }

  update_data_size_bytes(0);
  mark_unloaded();
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext8::create_vbuffer
//       Access: Public
//  Description: Creates a new vertex buffer (but does not upload data
//               to it).
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext8::
create_vbuffer(DXScreenData &scrn,
               const GeomVertexArrayDataHandle *reader) {
  nassertv(reader->get_object() == get_data());
  Thread *current_thread = reader->get_current_thread();

  if (_vbuffer != NULL) {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "deleting vertex buffer " << _vbuffer << "\n";
    }

    RELEASE(_vbuffer, dxgsg8, "vertex buffer", RELEASE_ONCE);
    _vbuffer = NULL;
  }

  PStatTimer timer(GraphicsStateGuardian::_create_vertex_buffer_pcollector,
                   current_thread);

  HRESULT hr = scrn._d3d_device->CreateVertexBuffer
//    (reader->get_data_size_bytes(), D3DUSAGE_WRITEONLY,
//     _fvf, D3DPOOL_MANAGED, &_vbuffer);
     (reader->get_data_size_bytes(), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
     _fvf, D3DPOOL_DEFAULT, &_vbuffer);
  if (FAILED(hr)) {
    dxgsg8_cat.warning()
      << "CreateVertexBuffer failed" << D3DERRORSTRING(hr);
    _vbuffer = NULL;
  } else {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "created vertex buffer " << _vbuffer << ": "
        << reader->get_num_rows() << " vertices "
        << *reader->get_array_format() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext8::upload_data
//       Access: Public
//  Description: Copies the latest data from the client store to
//               DirectX.
////////////////////////////////////////////////////////////////////
bool DXVertexBufferContext8::
upload_data(const GeomVertexArrayDataHandle *reader, bool force) {
  nassertr(reader->get_object() == get_data(), false);
  nassertr(_vbuffer != NULL, false);
  Thread *current_thread = reader->get_current_thread();

  const unsigned char *data_pointer = reader->get_read_pointer(force);
  if (data_pointer == NULL) {
    return false;
  }
  int data_size = reader->get_data_size_bytes();

  if (dxgsg8_cat.is_spam()) {
    dxgsg8_cat.spam()
      << "copying " << data_size
      << " bytes into vertex buffer " << _vbuffer << "\n";
  }

  PStatTimer timer(GraphicsStateGuardian::_load_vertex_buffer_pcollector,
                   current_thread);

  BYTE *local_pointer;
//  HRESULT hr = _vbuffer->Lock(0, data_size, &local_pointer, 0);
  HRESULT hr = _vbuffer->Lock(0, data_size, &local_pointer, D3DLOCK_DISCARD);
  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "VertexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return false;
  }

  GraphicsStateGuardian::_data_transferred_pcollector.add_level(data_size);
  memcpy(local_pointer, data_pointer, data_size);

  _vbuffer->Unlock();
  return true;
}

