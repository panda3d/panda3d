// Filename: eggTextureCards.h
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

#ifndef EGGTEXTURECARDS_H
#define EGGTEXTURECARDS_H

#include "pandatoolbase.h"

#include "eggWriter.h"
#include "eggTexture.h"
#include "luse.h"

class EggVertexPool;
class EggVertex;

////////////////////////////////////////////////////////////////////
//       Class : EggTextureCards
// Description : Generates an egg file featuring a number of polygons,
//               one for each named texture.  This is a support
//               program for getting textures through egg-palettize.
////////////////////////////////////////////////////////////////////
class EggTextureCards : public EggWriter {
public:
  EggTextureCards();

protected:
  virtual bool handle_args(Args &args);

  static bool dispatch_wrap_mode(const string &opt, const string &arg, void *var);
  static bool dispatch_format(const string &opt, const string &arg, void *var);

private:
  bool scan_texture(const Filename &filename, LVecBase4d &geometry,
                    int &num_channels);
  void make_vertices(const LPoint4d &geometry, EggVertexPool *vpool,
                     EggVertex *&v1, EggVertex *&v2, EggVertex *&v3, EggVertex *&v4);

public:
  void run();

  LVecBase4d _polygon_geometry;
  LVecBase2d _pixel_scale;
  bool _got_pixel_scale;
  Colorf _polygon_color;
  vector_string _texture_names;
  EggTexture::WrapMode _wrap_mode;
  EggTexture::Format _format;
  EggTexture::Format _format_1, _format_2, _format_3, _format_4;
  bool _apply_bface;
};

#endif

