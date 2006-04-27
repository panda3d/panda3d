// Filename: dxVertexBufferContext9.cxx
// Created by:  drose (18Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
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

#include "dxVertexBufferContext9.h"
#include "geomVertexArrayData.h"
#include "geomVertexArrayFormat.h"
#include "graphicsStateGuardian.h"
#include "pStatTimer.h"
#include "internalName.h"
#include "config_dxgsg9.h"

#define DEBUG_VERTEX_BUFFER false

TypeHandle DXVertexBufferContext9::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXVertexBufferContext9::
DXVertexBufferContext9(PreparedGraphicsObjects *pgo, GeomVertexArrayData *data, DXScreenData &scrn) :
  VertexBufferContext(pgo, data),
  _vbuffer(NULL)
{
  // Now fill in the FVF code.
  const GeomVertexArrayFormat *array_format = data->get_array_format();

  // We have to start with the vertex data, and work up from there in
  // order, since that's the way the FVF is defined.
  int index;
  int n = 0;
  int num_columns = array_format->get_num_columns();

  _vertex_element_type_array = 0;

  VERTEX_ELEMENT_TYPE *vertex_element_type_array;

//  if (scrn._dxgsg9 -> _current_shader_context)
  {
    int total_elements;
    unsigned char vertex_element_type_counter_array [VS_TOTAL_TYPES];
    VERTEX_ELEMENT_TYPE *vertex_element_type;

    total_elements = num_columns + 2;
    vertex_element_type_array = new VERTEX_ELEMENT_TYPE [total_elements];
    memset (vertex_element_type_array, 0, total_elements * sizeof (VERTEX_ELEMENT_TYPE));
    memset (vertex_element_type_counter_array, 0, sizeof (vertex_element_type_counter_array));

    // create a simple vertex type mapping from the vertex elements
    vertex_element_type = vertex_element_type_array;
    for (index = 0; index < num_columns; index++)
    {
      int num_values;
      const InternalName *name;

      name = array_format -> get_column (index) -> get_name ( );
      num_values = array_format -> get_column(index) -> get_num_values ( );

      if (false) {

      } else if (name -> get_top ( ) == InternalName::get_vertex ( )) {

        switch (num_values)
        {
          case 3:
            vertex_element_type -> vs_input_type = VS_POSITION_XYZ;
            break;
          case 4:
            vertex_element_type -> vs_input_type = VS_POSITION_XYZW;
            break;
          default:
            dxgsg9_cat.warning ( ) << "VERTEX ERROR: invalid number of position coordinate elements " << num_values << "\n";
            break;
        }

      } else if (name -> get_top ( ) == InternalName::get_texcoord ( )) {

        switch (num_values)
        {
          case 1:
            vertex_element_type -> vs_input_type = VS_TEXTURE_U;
            break;
          case 2:
            vertex_element_type -> vs_input_type = VS_TEXTURE_UV;
            break;
          case 3:
            vertex_element_type -> vs_input_type = VS_TEXTURE_UVW;
            break;
          default:
            dxgsg9_cat.warning ( ) << "VERTEX ERROR: invalid number of vertex texture coordinate elements " << num_values << "\n";
            break;
        }

      } else if (name -> get_top ( ) == InternalName::get_normal ( )) {

        vertex_element_type -> vs_input_type = VS_NORMAL;

      } else if (name -> get_top ( ) == InternalName::get_binormal ( )) {

        vertex_element_type -> vs_input_type = VS_BINORMAL;

      } else if (name -> get_top ( ) == InternalName::get_tangent ( )) {

        vertex_element_type -> vs_input_type = VS_TANGENT;

      } else if (name -> get_top ( ) == InternalName::get_color ( )) {

        vertex_element_type -> vs_input_type = VS_DIFFUSE;

      } else {

        dxgsg9_cat.error ( )
          << "VERTEX ERROR: unsupported vertex element " << name -> get_name ( )
          << "\n";

        vertex_element_type -> vs_input_type = VS_ERROR;
      }

      vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> vs_input_type];
      vertex_element_type_counter_array [vertex_element_type -> vs_input_type]++;

  // SHADER ISSUE: STREAM INDEX ALWAYS 0 FOR VERTEX BUFFER ???
      vertex_element_type -> stream = 0;
      vertex_element_type -> offset = array_format -> get_column(index) -> get_start ( );

      DBG_VEA  dxgsg9_cat.debug()
        << "INFO VertexElementArray " << index
        << " " << name -> get_name ( )
        << " VS INPUT TYPE " << vertex_element_type -> vs_input_type
        << " index " << vertex_element_type -> index
        << " offset " << vertex_element_type -> offset
        << "\n";
      DBG_E

      vertex_element_type++;
    }

    DBG_VEA  dxgsg9_cat.debug()
      << "INFO stride " << array_format -> get_stride ( )
      << " total bytes " << array_format-> get_total_bytes ( )
      << "\n";
    DBG_E
  }

  _vertex_element_type_array = vertex_element_type_array;

  _fvf = 0;
  _managed = -1;
  _lru_page = 0;

  _direct_3d_vertex_declaration = 0;
  _shader_context = 0;

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
//     Function: DXVertexBufferContext9::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXVertexBufferContext9::
~DXVertexBufferContext9() {

  if (_vertex_element_type_array) {
    delete _vertex_element_type_array;
    _vertex_element_type_array = 0;
  }
  if (_direct_3d_vertex_declaration) {
    _direct_3d_vertex_declaration -> Release ( );
    _direct_3d_vertex_declaration = 0;
  }

  free_vbuffer ( );

  if (_lru_page) {
    _lru_page -> _m.lru -> remove_page (_lru_page);
    _lru_page -> _m.lru -> free_page (_lru_page);
    _lru_page = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext9::free_vbuffer
//       Access: Public
//  Description: Frees vertex buffer memory.
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext9::
free_vbuffer(void) {

  if (_vbuffer != NULL) {
    if (DEBUG_VERTEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "deleting vertex buffer " << _vbuffer << "\n";
    }

    if (DEBUG_VERTEX_BUFFER) {
      RELEASE(_vbuffer, dxgsg9, "vertex buffer", RELEASE_ONCE);
    }
    else {
      _vbuffer -> Release ( );
    }

    _vbuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext9::allocate_vbuffer
//       Access: Public
//  Description: Allocates vertex buffer memory.
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext9::
allocate_vbuffer(DXScreenData &scrn,
                 const GeomVertexArrayDataPipelineReader *reader) {

  int data_size;
  HRESULT hr;
  DWORD usage;
  D3DPOOL pool;

  data_size = reader->get_data_size_bytes();

  _managed = scrn._managed_vertex_buffers;
  if (_managed) {
    pool = D3DPOOL_MANAGED;
    usage = D3DUSAGE_WRITEONLY;
  }
  else {
    pool = D3DPOOL_DEFAULT;
    usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
  }

  int attempts;

  attempts = 0;
  do
  {
    hr = scrn._d3d_device->CreateVertexBuffer
        (data_size, usage, _fvf, pool, &_vbuffer, NULL);
    attempts++;
  }
  while (scrn._dxgsg9 -> check_dx_allocation (hr, data_size, attempts));

  if (FAILED(hr)) {
    dxgsg9_cat.warning()
      << "CreateVertexBuffer failed" << D3DERRORSTRING(hr);
    _vbuffer = NULL;
  } else {
    if (DEBUG_VERTEX_BUFFER && dxgsg9_cat.is_debug()) {
      dxgsg9_cat.debug()
        << "created vertex buffer " << _vbuffer << ": "
        << reader->get_num_rows() << " vertices "
        << *reader->get_array_format() << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext9::create_vbuffer
//       Access: Public
//  Description: Creates a new vertex buffer (but does not upload data
//               to it).
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext9::
create_vbuffer(DXScreenData &scrn,
               const GeomVertexArrayDataPipelineReader *reader) {
  nassertv(reader->get_object() == get_data());
  Thread *current_thread = reader->get_current_thread();

  free_vbuffer ( );

  if (_lru_page) {
    _lru_page -> _m.lru -> remove_page (_lru_page);
    _lru_page -> _m.lru -> free_page (_lru_page);
    _lru_page = 0;
  }

  PStatTimer timer(GraphicsStateGuardian::_create_vertex_buffer_pcollector,
                   current_thread);

  int data_size;

  data_size = reader->get_data_size_bytes();

  this -> allocate_vbuffer(scrn, reader);

  if (_vbuffer) {
    if (_managed == false) {
      Lru *lru;

      lru = scrn._dxgsg9 -> _lru;
      if (lru) {
        LruPage *lru_page;

        lru_page = lru -> allocate_page (data_size);
        if (lru_page) {
          lru_page -> _m.v.type = GPT_VertexBuffer;
          lru_page -> _m.lru_page_type.pointer = this;

          lru -> add_cached_page (LPP_New, lru_page);
          _lru_page = lru_page;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXVertexBufferContext9::upload_data
//       Access: Public
//  Description: Copies the latest data from the client store to
//               DirectX.
////////////////////////////////////////////////////////////////////
void DXVertexBufferContext9::
upload_data(const GeomVertexArrayDataPipelineReader *reader) {
  nassertv(reader->get_object() == get_data());
  nassertv(_vbuffer != NULL);
  Thread *current_thread = reader->get_current_thread();
  PStatTimer timer(GraphicsStateGuardian::_load_vertex_buffer_pcollector,
                   current_thread);

  int data_size = reader->get_data_size_bytes();

  if (dxgsg9_cat.is_spam()) {
    dxgsg9_cat.spam()
      << "copying " << data_size
      << " bytes into vertex buffer " << _vbuffer << "\n";
  }

  HRESULT hr;
  BYTE *local_pointer;

  if (_managed) {
    hr = _vbuffer->Lock(0, data_size, (void **) &local_pointer, 0);
  }
  else {
    hr = _vbuffer->Lock(0, data_size, (void **) &local_pointer, D3DLOCK_DISCARD);
  }
  if (FAILED(hr)) {
    dxgsg9_cat.error()
      << "VertexBuffer::Lock failed" << D3DERRORSTRING(hr);
    return;
  }

  GraphicsStateGuardian::_data_transferred_pcollector.add_level(data_size);
  memcpy(local_pointer, reader->get_data(), data_size);

  _vbuffer->Unlock();
}
