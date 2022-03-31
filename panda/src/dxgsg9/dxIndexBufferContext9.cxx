/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dxIndexBufferContext9.cxx
 * @author drose
 * @date 2005-03-18
 */

#include "dxIndexBufferContext9.h"
#include "geomPrimitive.h"
#include "config_dxgsg9.h"
#include "graphicsStateGuardian.h"
#include "pStatTimer.h"

#define DEBUG_INDEX_BUFFER false

TypeHandle DXIndexBufferContext9::_type_handle;

/**
 *
 */
DXIndexBufferContext9::
DXIndexBufferContext9(PreparedGraphicsObjects *pgo, GeomPrimitive *data) :
  IndexBufferContext(pgo, data),
  _ibuffer(nullptr),
  _managed(-1) {
}

/**
 *
 */
DXIndexBufferContext9::
~DXIndexBufferContext9() {
  free_ibuffer();
}

/**
 * Evicts the page from the LRU.  Called internally when the LRU determines
 * that it is full.  May also be called externally when necessary to
 * explicitly evict the page.
 *
 * It is legal for this method to either evict the page as requested, do
 * nothing (in which case the eviction will be requested again at the next
 * epoch), or requeue itself on the tail of the queue (in which case the
 * eviction will be requested again much later).
 */
void DXIndexBufferContext9::
evict_lru() {
  dequeue_lru();
  free_ibuffer();
  update_data_size_bytes(0);
  mark_unloaded();
}

/**
 * Free index buffer.
 */
void DXIndexBufferContext9::
free_ibuffer(void) {
  if (_ibuffer != nullptr) {
    if (DEBUG_INDEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "deleting index buffer " << _ibuffer << "\n";
    }

    if (DEBUG_INDEX_BUFFER) {
      RELEASE(_ibuffer, dxgsg9, "index buffer", RELEASE_ONCE);
    } else {
      _ibuffer->Release();
    }

    _ibuffer = nullptr;
  }
}

/**
 * Allocates index buffer memory.
 */
void DXIndexBufferContext9::
allocate_ibuffer(DXScreenData &scrn,
                 const GeomPrimitivePipelineReader *reader) {

  D3DFORMAT index_type =
    DXGraphicsStateGuardian9::get_index_type(reader->get_index_type());

  int data_size;
  DWORD usage;
  D3DPOOL pool;

  data_size = reader->get_data_size_bytes();

  if (reader->get_index_type() == GeomEnums::NT_uint8) {
    // We widen 8-bits indices to 16-bits.
    data_size *= 2;
  }

  _managed = scrn._managed_index_buffers;
  if (_managed)
  {
    usage = D3DUSAGE_WRITEONLY;
    pool = D3DPOOL_MANAGED;
  }
  else
  {
    usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    pool = D3DPOOL_DEFAULT;
  }

  int attempts;
  HRESULT hr;

  attempts = 0;
  do
  {
     hr = scrn._d3d_device->CreateIndexBuffer
      (data_size, usage, index_type, pool, &_ibuffer, nullptr);
     attempts++;
  }
  while (scrn._dxgsg9 -> check_dx_allocation (hr, data_size, attempts));

  if (FAILED(hr)) {
    dxgsg9_cat.warning()
      << "CreateIndexBuffer failed" << D3DERRORSTRING(hr);
    _ibuffer = nullptr;
  } else {
    if (DEBUG_INDEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "creating index buffer " << _ibuffer << ": "
        << reader->get_num_vertices() << " indices ("
        << reader->get_index_type() << ")\n";
    }
  }
}

/**
 * Creates a new index buffer (but does not upload data to it).
 */
void DXIndexBufferContext9::
create_ibuffer(DXScreenData &scrn,
               const GeomPrimitivePipelineReader *reader) {
  nassertv(reader->get_object() == get_data());
  Thread *current_thread = reader->get_current_thread();

  free_ibuffer();

  PStatTimer timer(GraphicsStateGuardian::_create_index_buffer_pcollector,
                   current_thread);

  allocate_ibuffer(scrn, reader);
}

/**
 * Copies the latest data from the client store to DirectX.
 */
bool DXIndexBufferContext9::
upload_data(const GeomPrimitivePipelineReader *reader, bool force) {
  nassertr(reader->get_object() == get_data(), false);
  Thread *current_thread = reader->get_current_thread();

  nassertr(_ibuffer != nullptr, false);

  const unsigned char *data_pointer = reader->get_read_pointer(force);
  if (data_pointer == nullptr) {
    return false;
  }
  size_t data_size = (size_t)reader->get_data_size_bytes();

  if (reader->get_index_type() == GeomEnums::NT_uint8) {
    // We widen 8-bits indices to 16-bits.
    data_size *= 2;
  }

  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "copying " << data_size
      << " bytes into index buffer " << _ibuffer << "\n";
  }
  PStatTimer timer(GraphicsStateGuardian::_load_index_buffer_pcollector,
                   current_thread);

  HRESULT hr;
  BYTE *local_pointer;

  if (_managed) {
    hr = _ibuffer->Lock(0, data_size, (void **) &local_pointer, 0);
  } else {
    hr = _ibuffer->Lock(0, data_size, (void **) &local_pointer, D3DLOCK_DISCARD);
  }
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "IndexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return false;
  }

  GraphicsStateGuardian::_data_transferred_pcollector.add_level(data_size);

  if (reader->get_index_type() == GeomEnums::NT_uint8) {
    // Widen to 16-bits, as DirectX doesn't support 8-bits indices.
    uint16_t *ptr = (uint16_t *)local_pointer;
    for (size_t i = 0; i < data_size; i += 2) {
      *ptr++ = (uint16_t)*data_pointer++;
    }
  } else {
    memcpy(local_pointer, data_pointer, data_size);
  }

  _ibuffer->Unlock();
  return true;
}
