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

  void add_position_xyz_vertex_element (int stream_index, int offset);
  void add_position_xyzw_vertex_element (int stream_index, int offset);

  void add_normal_vertex_element (int stream_index, int offset);
  void add_binormal_vertex_element (int stream_index, int offset);
  void add_tangent_vertex_element (int stream_index, int offset);

  void add_diffuse_color_vertex_element (int stream_index, int offset);
  void add_specular_color_vertex_element (int stream_index, int offset);

  void add_u_vertex_element (int stream_index, int offset, int texture_stage);
  void add_uv_vertex_element (int stream_index, int offset, int texture_stage);
  void add_uvw_vertex_element (int stream_index, int offset, int texture_stage);
  void add_xyzw_vertex_element (int stream_index, int offset, int texture_stage);

  bool add_end_vertex_element (void);

  int _total_elements;
  int _maximum_vertex_elements;

  LPD3DVERTEXELEMENT9 _vertex_element_array;
};

#endif
