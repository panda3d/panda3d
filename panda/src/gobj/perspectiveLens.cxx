// Filename: perspectiveLens.cxx
// Created by:  drose (18Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "perspectiveLens.h"

TypeHandle PerspectiveLens::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::make_copy
//       Access: Public, Virtual
//  Description: Allocates a new Lens just like this one.
////////////////////////////////////////////////////////////////////
PT(Lens) PerspectiveLens::
make_copy() const {
  return new PerspectiveLens(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::is_linear
//       Access: Published, Virtual
//  Description: Returns true if the lens represents a linear
//               projection (e.g. PerspectiveLens, OrthographicLens),
//               and therefore there is a valid matrix returned by
//               get_projection_mat(), or false otherwise.
////////////////////////////////////////////////////////////////////
bool PerspectiveLens::
is_linear() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::compute_projection_mat
//       Access: Protected, Virtual
//  Description: Computes the complete transformation matrix from 3-d
//               point to 2-d point, if the lens is linear.
////////////////////////////////////////////////////////////////////
void PerspectiveLens::
compute_projection_mat() {
  CoordinateSystem cs = _cs;
  if (cs == CS_default) {
    cs = default_coordinate_system;
  }

  float fl = get_focal_length();
  float fFar = get_far();
  float fNear = get_near();
  float far_minus_near = fFar-fNear;
  float a = (fFar + fNear);
  float b = -2.0f * fFar * fNear;

  a/=far_minus_near;
  b/=far_minus_near;

  LMatrix4f canonical;
  switch (cs) {
  case CS_zup_right:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  1.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_yup_right:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a, -1.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_zup_left:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,  0.0f,    -a, -1.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  case CS_yup_left:
    canonical.set(  fl,  0.0f,  0.0f,  0.0f,
                  0.0f,    fl,  0.0f,  0.0f,
                  0.0f,  0.0f,     a,  1.0f,
                  0.0f,  0.0f,     b,  0.0f);
    break;

  default:
    gobj_cat.error()
      << "Invalid coordinate system " << (int)cs << " in PerspectiveLens!\n";
    canonical = LMatrix4f::ident_mat();
  }


  _projection_mat = get_lens_mat_inv() * canonical * get_film_mat();
  adjust_comp_flags(CF_projection_mat_inv, 
                    CF_projection_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::fov_to_film
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a focal length,
//               compute the correspdonding width (or height) on the
//               film.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float PerspectiveLens::
fov_to_film(float fov, float focal_length, bool) const {
  return (ctan(deg_2_rad(fov * 0.5f)) * focal_length) * 2.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::fov_to_focal_length
//       Access: Protected, Virtual
//  Description: Given a field of view in degrees and a width (or
//               height) on the film, compute the focal length of the
//               lens.  If horiz is true, this is in the horizontal
//               direction; otherwise, it is in the vertical direction
//               (some lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float PerspectiveLens::
fov_to_focal_length(float fov, float film_size, bool) const {
  return film_size * 0.5f / ctan(deg_2_rad(fov * 0.5f));
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::film_to_fov
//       Access: Protected, Virtual
//  Description: Given a width (or height) on the film and a focal
//               length, compute the field of view in degrees.  If
//               horiz is true, this is in the horizontal direction;
//               otherwise, it is in the vertical direction (some
//               lenses behave differently in each direction).
////////////////////////////////////////////////////////////////////
float PerspectiveLens::
film_to_fov(float film_size, float focal_length, bool) const {
  return rad_2_deg(catan(film_size * 0.5f / focal_length)) * 2.0f;
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Lens.
////////////////////////////////////////////////////////////////////
void PerspectiveLens::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: PerspectiveLens::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Lens is encountered
//               in the Bam file.  It should create the Lens
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *PerspectiveLens::
make_from_bam(const FactoryParams &params) {
  PerspectiveLens *lens = new PerspectiveLens;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  lens->fillin(scan, manager);

  return lens;
}
