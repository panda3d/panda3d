/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file texturePeeker.cxx
 * @author drose
 * @date 2008-08-26
 */

#include "texturePeeker.h"


/**
 * Use Texture::peek() to construct a TexturePeeker.
 *
 * This constructor is called only by Texture::peek(), and assumes the
 * texture's lock is already held.
 */
TexturePeeker::
TexturePeeker(Texture *tex, Texture::CData *cdata) {
  if (cdata->_texture_type == Texture::TT_cube_map) {
    // Cube map texture.  We'll need to map from (u, v, w) to (u, v) within
    // the appropriate page, where w indicates the page.

    // TODO: handle cube maps.
    return;

  } else {
    // Regular 1-d, 2-d, or 3-d texture.  The coordinates map directly.
    // Simple ram images are possible if it is a 2-d texture.
    if (tex->do_has_ram_image(cdata) && cdata->_ram_image_compression == Texture::CM_off) {
      // Get the regular RAM image if it is available.
      _image = tex->do_get_ram_image(cdata);
      _x_size = cdata->_x_size;
      _y_size = cdata->_y_size;
      _z_size = cdata->_z_size;
      _component_width = cdata->_component_width;
      _num_components = cdata->_num_components;
      _format = cdata->_format;
      _component_type = cdata->_component_type;

    } else if (!cdata->_simple_ram_image._image.empty()) {
      // Get the simple RAM image if *that* is available.
      _image = cdata->_simple_ram_image._image;
      _x_size = cdata->_simple_x_size;
      _y_size = cdata->_simple_y_size;
      _z_size = 1;

      _component_width = 1;
      _num_components = 4;
      _format = Texture::F_rgba;
      _component_type = Texture::T_unsigned_byte;

    } else {
      // Failing that, reload and get the uncompressed RAM image.
      _image = tex->do_get_uncompressed_ram_image(cdata);
      _x_size = cdata->_x_size;
      _y_size = cdata->_y_size;
      _z_size = cdata->_z_size;
      _component_width = cdata->_component_width;
      _num_components = cdata->_num_components;
      _format = cdata->_format;
      _component_type = cdata->_component_type;
    }
  }

  if (_image.is_null()) {
    return;
  }
  _pixel_width = _component_width * _num_components;

  switch (_component_type) {
  case Texture::T_unsigned_byte:
    _get_component = Texture::get_unsigned_byte;
    break;

  case Texture::T_unsigned_short:
    _get_component = Texture::get_unsigned_short;
    break;

  case Texture::T_unsigned_int:
    _get_component = Texture::get_unsigned_int;
    break;

  case Texture::T_float:
    _get_component = Texture::get_float;
    break;

  case Texture::T_half_float:
    _get_component = Texture::get_half_float;
    break;

  case Texture::T_unsigned_int_24_8:
    _get_component = Texture::get_unsigned_int_24;
    break;

  default:
    // Not supported.
    _image.clear();
    return;
  }

  switch (_format) {
  case Texture::F_depth_stencil:
  case Texture::F_depth_component:
  case Texture::F_depth_component16:
  case Texture::F_depth_component24:
  case Texture::F_depth_component32:
  case Texture::F_red:
  case Texture::F_r16:
  case Texture::F_r32:
  case Texture::F_r32i:
    _get_texel = get_texel_r;
    break;

  case Texture::F_green:
    _get_texel = get_texel_g;
    break;

  case Texture::F_blue:
    _get_texel = get_texel_b;
    break;

  case Texture::F_alpha:
    _get_texel = get_texel_a;
    break;

  case Texture::F_luminance:
  case Texture::F_sluminance:
    _get_texel = get_texel_l;
    break;

  case Texture::F_luminance_alpha:
  case Texture::F_sluminance_alpha:
  case Texture::F_luminance_alphamask:
    _get_texel = get_texel_la;
    break;

  case Texture::F_rg16:
  case Texture::F_rg32:
  case Texture::F_rg:
    _get_texel = get_texel_rg;
    break;

  case Texture::F_rgb:
  case Texture::F_rgb5:
  case Texture::F_rgb8:
  case Texture::F_rgb12:
  case Texture::F_rgb16:
  case Texture::F_rgb332:
  case Texture::F_r11_g11_b10:
  case Texture::F_rgb9_e5:
  case Texture::F_rgb32:
    _get_texel = get_texel_rgb;
    break;

  case Texture::F_rgba:
  case Texture::F_rgbm:
  case Texture::F_rgba4:
  case Texture::F_rgba5:
  case Texture::F_rgba8:
  case Texture::F_rgba12:
  case Texture::F_rgba16:
  case Texture::F_rgba32:
  case Texture::F_rgb10_a2:
    _get_texel = get_texel_rgba;
    break;

  case Texture::F_srgb:
    if (_component_type == Texture::T_unsigned_byte) {
      _get_texel = get_texel_srgb;
    } else {
      gobj_cat.error()
        << "sRGB texture should have component type T_unsigned_byte\n";
    }
    break;

  case Texture::F_srgb_alpha:
    if (_component_type == Texture::T_unsigned_byte) {
      _get_texel = get_texel_srgba;
    } else {
      gobj_cat.error()
        << "sRGB texture should have component type T_unsigned_byte\n";
    }
    break;

  default:
    // Not supported.
    gobj_cat.error() << "Unsupported texture peeker format: "
      << Texture::format_format(_format) << std::endl;
    _image.clear();
    return;
  }
}


