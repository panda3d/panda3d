// Filename: builderVertex.I
// Created by:  drose (18Sep97)
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

#include "notify.h"

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_coord_value
//       Access: Public
//  Description: Reassigns the vertex coordinate, without knowing
//               whether the vertex is indexed or nonindexed.  A
//               nonindexed vertex will look up the index in the array
//               and store the resulting value, while an indexed
//               vertex will just store the index number (which
//               assumes the array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertex::
set_coord_value(const BuilderV *array, ushort index) {
  set_coord(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_normal_value
//       Access: Public
//  Description: Reassigns the vertex normal, without knowing whether
//               the vertex is indexed or nonindexed.  A nonindexed
//               vertex will look up the index in the array and store
//               the resulting value, while an indexed vertex will
//               just store the index number (which assumes the array
//               is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertex::
set_normal_value(const BuilderN *array, ushort index) {
  set_normal(array[index]);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_texcoord_value
//       Access: Public
//  Description: Reassigns the vertex texture coordinate, without
//               knowing whether the vertex is indexed or nonindexed.
//               A nonindexed vertex will look up the index in the
//               array and store the resulting value, while an indexed
//               vertex will just store the index number (which
//               assumes the array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertex::
set_texcoord_value(const TexCoordName *name, const BuilderTC *array, ushort index) {
  set_texcoord(name, array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_color_value
//       Access: Public
//  Description: Reassigns the vertex color, without knowing whether
//               the vertex is indexed or nonindexed.  A nonindexed
//               vertex will look up the index in the array and store
//               the resulting value, while an indexed vertex will
//               just store the index number (which assumes the array
//               is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertex::
set_color_value(const BuilderC *array, ushort index) {
  set_color(array[index]);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_coord_value
//       Access: Public
//  Description: Returns the actual coordinate value of the vertex,
//               whether it is indexed or nonindexed.  Normally, the
//               value returned by get_coord(), which will be either a
//               BuilderV or a ushort, is sufficient, but there are
//               occasional times when it is necessary to get the
//               actual location in space of the vertex position (for
//               instance, to subdivide a concave polygon).
//
//               This function returns the actual coordinate value.
//               For a nonindexed vertex, its return value is the same
//               as get_coord(); for an indexed vertex, it looks up
//               the vertex's index in the bucket's coord array, and
//               returns that value.
//
//               Note that this makes some perhaps unwarranted
//               assumptions about indexed geometry; specifically,
//               that its value is valid at creation time, and that it
//               won't change too drastically during runtime.
////////////////////////////////////////////////////////////////////
INLINE BuilderV BuilderVertex::
get_coord_value(const BuilderBucket &) const {
  return get_coord();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_normal_value
//       Access: Public
//  Description: Returns the actual normal value of the vertex,
//               whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderN BuilderVertex::
get_normal_value(const BuilderBucket &) const {
  return get_normal();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_texcoord_value
//       Access: Public
//  Description: Returns the actual texture coordinate value of the
//               vertex, whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderTC BuilderVertex::
get_texcoord_value(const TexCoordName *name, const BuilderBucket &) const {
  return get_texcoord(name);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_color_value
//       Access: Public
//  Description: Returns the actual color value of the vertex,
//               whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderC BuilderVertex::
get_color_value(const BuilderBucket &) const {
  return get_color();
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_coord_value
//       Access: Public
//  Description: Reassigns the vertex coordinate, without knowing
//               whether the vertex is indexed or nonindexed.  A
//               nonindexed vertex will look up the index in the array
//               and store the resulting value, while an indexed
//               vertex will just store the index number (which
//               assumes the array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertexI::
set_coord_value(const BuilderV *, ushort index) {
  set_coord(index);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_normal_value
//       Access: Public
//  Description: Reassigns the vertex normal, without knowing whether
//               the vertex is indexed or nonindexed.  A nonindexed
//               vertex will look up the index in the array and store
//               the resulting value, while an indexed vertex will
//               just store the index number (which assumes the array
//               is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertexI::
set_normal_value(const BuilderN *, ushort index) {
  set_normal(index);
}


////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_texcoord_value
//       Access: Public
//  Description: Reassigns the vertex texture coordinate, without
//               knowing whether the vertex is indexed or nonindexed.
//               A nonindexed vertex will look up the index in the
//               array and store the resulting value, while an indexed
//               vertex will just store the index number (which
//               assumes the array is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertexI::
set_texcoord_value(const TexCoordName *name,const BuilderTC *, ushort index) {
  set_texcoord(name, index);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::set_color_value
//       Access: Public
//  Description: Reassigns the vertex color, without knowing whether
//               the vertex is indexed or nonindexed.  A nonindexed
//               vertex will look up the index in the array and store
//               the resulting value, while an indexed vertex will
//               just store the index number (which assumes the array
//               is the same one it's indexing on).
////////////////////////////////////////////////////////////////////
INLINE void BuilderVertexI::
set_color_value(const BuilderC *, ushort index) {
  set_color(index);
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_coord_value
//       Access: Public
//  Description: Returns the actual coordinate value of the vertex,
//               whether it is indexed or nonindexed.  Normally, the
//               value returned by get_coord(), which will be either a
//               BuilderV or a ushort, is sufficient, but there are
//               occasional times when it is necessary to get the
//               actual location in space of the vertex position (for
//               instance, to subdivide a concave polygon).
//
//               This function returns the actual coordinate value.
//               For a nonindexed vertex, its return value is the same
//               as get_coord(); for an indexed vertex, it looks up
//               the vertex's index in the bucket's coord array, and
//               returns that value.
//
//               Note that this makes some perhaps unwarranted
//               assumptions about indexed geometry; specifically,
//               that its value is valid at creation time, and that it
//               won't change too drastically during runtime.
////////////////////////////////////////////////////////////////////
INLINE BuilderV BuilderVertexI::
get_coord_value(const BuilderBucket &bucket) const {
  nassertr(bucket.get_coords() != (Vertexf *)NULL, BuilderV());
  return bucket.get_coords()[get_coord()];
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_normal_value
//       Access: Public
//  Description: Returns the actual normal value of the vertex,
//               whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderN BuilderVertexI::
get_normal_value(const BuilderBucket &bucket) const {
  nassertr(bucket.get_normals() != (Normalf *)NULL, BuilderN());
  return bucket.get_normals()[get_normal()];
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_texcoord_value
//       Access: Public
//  Description: Returns the actual texture coordinate value of the
//               vertex, whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderTC BuilderVertexI::
get_texcoord_value(const TexCoordName *name, const BuilderBucket &bucket) const {
  nassertr(bucket.get_texcoords(name) != (TexCoordf *)NULL, BuilderTC());
  return bucket.get_texcoords(name)[get_texcoord(name)];
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderVertex::get_color_value
//       Access: Public
//  Description: Returns the actual color value of the vertex,
//               whether it is indexed or nonindexed.  See
//               get_coord_value().
////////////////////////////////////////////////////////////////////
INLINE BuilderC BuilderVertexI::
get_color_value(const BuilderBucket &bucket) const {
  nassertr(bucket.get_colors() != (Colorf *)NULL, BuilderC());
  return bucket.get_colors()[get_color()];
}
