// Filename: vertexElementArray.cxx
// Created by: aignacio (Jan06)
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

#include "dxGraphicsStateGuardian9.h"
#include "vertexElementArray.h"


VertexElementArray::VertexElementArray(int maximum_vertex_elements) {
  _total_elements = 0;
  _maximum_vertex_elements = maximum_vertex_elements;

  _vertex_element_array = new D3DVERTEXELEMENT9[maximum_vertex_elements];
  memset(_vertex_element_array, 0, sizeof(D3DVERTEXELEMENT9) * maximum_vertex_elements);
}

VertexElementArray::~VertexElementArray() {
  delete _vertex_element_array;
}

void VertexElementArray::add_position_xyz_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT3;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_POSITION;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_position_xyzw_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT4;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_POSITION;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_normal_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT3;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_NORMAL;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_binormal_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT3;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_BINORMAL;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_tangent_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT3;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_TANGENT;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_diffuse_color_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_D3DCOLOR;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_COLOR;
  vertex_element->UsageIndex = 0;

  _total_elements++;
}

void VertexElementArray::add_specular_color_vertex_element(int stream_index, int offset) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_D3DCOLOR;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_COLOR;
  vertex_element->UsageIndex = 1;

  _total_elements++;
}

void VertexElementArray::add_u_vertex_element(int stream_index, int offset, int texture_stage) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT1;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_TEXCOORD;
  vertex_element->UsageIndex = texture_stage;

  _total_elements++;
}

void VertexElementArray::add_uv_vertex_element(int stream_index, int offset, int texture_stage) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT2;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_TEXCOORD;
  vertex_element->UsageIndex = texture_stage;

  _total_elements++;
}

void VertexElementArray::add_uvw_vertex_element(int stream_index, int offset, int texture_stage) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT3;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_TEXCOORD;
  vertex_element->UsageIndex = texture_stage;

  _total_elements++;
}

void VertexElementArray::add_xyzw_vertex_element(int stream_index, int offset, int texture_stage) {
  if (_total_elements >= _maximum_vertex_elements) {
    return;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = stream_index;
  vertex_element->Offset = offset;
  vertex_element->Type = D3DDECLTYPE_FLOAT4;
  vertex_element->Method = D3DDECLMETHOD_DEFAULT;

  vertex_element->Usage = D3DDECLUSAGE_TEXCOORD;
  vertex_element->UsageIndex = texture_stage;

  _total_elements++;
}

bool VertexElementArray::add_end_vertex_element(void) {
  if (_total_elements >= _maximum_vertex_elements) {
    return false;
  }

  LPD3DVERTEXELEMENT9 vertex_element = &_vertex_element_array[_total_elements];

  vertex_element->Stream = 0xFF;
  vertex_element->Offset = 0;
  vertex_element->Type = D3DDECLTYPE_UNUSED;
  vertex_element->Method = 0;

  vertex_element->Usage = 0;
  vertex_element->UsageIndex = 0;

  return true;
}

