// Filename: pfmVizzer.I
// Created by:  drose (30Sep12)
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
//     Function: PfmVizzer::get_pfm
//       Access: Published
//  Description: Returns the reference to the PfmFile manipulated by
//               this PfmVizzer.
////////////////////////////////////////////////////////////////////
INLINE PfmFile &PfmVizzer::
get_pfm() {
  return _pfm;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_pfm
//       Access: Published
//  Description: Returns the reference to the PfmFile manipulated by
//               this PfmVizzer.
////////////////////////////////////////////////////////////////////
INLINE const PfmFile &PfmVizzer::
get_pfm() const {
  return _pfm;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_vis_inverse
//       Access: Published
//  Description: Sets the vis_inverse flag.  When this flag is true,
//               vis meshes and point clouds are generated with the
//               3-d depth value in the texture coordinates, and the
//               2-d index value in the vertex position.  When it is
//               false, meshes are generated normally, with the 3-d
//               depth value in the vertex position and the 2-d index
//               value in the texture coordinates.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_vis_inverse(bool vis_inverse) {
  _vis_inverse = vis_inverse;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_vis_inverse
//       Access: Published
//  Description: Returns the vis_inverse flag.  See set_vis_inverse().
////////////////////////////////////////////////////////////////////
INLINE bool PfmVizzer::
get_vis_inverse() const {
  return _vis_inverse;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_flat_texcoord_name
//       Access: Published
//  Description: If the flat_texcoord_name is specified, it is the
//               name of an additional vertex column that will be
//               created for the "flat" texture coordinates, i.e. the
//               original 0..1 values that correspond to the 2-D index
//               position of each point in the original pfm file.
//
//               These are the same values that will be assigned to
//               the default texture coordinates if the vis_inverse
//               flag is *not* true.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_flat_texcoord_name(InternalName *flat_texcoord_name) {
  _flat_texcoord_name = flat_texcoord_name;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::clear_flat_texcoord_name
//       Access: Published
//  Description: Resets the flat_texcoord_name to empty, so that
//               additional texture coordinates are not created.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
clear_flat_texcoord_name() {
  _flat_texcoord_name = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_flat_texcoord_name
//       Access: Published
//  Description: Returns the flat_texcoord_name.  See set_flat_texcoord_name().
////////////////////////////////////////////////////////////////////
INLINE InternalName *PfmVizzer::
get_flat_texcoord_name() const {
  return _flat_texcoord_name;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_vis_2d
//       Access: Published
//  Description: Sets the vis_2d flag.  When this flag is true,
//               only the first two (x, y) value of each depth point
//               is considered meaningful; the z component is ignored.
//               This is only relevant for generating visualizations.
//
//               This may be used in lieu of the lower-level
//               add_vis_column().
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_vis_2d(bool vis_2d) {
  _vis_2d = vis_2d;
  clear_vis_columns();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_vis_2d
//       Access: Published
//  Description: Returns the vis_2d flag.  See set_vis_2d().
////////////////////////////////////////////////////////////////////
INLINE bool PfmVizzer::
get_vis_2d() const {
  return _vis_2d;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_keep_beyond_lens
//       Access: Published
//  Description: Sets the keep_beyond_lens flag.  When this flag is
//               true, points that fall outside of the normal lens
//               range in project() or in add_vis_column() will be
//               retained anyway; when it is false, these points will
//               be discarded.
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_keep_beyond_lens(bool keep_beyond_lens) {
  _keep_beyond_lens = keep_beyond_lens;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_keep_beyond_lens
//       Access: Published
//  Description: Returns the keep_beyond_lens flag.  See
//               set_keep_beyond_lens().
////////////////////////////////////////////////////////////////////
INLINE bool PfmVizzer::
get_keep_beyond_lens() const {
  return _keep_beyond_lens;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_vis_blend
//       Access: Published
//  Description: Specifies a blending map--a grayscale image--that
//               will be applied to the vertex color during
//               generate_vis_mesh() and generate_vis_points().  The
//               image size must exactly match the mesh size of the
//               PfmVizzer.
//
//               Ownership of the pointer is not kept by the PfmVizzer;
//               it is your responsibility to ensure it does not
//               destruct during the lifetime of the PfmVizzer (or at
//               least not before your subsequent call to
//               generate_vis_mesh()).
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_vis_blend(const PNMImage *vis_blend) {
  _vis_blend = vis_blend;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_vis_blend
//       Access: Published
//  Description: Removes the blending map set by a prior call to
//               set_vis_blend().
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
clear_vis_blend() {
  _vis_blend = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_vis_blend
//       Access: Published
//  Description: Returns the blending map set by the most recent call
//               to set_vis_blend(), or NULL if there is no blending
//               map in effect.
////////////////////////////////////////////////////////////////////
INLINE const PNMImage *PfmVizzer::
get_vis_blend() const {
  return _vis_blend;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::set_aux_pfm
//       Access: Published
//  Description: Assigns an auxiliary PfmFile to this PfmVizzer.  This
//               file will be queried by column types
//               CT_aux_vertex1/2/3, but has no other meaning to the
//               vizzer.  This size of this PfmFile should exactly
//               match the base PfmFile.  No reference count is held
//               and no copy is made; the caller is responsible for
//               ensuring that the auxiliary PfmFile will persist
//               throughout the lifetime of the PfmVizzer it is
//               assigned to.
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
set_aux_pfm(const PfmFile *pfm) {
  assert(pfm == NULL || (pfm->get_x_size() == _pfm.get_x_size() &&
                         pfm->get_y_size() == _pfm.get_y_size()));
  _aux_pfm = pfm;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::clear_aux_pfm
//       Access: Published
//  Description: Removes the auxiliary PfmFile from this PfmVizzer.
////////////////////////////////////////////////////////////////////
INLINE void PfmVizzer::
clear_aux_pfm() {
  _aux_pfm = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::get_aux_pfm
//       Access: Published
//  Description: Returns the reference to the auxiliary PfmFile
//               queried by this PfmVizzer.  This contains the values
//               that will be reflected in CT_aux_vertex3 etc.  See
//               set_aux_pfm().
////////////////////////////////////////////////////////////////////
INLINE const PfmFile *PfmVizzer::
get_aux_pfm() const {
  return _aux_pfm;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmVizzer::VisColumn::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PfmVizzer::VisColumn::
VisColumn() {
  _undist_lut = NULL;
}
