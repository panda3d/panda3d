// Filename: vertexElementArray.cxx
// Created by: aignacio (Jan06)
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


#include "dxGraphicsStateGuardian9.h"
#include "vertexElementArray.h"


VertexElementArray::VertexElementArray (int maximum_vertex_elements)
{
  this -> offset = 0;
  this -> total_elements = 0;
  this -> maximum_vertex_elements = maximum_vertex_elements;
  this -> vertex_element_array = new DIRECT_3D_VERTEX_ELEMENT [maximum_vertex_elements];
  this -> vertex_element_type_array = new VERTEX_ELEMENT_TYPE [maximum_vertex_elements];

  memset (this -> vertex_element_type_counter_array, 0, VS_TOTAL_TYPES * sizeof (int));
}

VertexElementArray::~VertexElementArray ( )
{
  delete this -> vertex_element_array;
  delete this -> vertex_element_type_array;
}

int VertexElementArray::set_vertex_element_offset (int vertex_element_index, int offset)
{
  int state;
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;

  if (vertex_element_index >= 0 && vertex_element_index < this -> total_elements)
  {
    vertex_element = &this -> vertex_element_array [vertex_element_index];
    vertex_element -> Offset = offset;
    state = true;
  }

  return state;
}

void VertexElementArray::add_position_xyz_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_POSITION_XYZ;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT3;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_POSITION;
    vertex_element -> UsageIndex = 0;

    this -> offset += 12;
    this -> total_elements++;
  }
}

void VertexElementArray::add_position_xyzw_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_POSITION_XYZW;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT4;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_POSITION;
    vertex_element -> UsageIndex = 0;

    this -> offset += 16;
    this -> total_elements++;
  }
}

void VertexElementArray::add_normal_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_NORMAL;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT3;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_NORMAL;
    vertex_element -> UsageIndex = 0;

    this -> offset += 12;
    this -> total_elements++;
  }
}

void VertexElementArray::add_binormal_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_BINORMAL;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT3;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_BINORMAL;
    vertex_element -> UsageIndex = 0;

    this -> offset += 12;
    this -> total_elements++;
  }
}

void VertexElementArray::add_tangent_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_TANGENT;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT3;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_TANGENT;
    vertex_element -> UsageIndex = 0;

    this -> offset += 12;
    this -> total_elements++;
  }
}

void VertexElementArray::add_diffuse_color_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_DIFFUSE;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_D3DCOLOR;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_COLOR;
    vertex_element -> UsageIndex = 0;

    this -> offset += 4;
    this -> total_elements++;
  }
}

void VertexElementArray::add_specular_color_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_SPECULAR;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_D3DCOLOR;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_COLOR;
    vertex_element -> UsageIndex = 1;

    this -> offset += 4;
    this -> total_elements++;
  }
}

void VertexElementArray::add_u_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_TEXTURE_U;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT1;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_TEXCOORD;
    vertex_element -> UsageIndex = 0;

    this -> offset += 4;
    this -> total_elements++;
  }
}

void VertexElementArray::add_uv_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_TEXTURE_UV;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT2;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_TEXCOORD;
    vertex_element -> UsageIndex = 0;

    this -> offset += 8;
    this -> total_elements++;
  }
}

void VertexElementArray::add_uvw_vertex_element (int stream_index)
{
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_TEXTURE_UVW;
    vertex_element_type -> index = vertex_element_type_counter_array [vertex_element_type -> id];
    vertex_element_type_counter_array [vertex_element_type -> id]++;

    vertex_element -> Stream = stream_index;
    vertex_element -> Offset = this -> offset;
    vertex_element -> Type = D3DDECLTYPE_FLOAT3;
    vertex_element -> Method = D3DDECLMETHOD_DEFAULT;

    vertex_element -> Usage = D3DDECLUSAGE_TEXCOORD;
    vertex_element -> UsageIndex = 0;

    this -> offset += 12;
    this -> total_elements++;
  }
}

int VertexElementArray::add_end_vertex_element (void)
{
  int add;
  DIRECT_3D_VERTEX_ELEMENT *vertex_element;
  VERTEX_ELEMENT_TYPE *vertex_element_type;

  add = FALSE;
  if (this -> total_elements < this -> maximum_vertex_elements)
  {
    vertex_element = &this -> vertex_element_array [this -> total_elements];
    memset (vertex_element, 0, sizeof (DIRECT_3D_VERTEX_ELEMENT));

    vertex_element_type = &this -> vertex_element_type_array [this -> total_elements];
    memset (vertex_element_type, 0, sizeof (VERTEX_ELEMENT_TYPE));

    vertex_element_type -> id = VS_END;

    vertex_element -> Stream = 0xFF;
    vertex_element -> Type = D3DDECLTYPE_UNUSED;

    add = TRUE;
  }

  return add;
}

