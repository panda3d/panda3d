// Filename: builderTypes.cxx
// Created by:  drose (11Sep97)
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

#include "builderTypes.h"
#include "notify.h"

ostream &operator << (ostream &out, BuilderAttribFlags baf) {
  const char *space = "";
  if (baf & BAF_coord) {
    out << space << "coord";
    space = " ";
  }
  if (baf & BAF_normal) {
    out << space << "normal";
    space = " ";
  }
  if (baf & BAF_texcoord) {
    out << space << "texcoord";
    space = " ";
  }
  if (baf & BAF_color) {
    out << space << "color";
    space = " ";
  }
  if (baf & BAF_pixel_size) {
    out << space << "pixel_size";
    space = " ";
  }
  if (baf & BAF_overall_updated) {
    out << space << "overall_updated";
    space = " ";
  }
  if (baf & BAF_overall_normal) {
    out << space << "overall_normal";
    space = " ";
  }
  if (baf & BAF_overall_color) {
    out << space << "overall_color";
    space = " ";
  }
  if (baf & BAF_overall_pixel_size) {
    out << space << "overall_pixel_size";
    space = " ";
  }
  if (baf & BAF_vertex_normal) {
    out << space << "vertex_normal";
    space = " ";
  }
  if (baf & BAF_vertex_texcoord) {
    out << space << "vertex_texcoord";
    space = " ";
  }
  if (baf & BAF_vertex_color) {
    out << space << "vertex_color";
    space = " ";
  }
  if (baf & BAF_vertex_pixel_size) {
    out << space << "vertex_pixel_size";
    space = " ";
  }
  if (baf & BAF_component_normal) {
    out << space << "component_normal";
    space = " ";
  }
  if (baf & BAF_component_color) {
    out << space << "component_color";
    space = " ";
  }
  if (baf & BAF_component_pixel_size) {
    out << space << "component_pixel_size";
    space = " ";
  }
  return out;
}


ostream &operator << (ostream &out, BuilderPrimType bpt) {
  switch (bpt) {
  case BPT_poly:
    return out << "poly";
  case BPT_point:
    return out << "point";
  case BPT_line:
    return out << "line";
  case BPT_tri:
    return out << "tri";
  case BPT_tristrip:
    return out << "tristrip";
  case BPT_trifan:
    return out << "trifan";
  case BPT_quad:
    return out << "quad";
  case BPT_quadstrip:
    return out << "quadstrip";
  case BPT_linestrip:
    return out << "linestrip";
  }
  nassertr(false, out);
  return out << "(**invalid**)";
}

