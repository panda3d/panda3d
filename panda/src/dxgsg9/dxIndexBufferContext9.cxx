// Filename: dxIndexBufferContext9.cxx
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

#include "dxIndexBufferContext9.h"
#include "geomPrimitive.h"
#include "config_dxgsg9.h"
#include "graphicsStateGuardian.h"
#include "pStatTimer.h"
#include <d3dx9.h>

#define DEBUG_INDEX_BUFFER false

TypeHandle DXIndexBufferContext9::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXIndexBufferContext9::
DXIndexBufferContext9(GeomPrimitive *data) :
  IndexBufferContext(data),
  _ibuffer(NULL)
{
  _managed = -1;
  _lru_page = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXIndexBufferContext9::
~DXIndexBufferContext9() {

  this -> free_ibuffer ( );

  if (_lru_page)
  {
    _lru_page -> _m.lru -> remove_page (_lru_page);
    _lru_page -> _m.lru -> free_page (_lru_page);
    _lru_page = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::free_ibuffer
//       Access: Public
//  Description: Free index buffer.
////////////////////////////////////////////////////////////////////
void DXIndexBufferContext9::
free_ibuffer(void) {
  if (_ibuffer != NULL) {
    if (DEBUG_INDEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "deleting index buffer " << _ibuffer << "\n";
    }

    if (DEBUG_INDEX_BUFFER)
    {
      RELEASE(_ibuffer, dxgsg9, "index buffer", RELEASE_ONCE);
    }
    else
    {
      _ibuffer -> Release ( );
    }

    _ibuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::allocate_ibuffer
//       Access: Public
//  Description: Allocates index buffer memory.
////////////////////////////////////////////////////////////////////
void DXIndexBufferContext9::
allocate_ibuffer(DXScreenData &scrn) {

  D3DFORMAT index_type =
    DXGraphicsStateGuardian9::get_index_type(get_data()->get_index_type());

  int data_size;
  DWORD usage;
  D3DPOOL pool;

  data_size = get_data()->get_data_size_bytes();

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

  HRESULT hr = scrn._d3d_device->CreateIndexBuffer
    (data_size, usage, index_type, pool, &_ibuffer, NULL);

  if (FAILED(hr)) {
    dxgsg9_cat.warning()
      << "CreateIndexBuffer failed" << D3DERRORSTRING(hr);
    _ibuffer = NULL;
  } else {
    if (DEBUG_INDEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "creating index buffer " << _ibuffer << ": "
        << get_data()->get_num_vertices() << " indices ("
        << get_data()->get_vertices()->get_array_format()->get_column(0)->get_numeric_type()
        << ")\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::create_ibuffer
//       Access: Public
//  Description: Creates a new index buffer (but does not upload data
//               to it).
////////////////////////////////////////////////////////////////////
void DXIndexBufferContext9::
create_ibuffer(DXScreenData &scrn) {

  this -> free_ibuffer ( );

  if (_lru_page)
  {
    _lru_page -> _m.lru -> remove_page (_lru_page);
    _lru_page -> _m.lru -> free_page (_lru_page);
    _lru_page = 0;
  }

  PStatTimer timer(GraphicsStateGuardian::_create_index_buffer_pcollector);

  int data_size;

  data_size = get_data()->get_data_size_bytes();

  this -> allocate_ibuffer(scrn);

  if (_ibuffer)
  {
    if (_managed == false)
    {
      Lru *lru;

      lru = scrn._dxgsg9 -> _lru;
      if (lru)
      {
        LruPage *lru_page;

        lru_page = lru -> allocate_page (data_size);
        if (lru_page)
        {
          lru_page -> _m.type = GPT_IndexBuffer;
          lru_page -> _m.lru_page_type.pointer = this;

          lru -> add_cached_page (LPP_New, lru_page);
          _lru_page = lru_page;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXIndexBufferContext9::upload_data
//       Access: Public
//  Description: Copies the latest data from the client store to
//               DirectX.
////////////////////////////////////////////////////////////////////
void DXIndexBufferContext9::
upload_data() {
  nassertv(_ibuffer != NULL);
  PStatTimer timer(GraphicsStateGuardian::_load_index_buffer_pcollector);

  int data_size = get_data()->get_data_size_bytes();

  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "copying " << data_size
      << " bytes into index buffer " << _ibuffer << "\n";
  }

  HRESULT hr;
  BYTE *local_pointer;

  if (_managed)
  {
    hr = _ibuffer->Lock(0, data_size, (void **) &local_pointer, 0);
  }
  else
  {
    hr = _ibuffer->Lock(0, data_size, (void **) &local_pointer, D3DLOCK_DISCARD);
  }
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "IndexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return;
  }

  GraphicsStateGuardian::_data_transferred_pcollector.add_level(data_size);
  memcpy(local_pointer, get_data()->get_data(), data_size);

  _ibuffer->Unlock();
}
