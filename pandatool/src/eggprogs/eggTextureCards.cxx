// Filename: eggTextureCards.cxx
// Created by:  drose (21Feb01)
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

#include "eggTextureCards.h"

#include "eggGroup.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggTexture.h"
#include "eggPolygon.h"
#include "pnmImageHeader.h"

#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggTextureCards::
EggTextureCards() : EggWriter(true, true) {
  set_program_description
    ("egg-texture-cards generates an egg file consisting of several "
     "square polygons, one for each texture name that appears on the "
     "command line.\n\n"

     "This is a handy thing to have for importing texture images through "
     "egg-palettize, even when those textures do not appear on any real "
     "geometry; it can also be used for creating a lot of simple polygons "
     "for rendering click buttons and similar interfaces.");

  clear_runlines();
  add_runline("[opts] texture [texture ...] output.egg");
  add_runline("[opts] -o output.egg texture [texture ...]");
  add_runline("[opts] texture [texture ...] >output.egg");

  add_option
    ("g", "left,right,bottom,top", 0,
     "Specifies the geometry of each polygon.  The default is a unit polygon "
     "centered on the origin: -0.5,0.5,-0.5,0.5.  Polygons are always created "
     "on the X-Y plane.  If -p is not also specified, all polygons will be "
     "the same size and shape.",
     &EggTextureCards::dispatch_double_quad, NULL, &_polygon_geometry[0]);

  add_option
    ("p", "xpixels,ypixels", 0,
     "Indicates that polygons should be sized in proportion to the pixel "
     "size of the texture image.  This will potentially create a "
     "different size and shape polygon for each texture.  The coordinate "
     "pair represents the image size in "
     "pixels that will exactly fill up the polygon described with -g (or the "
     "default polygon if -g is not specified); smaller images will be "
     "given proportionately smaller polygons, and larger images will be "
     "given proportionately larger polygons.",
     &EggTextureCards::dispatch_double_pair, &_got_pixel_scale, &_pixel_scale[0]);

  add_option
    ("c", "r,g,b[,a]", 0,
     "Specifies the color of each polygon.  The default is white: 1,1,1,1.",
     &EggTextureCards::dispatch_color, NULL, &_polygon_color[0]);

  add_option
    ("wm", "[repeat | clamp]", 0,
     "Indicates the wrap mode of the texture: either \"repeat\" or \"clamp\" "
     "(or \"r\" or \"c\").  The default is to leave this unspecified.",
     &EggTextureCards::dispatch_wrap_mode, NULL, &_wrap_mode);

  add_option
    ("f", "format", 0,
     "Indicates the format for all textures: typical choices are \"rgba12\" "
     "or \"rgb5\" or \"alpha\".  The default is to leave this unspecified.",
     &EggTextureCards::dispatch_format, NULL, &_format);

  add_option
    ("f1", "format", 0,
     "Indicates the format for one-channel textures only.  If specified, this "
     "overrides the format specified by -f.",
     &EggTextureCards::dispatch_format, NULL, &_format_1);

  add_option
    ("f2", "format", 0,
     "Indicates the format for two-channel textures only.  If specified, this "
     "overrides the format specified by -f.",
     &EggTextureCards::dispatch_format, NULL, &_format_2);

  add_option
    ("f3", "format", 0,
     "Indicates the format for three-channel textures only.  If specified, this "
     "overrides the format specified by -f.",
     &EggTextureCards::dispatch_format, NULL, &_format_3);

  add_option
    ("f4", "format", 0,
     "Indicates the format for four-channel textures only.  If specified, this "
     "overrides the format specified by -f.",
     &EggTextureCards::dispatch_format, NULL, &_format_4);

  add_option
    ("b", "", 0,
     "Make the textured polygons backfaced",
     &EggTextureCards::dispatch_none, &_apply_bface);


  _polygon_geometry.set(-0.5, 0.5, -0.5, 0.5);
  _polygon_color.set(1.0, 1.0, 1.0, 1.0);
  _wrap_mode = EggTexture::WM_unspecified;
  _format = EggTexture::F_unspecified;
  _format_1 = EggTexture::F_unspecified;
  _format_2 = EggTexture::F_unspecified;
  _format_3 = EggTexture::F_unspecified;
  _format_4 = EggTexture::F_unspecified;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
handle_args(ProgramBase::Args &args) {
  if (!check_last_arg(args, 0)) {
    return false;
  }

  if (args.empty()) {
    nout << "No texture names specified on the command line.\n";
    return false;
  }

  copy(args.begin(), args.end(),
       back_inserter(_texture_names));

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::dispatch_wrap_mode
//       Access: Protected, Static
//  Description: Standard dispatch function for an option that takes
//               one parameter, which is to be interpreted as a
//               WrapMode string.  The data pointer is to a WrapMode
//               enum variable.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
dispatch_wrap_mode(const string &opt, const string &arg, void *var) {
  EggTexture::WrapMode *wmp = (EggTexture::WrapMode *)var;

  *wmp = EggTexture::string_wrap_mode(arg);
  if (*wmp == EggTexture::WM_unspecified) {
    // An unknown string.  Let's check for our special cases.
    if (arg == "r") {
      *wmp = EggTexture::WM_repeat;
    } else if (arg == "c") {
      *wmp = EggTexture::WM_clamp;
    } else {
      nout << "Invalid wrap mode parameter for -" << opt << ": "
           << arg << "\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::dispatch_format
//       Access: Protected, Static
//  Description: Standard dispatch function for an option that takes
//               one parameter, which is to be interpreted as a
//               Format string.  The data pointer is to a Format
//               enum variable.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
dispatch_format(const string &opt, const string &arg, void *var) {
  EggTexture::Format *fp = (EggTexture::Format *)var;

  *fp = EggTexture::string_format(arg);
  if (*fp == EggTexture::F_unspecified) {
    nout << "Invalid format parameter for -" << opt << ": "
         << arg << "\n";
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::scan_texture
//       Access: Private
//  Description: Reads the texture image header to determine its size,
//               and based on this size, computes the appropriate
//               left,right,bottom,top geometry of the card that
//               correspond to this texture.
//
//               Returns true if successful, or false if the texture
//               cannot be read.
////////////////////////////////////////////////////////////////////
bool EggTextureCards::
scan_texture(const Filename &filename, LVecBase4d &geometry,
             int &num_channels) {
  PNMImageHeader header;
  if (!header.read_header(filename)) {
    nout << "Unable to read image " << filename << "\n";
    return false;
  }

  num_channels = header.get_num_channels();

  double xscale = header.get_x_size() / _pixel_scale[0];
  double yscale = header.get_y_size() / _pixel_scale[1];

  geometry.set(_polygon_geometry[0] * xscale,
               _polygon_geometry[1] * xscale,
               _polygon_geometry[2] * yscale,
               _polygon_geometry[3] * yscale);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::make_vertices
//       Access: Private
//  Description: Creates a set of four vertices for the polygon
//               according to the left,right,bottom,top geometry.
////////////////////////////////////////////////////////////////////
void EggTextureCards::
make_vertices(const LPoint4d &geometry, EggVertexPool *vpool,
              EggVertex *&v1, EggVertex *&v2, EggVertex *&v3, EggVertex *&v4) {
  //
  //   1     4
  //
  //
  //   2     3
  //

  v1 = vpool->make_new_vertex
    (LPoint3d(geometry[0], geometry[3], 0.0));
  v2 = vpool->make_new_vertex
    (LPoint3d(geometry[0], geometry[2], 0.0));
  v3 = vpool->make_new_vertex
    (LPoint3d(geometry[1], geometry[2], 0.0));
  v4 = vpool->make_new_vertex
    (LPoint3d(geometry[1], geometry[3], 0.0));

  v1->set_uv(TexCoordd(0.0, 1.0));
  v2->set_uv(TexCoordd(0.0, 0.0));
  v3->set_uv(TexCoordd(1.0, 0.0));
  v4->set_uv(TexCoordd(1.0, 1.0));
}

////////////////////////////////////////////////////////////////////
//     Function: EggTextureCards::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggTextureCards::
run() {
  // First, create an enclosing group and a vertex pool with four
  // vertices.  We can use the same four vertices on all polygons.

  EggGroup *group = new EggGroup();
  _data.add_child(group);

  // If we have more than one tile, make the group a sequence, as a
  // convenience.  If we view the egg file directly we can see all the
  // tiles one at a time.
  if (_texture_names.size() > 1) {
    group->set_switch_flag(true);
    group->set_switch_fps(2.0);
  }

  EggVertexPool *vpool = new EggVertexPool("vpool");
  group->add_child(vpool);

  EggVertex *v1, *v2, *v3, *v4;

  if (!_got_pixel_scale) {
    // If we don't have a per-texture pixel scale, all the polygons
    // will be the same size, and hence may all share the same four
    // vertices.
    make_vertices(_polygon_geometry, vpool, v1, v2, v3, v4);
  }

  // Now, create a texture reference and a polygon for each texture.

  vector_string::const_iterator ti;
  for (ti = _texture_names.begin(); ti != _texture_names.end(); ++ti) {
    Filename filename = (*ti);
    string name = filename.get_basename_wo_extension();

    // Read in the texture header and determine its size.
    LVecBase4d geometry;
    int num_channels;
    bool texture_ok = scan_texture(filename, geometry, num_channels);

    if (texture_ok) {
      if (_got_pixel_scale) {
        make_vertices(geometry, vpool, v1, v2, v3, v4);
      }

      EggTexture *tref = new EggTexture(name, filename);
      tref->set_wrap_mode(_wrap_mode);

      switch (num_channels) {
      case 1:
        tref->set_format(_format_1);
        break;

      case 2:
        tref->set_format(_format_2);
        break;

      case 3:
        tref->set_format(_format_3);
        break;

      case 4:
        tref->set_format(_format_4);
        break;
      }

      if (tref->get_format() == EggTexture::F_unspecified) {
        tref->set_format(_format);
      }
      group->add_child(tref);

      // Each polygon gets placed in its own sub-group.  This will make
      // pulling them out by name at runtime possible.
      EggGroup *sub_group = new EggGroup(name);
      group->add_child(sub_group);
      EggPolygon *poly = new EggPolygon();
      sub_group->add_child(poly);
      poly->set_texture(tref);
      poly->set_color(_polygon_color);
      if (_apply_bface){
        poly->set_bface_flag(1);
      }

      poly->add_vertex(v1);
      poly->add_vertex(v2);
      poly->add_vertex(v3);
      poly->add_vertex(v4);
    }
  }

  // Done!
  write_egg_file();
}


int main(int argc, char *argv[]) {
  EggTextureCards prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
