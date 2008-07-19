// Filename: vertexElementArray.h
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

#ifndef VERTEX_ELEMENT_ARRAY_H
#define VERTEX_ELEMENT_ARRAY_H


enum
{
  VS_END = 0,

  VS_POSITION_XYZ,
  VS_POSITION_XYZW,
  VS_NORMAL,
  VS_DIFFUSE,
  VS_SPECULAR,
  VS_TEXTURE_U,
  VS_TEXTURE_UV,
  VS_TEXTURE_UVW,
  VS_TANGENT,
  VS_BINORMAL,

  VS_ERROR,

  VS_TOTAL_TYPES
};

typedef struct
{
  int vs_input_type; // this is VS_XXXXX from the enum above
  int index;
  int stream;
  int offset;
}
VERTEX_ELEMENT_TYPE;

////////////////////////////////////////////////////////////////////
//       Class : VertexElementArray
// Description : This class gives the ability for a user-friendly way
//               of creating a vertex declaration for DirectX 9.
//               Since a vertex shader has a fixed input, the vertex
//               element array can be cached so that a new vertex
//               declaration for different vertex buffers can be
//               quickly created.  Be sure to call
//               add_end_vertex_element ( ) when finished creating a
//               vertex element array.
//               VERTEX_ELEMENT_TYPE is used for a simplified mapping
//               of vertex buffer data to vertex shader inputs.
//               This class is used with DXShaderContext9 and in
//               conjunction with DXVertexBufferContext9.
////////////////////////////////////////////////////////////////////
class VertexElementArray
{
public:

  VertexElementArray (int maximum_vertex_elements);
  ~VertexElementArray ( );

  int set_vertex_element_offset (int vertex_element_index, int offset);
  void set_vs_input_type (int vs_input_type, VERTEX_ELEMENT_TYPE *vertex_element_type);

  void add_position_xyz_vertex_element (int stream_index);
  void add_position_xyzw_vertex_element (int stream_index);
  void add_normal_vertex_element (int stream_index);
  void add_binormal_vertex_element (int stream_index);
  void add_tangent_vertex_element (int stream_index);
  void add_diffuse_color_vertex_element (int stream_index);
  void add_specular_color_vertex_element (int stream_index);
  void add_u_vertex_element (int stream_index);
  void add_uv_vertex_element (int stream_index);
  void add_uvw_vertex_element (int stream_index);
  int add_end_vertex_element (void);

  int offset;
  int total_elements;
  int maximum_vertex_elements;
  int total_texture_coordinate_elements;
  int vertex_element_type_counter_array [VS_TOTAL_TYPES];
  DIRECT_3D_VERTEX_ELEMENT *vertex_element_array;
  VERTEX_ELEMENT_TYPE *vertex_element_type_array;
};

#endif
