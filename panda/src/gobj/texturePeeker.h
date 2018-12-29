/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePeeker.h
 * @author drose
 * @date 2008-08-26
 */

#ifndef TEXTUREPEEKER_H
#define TEXTUREPEEKER_H

#include "pandabase.h"

#include "referenceCount.h"
#include "texture.h"

/**
 * An instance of this object is returned by Texture::peek().  This object
 * allows quick and easy inspection of a texture's texels by (u, v)
 * coordinates.
 */
class EXPCL_PANDA_GOBJ TexturePeeker : public ReferenceCount {
private:
  TexturePeeker(Texture *tex, Texture::CData *cdata);

public:
  INLINE bool is_valid() const;

PUBLISHED:
  INLINE int get_x_size() const;
  INLINE int get_y_size() const;
  INLINE int get_z_size() const;

  INLINE bool has_pixel(int x, int y) const;
  void lookup(LColor &color, PN_stdfloat u, PN_stdfloat v) const;
  void lookup(LColor &color, PN_stdfloat u, PN_stdfloat v, PN_stdfloat w) const;
  void fetch_pixel(LColor &color, int x, int y) const;
  bool lookup_bilinear(LColor &color, PN_stdfloat u, PN_stdfloat v) const;
  void filter_rect(LColor &color,
                   PN_stdfloat min_u, PN_stdfloat min_v,
                   PN_stdfloat max_u, PN_stdfloat max_v) const;
  void filter_rect(LColor &color,
                   PN_stdfloat min_u, PN_stdfloat min_v, PN_stdfloat min_w,
                   PN_stdfloat max_u, PN_stdfloat max_v, PN_stdfloat max_w) const;

private:
  static void init_rect_minmax(int &min_x, int &max_x,
                               PN_stdfloat &min_u, PN_stdfloat &max_u,
                               int x_size);

  void accum_filter_z(LColor &color, PN_stdfloat &net,
                      int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
                      int min_y, int max_y, PN_stdfloat min_v, PN_stdfloat max_v,
                      int min_z, int max_z, PN_stdfloat min_w, PN_stdfloat max_w) const;
  void accum_filter_y(LColor &color, PN_stdfloat &net, int zi,
                      int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
                      int min_y, int max_y, PN_stdfloat min_v, PN_stdfloat max_v,
                      PN_stdfloat weight) const;
  void accum_filter_x(LColor &color, PN_stdfloat &net, int yi, int zi,
                      int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
                      PN_stdfloat weight) const;
  void accum_texel(LColor &color, PN_stdfloat &net, const unsigned char *&p,
                   PN_stdfloat weight) const;

  typedef double GetComponentFunc(const unsigned char *&p);
  typedef void GetTexelFunc(LColor &color, const unsigned char *&p,
                            GetComponentFunc *get_component);

  static void get_texel_r(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_g(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_b(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_a(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_l(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_la(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_rg(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_rgb(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_rgba(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_srgb(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);
  static void get_texel_srgba(LColor &color, const unsigned char *&p, GetComponentFunc *get_component);

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
