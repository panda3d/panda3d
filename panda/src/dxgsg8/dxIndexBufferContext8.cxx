// Filename: dxIndexBufferContext8.cxx
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

#include "dxIndexBufferContext8.h"
#include "geomPrimitive.h"
#include "config_dxgsg8.h"
#include "graphicsStateGuardian.h"
#include "pStatTimer.h"
#include "dxgsg8base.h"

TypeHandle DXIndexBufferContext8::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXIndexBufferContext8::
DXIndexBufferContext8(PreparedGraphicsObjects *pgo, GeomPrimitive *data) :
  IndexBufferContext(pgo, data),
  _ibuffer(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext8::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXIndexBufferContext8::
~DXIndexBufferContext8() {
  if (_ibuffer != NULL) {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "deleting index buffer " << _ibuffer << "\n";
    }

    RELEASE(_ibuffer, dxgsg8, "index buffer", RELEASE_ONCE);
    _ibuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext8::evict_lru
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
void DXIndexBufferContext8::
evict_lru() {
  dequeue_lru();

  if (_ibuffer != NULL) {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "deleting index buffer " << _ibuffer << "\n";
    }

    RELEASE(_ibuffer, dxgsg8, "index buffer", RELEASE_ONCE);
    _ibuffer = NULL;
  }

  update_data_size_bytes(0);
  mark_unloaded();
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext8::create_ibuffer
//       Access: Public
//  Description: Creates a new index buffer (but does not upload data
//               to it).
////////////////////////////////////////////////////////////////////
void DXIndexBufferContext8::
create_ibuffer(DXScreenData &scrn, 
               const GeomPrimitivePipelineReader *reader) {
  nassertv(reader->get_object() == get_data());
  Thread *current_thread = reader->get_current_thread();

  if (_ibuffer != NULL) {
    RELEASE(_ibuffer, dxgsg8, "index buffer", RELEASE_ONCE);
    _ibuffer = NULL;
  }

  PStatTimer timer(GraphicsStateGuardian::_create_index_buffer_pcollector,
                   current_thread);

  D3DFORMAT index_type =
    DXGraphicsStateGuardian8::get_index_type(reader->get_index_type());

  HRESULT hr = scrn._d3d_device->CreateIndexBuffer
//    (reader->get_data_size_bytes(), D3DUSAGE_WRITEONLY,
//     index_type, D3DPOOL_MANAGED, &_ibuffer);
    (reader->get_data_size_bytes(), D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
     index_type, D3DPOOL_DEFAULT, &_ibuffer);
  if (FAILED(hr)) {
    dxgsg8_cat.warning()
      << "CreateIndexBuffer failed" << D3DERRORSTRING(hr);
    _ibuffer = NULL;
  } else {
    if (dxgsg8_cat.is_debug()) {
      dxgsg8_cat.debug()
        << "creating index buffer " << _ibuffer << ": "
        << reader->get_num_vertices() << " indices ("
        << reader->get_vertices_reader()->get_array_format()->get_column(0)->get_numeric_type()
        << ")\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext8::upload_data
//       Access: Public
//  Description: Copies the latest data from the client store to
//               DirectX.
////////////////////////////////////////////////////////////////////
bool DXIndexBufferContext8::
upload_data(const GeomPrimitivePipelineReader *reader, bool force) {
  nassertr(reader->get_object() == get_data(), false);
  Thread *current_thread = reader->get_current_thread();

  nassertr(_ibuffer != NULL, false);

  const unsigned char *data_pointer = reader->get_read_pointer(force);
  if (data_pointer == NULL) {
    return false;
  }
  int data_size = reader->get_data_size_bytes();

  if (dxgsg8_cat.is_spam()) {
    dxgsg8_cat.spam()
      << "copying " << data_size
      << " bytes into index buffer " << _ibuffer << "\n";
  }

  PStatTimer timer(GraphicsStateGuardian::_load_index_buffer_pcollector,
                   current_thread);

  BYTE *local_pointer;
//  HRESULT hr = _ibuffer->Lock(0, data_size, &local_pointer, 0);
  HRESULT hr = _ibuffer->Lock(0, data_size, &local_pointer, D3DLOCK_DISCARD);
  if (FAILED(hr)) {
    dxgsg8_cat.error()
      << "IndexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return false;
  }

  GraphicsStateGuardian::_data_transferred_pcollector.add_level(data_size);
  memcpy(local_pointer, data_pointer, data_size);

  _ibuffer->Unlock();
  return true;
}

