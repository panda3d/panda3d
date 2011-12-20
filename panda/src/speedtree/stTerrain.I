// Filename: stTerrain.I
// Created by:  drose (11Oct10)
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


////////////////////////////////////////////////////////////////////
//     Function: STTerrain::is_valid
//       Access: Published
//  Description: Returns true if the terrain data is well-defined and
//               ready to use.
////////////////////////////////////////////////////////////////////
INLINE bool STTerrain::
is_valid() const {
  return _is_valid && !_st_vertex_attribs.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_normal_map
//       Access: Published
//  Description: Returns the normal map that should be applied to the
//               terrain.  This will be loaded and supplied to the
//               shader.
////////////////////////////////////////////////////////////////////
INLINE const Filename &STTerrain::
get_normal_map() const {
  return _normal_map;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_splat_map
//       Access: Published
//  Description: Returns the splat map that should be applied to the
//               terrain.  This will be loaded and supplied to the
//               shader.  Presumably, the shader will use the channels
//               of this map to determine which of the splat layers
//               are to be rendered at any given point.
////////////////////////////////////////////////////////////////////
INLINE const Filename &STTerrain::
get_splat_map() const {
  return _splat_map;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_num_splat_layers
//       Access: Published
//  Description: Returns the number of splat layers that are to be
//               applied to the terrain.  This must be consistent with
//               c_nNumTerrainSplatLayers in SpeedTree's TerrainRI.h.
////////////////////////////////////////////////////////////////////
INLINE int STTerrain::
get_num_splat_layers() const {
  return _splat_layers.size();
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_splat_layer
//       Access: Published
//  Description: Returns the nth splat layer that is to be applied to
//               the terrain.
////////////////////////////////////////////////////////////////////
INLINE const Filename &STTerrain::
get_splat_layer(int n) const {
  nassertr(n >= 0 && n < (int)_splat_layers.size(), _splat_layers[0]._filename);
  return _splat_layers[n]._filename;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_splat_layer_tiling
//       Access: Published
//  Description: Returns the tiling value of the nth splat layer.
//               This is an arbitrary UV scale that is applied to each
//               layer individually, by the terrain shader.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat STTerrain::
get_splat_layer_tiling(int n) const {
  nassertr(n >= 0 && n < (int)_splat_layers.size(), 0.0f);
  return _splat_layers[n]._tiling;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_splat_layer_color
//       Access: Published
//  Description: Returns the overall color of the nth splat layer.  This
//               is used just to match the color of the grass to its
//               terrain.
////////////////////////////////////////////////////////////////////
INLINE LColor STTerrain::
get_splat_layer_color(int n) const {
  nassertr(n >= 0 && n < (int)_splat_layers.size(), _splat_layers[0]._color);
  return LVecBase4(_splat_layers[n]._color);
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_vertex_format
//       Access: Published
//  Description: Returns the vertex format of the vertex array that is
//               supported by this terrain data.  A GeomVertexData of
//               the requested format will be passed to
//               fill_vertices().
////////////////////////////////////////////////////////////////////
INLINE const GeomVertexFormat *STTerrain::
get_vertex_format() {
  return _vertex_format;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_min_height
//       Access: Published
//  Description: Returns the smallest height value that might be
//               returned by get_height().  This is used as a culling
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat STTerrain::
get_min_height() const {
  return _min_height;
}

////////////////////////////////////////////////////////////////////
//     Function: STTerrain::get_max_height
//       Access: Published
//  Description: Returns the largest height value that might be
//               returned by get_height().  This is used as a culling
//               optimization.
////////////////////////////////////////////////////////////////////
INLINE PN_stdfloat STTerrain::
get_max_height() const {
  return _max_height;
}