/**
 * Fills "color" with the RGBA color of the texel at point (u, v).
 *
 * The texel color is determined via nearest-point sampling (no filtering of
 * adjacent pixels), regardless of the filter type associated with the
 * texture.  u, v, and w will wrap around regardless of the texture's wrap
 * mode.
 */
void TexturePeeker::
lookup(LColor &color, PN_stdfloat u, PN_stdfloat v) const {
  int x = int((u - cfloor(u)) * (PN_stdfloat)_x_size) % _x_size;
  int y = int((v - cfloor(v)) * (PN_stdfloat)_y_size) % _y_size;
  fetch_pixel(color, x, y);
}

/**
 *  Works like TexturePeeker::lookup(), but instead uv-coordinates integer
 *  coordinates are used.
 */
void TexturePeeker::
fetch_pixel(LColor& color, int x, int y) const {
  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size);
  const unsigned char *p = _image.p() + (y * _x_size + x) * _pixel_width;
  (*_get_texel)(color, p, _get_component);
}


/**
 * Performs a bilinear lookup to retrieve the color value stored at the uv
 * coordinate (u, v).
 *
 * In case the point is outside of the uv range, color is set to zero,
 * and false is returned.  Otherwise true is returned.
 */
bool TexturePeeker::
lookup_bilinear(LColor &color, PN_stdfloat u, PN_stdfloat v) const {
  color = LColor::zero();

  u = u * _x_size - 0.5;
  v = v * _y_size - 0.5;

  int min_u = int(floor(u));
  int min_v = int(floor(v));

  PN_stdfloat frac_u = u - min_u;
  PN_stdfloat frac_v = v - min_v;

  LColor p00(LColor::zero()), p01(LColor::zero()), p10(LColor::zero()), p11(LColor::zero());
  PN_stdfloat w00 = 0.0, w01 = 0.0, w10 = 0.0, w11 = 0.0;

  if (has_pixel(min_u, min_v)) {
    w00 = (1.0 - frac_v) * (1.0 - frac_u);
    fetch_pixel(p00, min_u, min_v);
  }
  if (has_pixel(min_u + 1, min_v)) {
    w10 = (1.0 - frac_v) * frac_u;
    fetch_pixel(p10, min_u + 1, min_v);
  }
  if (has_pixel(min_u, min_v + 1)) {
    w01 = frac_v * (1.0 - frac_u);
    fetch_pixel(p01, min_u, min_v + 1);
  }
  if (has_pixel(min_u + 1, min_v + 1)) {
    w11 = frac_v * frac_u;
    fetch_pixel(p11, min_u + 1, min_v + 1);
  }

  PN_stdfloat net_w = w00 + w01 + w10 + w11;
  if (net_w == 0.0) {
    return false;
  }

  color = (p00 * w00 + p01 * w01 + p10 * w10 + p11 * w11) / net_w;
  return true;
}

