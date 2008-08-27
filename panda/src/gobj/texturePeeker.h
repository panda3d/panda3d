// Filename: texturePeeker.h
// Created by:  drose (26Aug08)
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

#ifndef TEXTUREPEEKER_H
#define TEXTUREPEEKER_H

#include "pandabase.h"

#include "referenceCount.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : TexturePeeker
// Description : An instance of this object is returned by
//               Texture::peek().  This object allows quick and easy
//               inspection of a texture's texels by (u, v)
//               coordinates.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ TexturePeeker : public ReferenceCount {
private:
  TexturePeeker(Texture *tex);

public:
  INLINE bool is_valid() const;

PUBLISHED:
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE int get_z_size() const;

  void lookup(Colorf &color, float u, float v) const;
  void lookup(Colorf &color, float u, float v, float w) const;
  void filter_rect(Colorf &color, 
                   float min_u, float min_v, 
                   float max_u, float max_v) const;
  void filter_rect(Colorf &color, 
                   float min_u, float min_v, float min_w,
                   float max_u, float max_v, float max_w) const;

private:
  static void init_rect_minmax(int &min_x, int &max_x, 
                               float &min_u, float &max_u,
                               int x_size);

  void accum_filter_z(Colorf &color, float &net,
                      int min_x, int max_x, float min_u, float max_u,
                      int min_y, int max_y, float min_v, float max_v,
                      int min_z, int max_z, float min_w, float max_w) const;
  void accum_filter_y(Colorf &color, float &net, int zi,
                      int min_x, int max_x, float min_u, float max_u,
                      int min_y, int max_y, float min_v, float max_v,
                      float weight) const;
  void accum_filter_x(Colorf &color, float &net, int yi, int zi,
                      int min_x, int max_x, float min_u, float max_u,
                      float weight) const;
  void accum_texel(Colorf &color, float &net, const unsigned char *&p, 
                   float weight) const;

  typedef double GetComponentFunc(const unsigned char *&p);
  typedef void GetTexelFunc(Colorf &color, const unsigned char *&p,
                            GetComponentFunc *get_component);

  static void get_texel_r(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_g(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_b(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_a(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_l(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_la(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_rgb(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_rgba(Colorf &color, const unsigned char *&p, GetComponentFunc *get_component);
  
  int _x_size;
  int _y_size;
  int _z_size;
  int _component_width;
  int _num_components;
  int _pixel_width;
  Texture::Format _format;
  Texture::ComponentType _component_type;
  CPTA_uchar _image;

  GetComponentFunc *_get_component;
  GetTexelFunc *_get_texel;

  friend class Texture;
};

#include "texturePeeker.I"

#endif