/**
 * Fills "color" with the RGBA color of the texel at point (u, v, w).
 *
 * The texel color is determined via nearest-point sampling (no filtering of
 * adjacent pixels), regardless of the filter type associated with the
 * texture.  u, v, and w will wrap around regardless of the texture's wrap
 * mode.
 */
void TexturePeeker::
lookup(LColor &color, PN_stdfloat u, PN_stdfloat v, PN_stdfloat w) const {
  int x = int((u - cfloor(u)) * (PN_stdfloat)_x_size) % _x_size;
  int y = int((v - cfloor(v)) * (PN_stdfloat)_y_size) % _y_size;
  int z = int((w - cfloor(w)) * (PN_stdfloat)_z_size) % _z_size;

  nassertv(x >= 0 && x < _x_size && y >= 0 && y < _y_size &&
           z >= 0 && z < _z_size);
  const unsigned char *p = _image.p() + (z * _x_size * _y_size + y * _x_size + x) * _pixel_width;

  (*_get_texel)(color, p, _get_component);
}

/**
 * Fills "color" with the average RGBA color of the texels within the
 * rectangle defined by the specified coordinate range.
 *
 * The texel color is linearly filtered over the entire region.  u, v, and w
 * will wrap around regardless of the texture's wrap mode.
 */
void TexturePeeker::
filter_rect(LColor &color,
            PN_stdfloat min_u, PN_stdfloat min_v, PN_stdfloat max_u, PN_stdfloat max_v) const {
  int min_x, max_x;
  init_rect_minmax(min_x, max_x, min_u, max_u, _x_size);

  int min_y, max_y;
  init_rect_minmax(min_y, max_y, min_v, max_v, _y_size);

  color.set(0.0f, 0.0f, 0.0f, 0.0f);
  PN_stdfloat net = 0.0f;
  accum_filter_y(color, net, 0,
                 min_x, max_x, min_u, max_u,
                 min_y, max_y, min_v, max_v,
                 1.0f);

  if (net != 0.0f) {
    color /= net;
  }
}

/**
 * Fills "color" with the average RGBA color of the texels within the
 * rectangle defined by the specified coordinate range.
 *
 * The texel color is linearly filtered over the entire region.  u, v, and w
 * will wrap around regardless of the texture's wrap mode.
 */
void TexturePeeker::
filter_rect(LColor &color,
            PN_stdfloat min_u, PN_stdfloat min_v, PN_stdfloat min_w,
            PN_stdfloat max_u, PN_stdfloat max_v, PN_stdfloat max_w) const {
  int min_x, max_x;
  init_rect_minmax(min_x, max_x, min_u, max_u, _x_size);

  int min_y, max_y;
  init_rect_minmax(min_y, max_y, min_v, max_v, _y_size);

  int min_z, max_z;
  init_rect_minmax(min_z, max_z, min_w, max_w, _z_size);

  color.set(0.0f, 0.0f, 0.0f, 0.0f);
  PN_stdfloat net = 0.0f;
  accum_filter_z(color, net,
                 min_x, max_x, min_u, max_u,
                 min_y, max_y, min_v, max_v,
                 min_z, max_z, min_w, max_w);

  if (net != 0.0f) {
    color /= net;
  }
}

/**
 * Sanity-checks min_u, max_u and computes min_x and min_y based on them.
 * Also works for y and z.
 */
void TexturePeeker::
init_rect_minmax(int &min_x, int &max_x, PN_stdfloat &min_u, PN_stdfloat &max_u,
                 int x_size) {
  if (min_u > max_u) {
    PN_stdfloat t = min_u;
    min_u = max_u;
    max_u = t;
  }
  if (max_u - min_u >= 1.0f) {
    min_u = 0.0f;
    max_u = 1.0f;
  }
  min_x = (int)cfloor(min_u * (PN_stdfloat)x_size);
  max_x = (int)cceil(max_u * (PN_stdfloat)x_size);
  nassertv(min_x <= max_x);
}

/**
 * Accumulates the range of pixels from min_z to max_z.
 */
void TexturePeeker::
accum_filter_z(LColor &color, PN_stdfloat &net,
               int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
               int min_y, int max_y, PN_stdfloat min_v, PN_stdfloat max_v,
               int min_z, int max_z, PN_stdfloat min_w, PN_stdfloat max_w) const {
  nassertv(min_z >= 0 && min_z <= _z_size &&
           max_z >= 0 && max_z <= _z_size);
  int zi = min_z;

  if (min_z >= max_z - 1) {
    // Within a single texel.
    accum_filter_y(color, net, zi % _z_size,
                   min_x, max_x, min_u, max_u,
                   min_y, max_y, min_v, max_v,
                   1.0f);

  } else {
    // First part-texel.
    PN_stdfloat w = (min_z + 1) - min_w * _z_size;
    accum_filter_y(color, net, zi % _z_size,
                   min_x, max_x, min_u, max_u,
                   min_y, max_y, min_v, max_v,
                   w);
    int zs = max_z - 1;

    // Run of full texels.
    zi = min_z + 1;
    while (zi < zs) {
      accum_filter_y(color, net, zi % _z_size,
                     min_x, max_x, min_u, max_u,
                     min_y, max_y, min_v, max_v,
                     1.0f);
      ++zi;
    }

    // Last part-texel.
    w = max_w * _z_size - (max_z - 1);
    accum_filter_y(color, net, zi % _z_size,
                   min_x, max_x, min_u, max_u,
                   min_y, max_y, min_v, max_v,
                   w);
  }
}

/**
 * Accumulates the range of pixels from min_y to max_y.
 */
void TexturePeeker::
accum_filter_y(LColor &color, PN_stdfloat &net, int zi,
               int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
               int min_y, int max_y, PN_stdfloat min_v, PN_stdfloat max_v,
               PN_stdfloat weight) const {
  nassertv(zi >= 0 && zi < _z_size);
  nassertv(min_y >= 0 && min_y <= _y_size &&
           max_y >= 0 && max_y <= _y_size);
  int yi = min_y;

  if (min_y >= max_y - 1) {
    // Within a single texel.
    accum_filter_x(color, net, yi % _y_size, zi, min_x, max_x, min_u, max_u, weight);

  } else {
    // First part-texel.
    PN_stdfloat w = (min_y + 1) - min_v * _y_size;
    accum_filter_x(color, net, yi % _y_size, zi, min_x, max_x, min_u, max_u, weight * w);
    int ys = max_y - 1;

    // Run of full texels.
    yi = min_y + 1;
    while (yi < ys) {
      accum_filter_x(color, net, yi % _y_size, zi, min_x, max_x, min_u, max_u, weight);
      ++yi;
    }

    // Last part-texel.
    w = max_v * _y_size - (max_y - 1);
    accum_filter_x(color, net, yi % _y_size, zi, min_x, max_x, min_u, max_u, weight * w);
  }
}

/**
 * Accumulates the range of pixels from min_x to max_x.
 */
void TexturePeeker::
accum_filter_x(LColor &color, PN_stdfloat &net, int yi, int zi,
               int min_x, int max_x, PN_stdfloat min_u, PN_stdfloat max_u,
               PN_stdfloat weight) const {
  nassertv(yi >= 0 && yi < _y_size && zi >= 0 && zi < _z_size);
  nassertv(min_x >= 0 && min_x <= _x_size &&
           max_x >= 0 && max_x <= _x_size);

  // Compute the p corresponding to min_x.
  int xi = min_x % _x_size;
  const unsigned char *p = _image.p() + (zi * _x_size * _y_size + yi * _x_size + xi) * _pixel_width;

  if (min_x >= max_x - 1) {
    // Within a single texel.
    accum_texel(color, net, p, weight);

  } else {
    // First part-texel.
    PN_stdfloat w = (min_x + 1) - min_u * _x_size;
    accum_texel(color, net, p, weight * w);
    int xs = max_x - 1;

    // Run of full texels.
    xi = min_x + 1;
    while (xi < xs) {
      if (xi == _x_size) {
        xi = 0;
        p = _image.p() + (zi * _x_size * _y_size + yi * _x_size + xi) * _pixel_width;
        xs -= _x_size;
      }
      accum_texel(color, net, p, weight);
      ++xi;
    }

    // Last part-texel.
    if (xi == _x_size) {
      xi = 0;
      p = _image.p() + (zi * _x_size * _y_size + yi * _x_size + xi) * _pixel_width;
    }
    w = max_u * _x_size - (max_x - 1);
    accum_texel(color, net, p, weight * w);
  }
}

/**
 * Accumulates a single texel into the total computed by filter_rect().
 */
void TexturePeeker::
accum_texel(LColor &color, PN_stdfloat &net, const unsigned char *&p, PN_stdfloat weight) const {
  LColor c;
  (*_get_texel)(c, p, _get_component);
  color += c * weight;
  net += weight;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_red.
 */
void TexturePeeker::
get_texel_r(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = (*get_component)(p);
  color[1] = 0.0f;
  color[2] = 0.0f;
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_green.
 */
void TexturePeeker::
get_texel_g(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = 0.0f;
  color[1] = (*get_component)(p);
  color[2] = 0.0f;
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_blue.
 */
void TexturePeeker::
get_texel_b(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = 0.0f;
  color[1] = 0.0f;
  color[2] = (*get_component)(p);
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_alpha.
 */
void TexturePeeker::
get_texel_a(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = 0.0f;
  color[1] = 0.0f;
  color[2] = 1.0f;
  color[3] = (*get_component)(p);
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_luminance.
 */
void TexturePeeker::
get_texel_l(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = (*get_component)(p);
  color[1] = color[0];
  color[2] = color[0];
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_luminance_alpha or similar.
 */
void TexturePeeker::
get_texel_la(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = (*get_component)(p);
  color[1] = color[0];
  color[2] = color[0];
  color[3] = (*get_component)(p);
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_rg or similar.
 */
void TexturePeeker::
get_texel_rg(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[0] = (*get_component)(p);
  color[1] = (*get_component)(p);
  color[2] = 0.0f;
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_rgb or similar.
 */
void TexturePeeker::
get_texel_rgb(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[2] = (*get_component)(p);
  color[1] = (*get_component)(p);
  color[0] = (*get_component)(p);
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_rgba or similar.
 */
void TexturePeeker::
get_texel_rgba(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[2] = (*get_component)(p);
  color[1] = (*get_component)(p);
  color[0] = (*get_component)(p);
  color[3] = (*get_component)(p);
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_srgb or similar.
 */
void TexturePeeker::
get_texel_srgb(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[2] = decode_sRGB_float(*p++);
  color[1] = decode_sRGB_float(*p++);
  color[0] = decode_sRGB_float(*p++);
  color[3] = 1.0f;
}

/**
 * Gets the color of the texel at byte p, given that the texture is in format
 * F_srgb_alpha or similar.
 */
void TexturePeeker::
get_texel_srgba(LColor &color, const unsigned char *&p, GetComponentFunc *get_component) {
  color[2] = decode_sRGB_float(*p++);
  color[1] = decode_sRGB_float(*p++);
  color[0] = decode_sRGB_float(*p++);
  color[3] = (*get_component)(p);
}
