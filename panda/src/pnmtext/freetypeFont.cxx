/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file freetypeFont.cxx
 * @author drose
 * @date 2003-09-07
 */

#include "freetypeFont.h"

#ifdef HAVE_FREETYPE

#include "config_pnmtext.h"
#include "config_putil.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "nurbsCurveEvaluator.h"
#include "nurbsCurveResult.h"

#undef interface  // I don't know where this symbol is defined, but it interferes with FreeType.
#include FT_OUTLINE_H

using std::istream;
using std::ostream;
using std::string;

// This constant determines how big a particular point size font appears to
// be.  By convention, 10 points is 1 unit (e.g.  1 foot) high.
const PN_stdfloat FreetypeFont::_points_per_unit = 10.0f;

// A universal typographic convention.
const PN_stdfloat FreetypeFont::_points_per_inch = 72.0f;

/**
 *
 */
FreetypeFont::
FreetypeFont() {
  _face = nullptr;

  _point_size = text_point_size;
  _requested_pixels_per_unit = text_pixels_per_unit;
  _tex_pixels_per_unit = text_pixels_per_unit;
  _requested_scale_factor = text_scale_factor;
  _scale_factor = text_scale_factor;
  _native_antialias = text_native_antialias;
  _winding_order = WO_default;

  _line_height = 1.0f;
  _space_advance = 0.25f;

  _char_size = 0;
  _dpi = 0;
  _pixel_width = 0;
  _pixel_height = 0;
}

/**
 *
 */
FreetypeFont::
FreetypeFont(const FreetypeFont &copy) :
  Namable(copy),
  _point_size(copy._point_size),
  _requested_pixels_per_unit(copy._requested_pixels_per_unit),
  _tex_pixels_per_unit(copy._tex_pixels_per_unit),
  _requested_scale_factor(copy._requested_scale_factor),
  _scale_factor(copy._scale_factor),
  _native_antialias(copy._native_antialias),
  _font_pixels_per_unit(copy._font_pixels_per_unit),
  _winding_order(copy._winding_order),
  _line_height(copy._line_height),
  _space_advance(copy._space_advance),
  _face(copy._face),
  _char_size(copy._char_size),
  _dpi(copy._dpi),
  _pixel_width(copy._pixel_width),
  _pixel_height(copy._pixel_height)
{
}

/**
 * This method accepts the name of some font file that FreeType can read,
 * along with face_index, indicating which font within the file to load
 * (usually 0).
 */
bool FreetypeFont::
load_font(const Filename &font_filename, int face_index) {
  unload_font();
  _face = new FreetypeFace;
  if (!_face->_ft_ok) {
    pnmtext_cat.error()
      << "Unable to read font " << font_filename
      << ": FreeType library not initialized properly.\n";
    unload_font();
    return false;
  }

  bool exists = false;
  int error;
  Filename path(font_filename);
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  vfs->resolve_filename(path, get_model_path());
  exists = vfs->read_file(path, _face->_font_data, true);
  if (exists) {
    FT_Face face;
    error = FT_New_Memory_Face(_face->_ft_library,
                               (const FT_Byte *)_face->_font_data.data(),
                               _face->_font_data.length(),
                               face_index, &face);
    _face->set_face(face);
  }

  bool okflag = false;
  if (!exists) {
    pnmtext_cat.error()
      << "Unable to find font file " << font_filename << "\n";
  } else {
    if (error == FT_Err_Unknown_File_Format) {
      pnmtext_cat.error()
        << "Unable to read font " << font_filename << ": unknown file format.\n";
    } else if (error) {
      pnmtext_cat.error()
        << "Unable to read font " << font_filename << ": invalid.\n";

    } else {
      okflag = reset_scale();
    }
  }

  if (!okflag) {
    unload_font();
  }

  return okflag;
}

/**
 * This method accepts a table of data representing the font file, loaded from
 * some source other than a filename on disk.
 */
bool FreetypeFont::
load_font(const char *font_data, int data_length, int face_index) {
  unload_font();
  _face = new FreetypeFace;

  if (!_face->_ft_ok) {
    pnmtext_cat.error()
      << "Unable to read font: FreeType library not initialized properly.\n";
    unload_font();
    return false;
  }

  int error;
  FT_Face face;
  error = FT_New_Memory_Face(_face->_ft_library,
                             (const FT_Byte *)font_data, data_length,
                             face_index, &face);
  _face->set_face(face);

  bool okflag = false;
  if (error == FT_Err_Unknown_File_Format) {
    pnmtext_cat.error()
      << "Unable to read font: unknown file format.\n";
  } else if (error) {
    pnmtext_cat.error()
      << "Unable to read font: invalid.\n";

  } else {
    okflag = reset_scale();
  }

  if (!okflag) {
    unload_font();
  }

  return okflag;
}

/**
 *
 */
void FreetypeFont::
unload_font() {
  _face = nullptr;
}

/**
 * Returns the WindingOrder value associated with the given string
 * representation, or WO_invalid if the string does not match any known
 * WindingOrder value.
 */
FreetypeFont::WindingOrder FreetypeFont::
string_winding_order(const string &string) {
  if (cmp_nocase_uh(string, "default") == 0) {
    return WO_default;
  } else if (cmp_nocase_uh(string, "left") == 0) {
    return WO_left;
  } else if (cmp_nocase_uh(string, "right") == 0) {
    return WO_right;
  } else {
    return WO_invalid;
  }
}

/**
 * Invokes Freetype to load and render the indicated glyph into a bitmap.
 * Returns true if successful, false otherwise.
 */
bool FreetypeFont::
load_glyph(FT_Face face, int glyph_index, bool prerender) {
  int flags = FT_LOAD_RENDER;
  if (!_native_antialias) {
    flags |= FT_LOAD_MONOCHROME;
  }

  if (!prerender) {
    // If we want to render as an outline font, don't pre-render it to a
    // bitmap.
    flags = 0;
  }

  int error = FT_Load_Glyph(face, glyph_index, flags);
  if (error) {
    pnmtext_cat.error()
      << "Unable to render glyph " << glyph_index << "\n";
    return false;
  }
  return true;
}

/**
 * Copies a bitmap as rendered by FreeType into a PNMImage, so it can be
 * rescaled.
 */
void FreetypeFont::
copy_bitmap_to_pnmimage(const FT_Bitmap &bitmap, PNMImage &image) {
  if (bitmap.pixel_mode == ft_pixel_mode_grays &&
      bitmap.num_grays == (int)image.get_maxval() + 1) {
    // This is the easy case: we can copy the rendered glyph directly into our
    // image, one pixel at a time.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {
      for (int xi = 0; xi < (int)bitmap.width; xi++) {
        image.set_gray_val(xi, yi, buffer_row[xi]);
      }
      buffer_row += bitmap.pitch;
    }

  } else if (bitmap.pixel_mode == ft_pixel_mode_mono) {
    // This is a little bit more work: we have to expand the one-bit-per-pixel
    // bitmap into a one-byte-per-pixel image.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {
      xelval maxval = image.get_maxval();
      int bit = 0x80;
      unsigned char *b = buffer_row;
      for (int xi = 0; xi < (int)bitmap.width; xi++) {
        if (*b & bit) {
          image.set_gray_val(xi, yi, maxval);
        } else {
          image.set_gray_val(xi, yi, 0);
        }
        bit >>= 1;
        if (bit == 0) {
          ++b;
          bit = 0x80;
        }
      }

      buffer_row += bitmap.pitch;
    }


  } else if (bitmap.pixel_mode == ft_pixel_mode_grays) {
    // Here we must expand a grayscale pixmap with n levels of gray into our
    // 256-level texture.
    unsigned char *buffer_row = bitmap.buffer;
    for (int yi = 0; yi < (int)bitmap.rows; yi++) {
      for (int xi = 0; xi < (int)bitmap.width; xi++) {
        image.set_gray(xi, yi, (PN_stdfloat)buffer_row[xi] / (bitmap.num_grays - 1));
      }
      buffer_row += bitmap.pitch;
    }

  } else {
    pnmtext_cat.error()
      << "Unexpected pixel mode in bitmap: " << (int)bitmap.pixel_mode << "\n";
  }
}

/**
 * Resets the font based on the current values for _point_size,
 * _tex_pixels_per_unit, and _scale_factor.  Returns true if successful, false
 * otherwise.
 */
bool FreetypeFont::
reset_scale() {
  if (_face == nullptr) {
    return false;
  }

  // Get the face, without requesting a particular size yet (we'll figure out
  // the size in a second).
  FT_Face face = _face->acquire_face(0, 0, 0, 0);

  // The font may be rendered larger (by a factor of _scale_factor), and then
  // reduced into the texture.  Hence the difference between
  // _font_pixels_per_unit and _tex_pixels_per_unit.
  _tex_pixels_per_unit = _requested_pixels_per_unit;
  _scale_factor = _requested_scale_factor;
  _font_pixels_per_unit = _tex_pixels_per_unit * _scale_factor;

  _pixel_height = 0;
  _pixel_width = 0;
  PN_stdfloat units_per_inch = (_points_per_inch / _points_per_unit);
  _dpi = (int)(_font_pixels_per_unit * units_per_inch);
  _char_size = (int)(_point_size * 64);

  int error = FT_Set_Char_Size(face, _char_size, _char_size, _dpi, _dpi);
  if (error) {
    // If we were unable to set a particular char size, perhaps we have a non-
    // scalable font.  Try to figure out the next larger available size, or
    // the largest size available if nothing is larger.
    int desired_height = (int)(_font_pixels_per_unit * _point_size / _points_per_unit + 0.5f);
    int best_size = -1;
    int largest_size = -1;
    if (face->num_fixed_sizes > 0) {
      largest_size = 0;
      int best_diff = 0;
      for (int i = 0; i < face->num_fixed_sizes; i++) {
        int diff = face->available_sizes[i].height - desired_height;
        if (diff > 0 && (best_size == -1 || diff < best_diff)) {
          best_size = i;
          best_diff = diff;
        }
        if (face->available_sizes[i].height > face->available_sizes[largest_size].height) {
          largest_size = i;
        }
      }
    }
    if (best_size < 0) {
      best_size = largest_size;
    }

    if (best_size >= 0) {
      _pixel_height = face->available_sizes[best_size].height;
      _pixel_width = face->available_sizes[best_size].width;
      error = FT_Set_Pixel_Sizes(face, _pixel_width, _pixel_height);
      if (!error) {
        _font_pixels_per_unit = _pixel_height * _points_per_unit / _point_size;
        _scale_factor = _font_pixels_per_unit / _tex_pixels_per_unit;

        if (_scale_factor < 1.0) {
          // No point in enlarging a fixed-point font.
          _scale_factor = 1.0;
          _tex_pixels_per_unit = _font_pixels_per_unit;
        }
      }
    }
  }

  if (error) {
    pnmtext_cat.warning()
      << "Unable to set " << get_name()
      << " to " << _point_size << "pt at " << _dpi << " dpi.\n";
    _line_height = 1.0f;
    _face->release_face(face);
    return false;
  }

  _line_height = face->size->metrics.height / (_font_pixels_per_unit * 64.0f);

  // Determine the correct width for a space.
  error = FT_Load_Char(face, ' ', FT_LOAD_DEFAULT);
  if (error) {
    // Space isn't defined.  Oh well.
    _space_advance = 0.25f * _line_height;

  } else {
    _space_advance = face->glyph->advance.x / (_font_pixels_per_unit * 64.0f);
  }

  _face->release_face(face);
  return true;
}

/**
 * Renders a signed distance field to the PNMImage based on the contours.
 */
void FreetypeFont::
render_distance_field(PNMImage &image, int outline, int min_x, int min_y) {
  Contours::const_iterator ci;

  PN_stdfloat offset_x = -outline / _tex_pixels_per_unit;
  PN_stdfloat offset_y = (image.get_y_size() - 1 - outline) / _tex_pixels_per_unit;;

  offset_x += min_x / (64.0f * _font_pixels_per_unit);
  offset_y += min_y / (64.0f * _font_pixels_per_unit);

  PN_stdfloat scale = _tex_pixels_per_unit / (outline * 2);

  for (int y = 0; y < image.get_y_size(); ++y) {
    LPoint2 p(0, offset_y - (y / _tex_pixels_per_unit));

    for (int x = 0; x < image.get_x_size(); ++x) {
      p[0] = offset_x + (x / _tex_pixels_per_unit);

      PN_stdfloat min_dist_sq = 100000000;
      int winding_number = 0;

      for (ci = _contours.begin(); ci != _contours.end(); ++ci) {
        // Find the shortest distance between this point and the contour.
        // Also keep track of the winding number, so we will know whether this
        // point is inside or outside the polygon.
        const Contour &contour = (*ci);

        for (size_t i = 1; i < contour._points.size(); ++i) {
          const LPoint2 &begin = contour._points[i - 1]._p;
          const LPoint2 &end = contour._points[i]._p;
          PN_stdfloat radius = contour._points[i]._radius;

          LVector2 v = end - begin;
          PN_stdfloat length_sq = v.length_squared();
          PN_stdfloat dist_sq;

          if (length_sq == 0) {
            dist_sq = (p - begin).length_squared();

          } else if (radius != 0) {
            // Circular arc approximation.
            LVector2 v1 = begin - contour._points[i]._center;
            LVector2 v2 = end - contour._points[i]._center;
            LVector2 vp = p - contour._points[i]._center;
            PN_stdfloat dist_to_center = vp.length();
            vp /= dist_to_center;
            v1 /= radius;
            v2 /= radius;
            PN_stdfloat range = v1.dot(v2);
            if (vp.dot(v1) > range && vp.dot(v2) > range) {
              dist_sq = dist_to_center - radius;
              bool inside = dist_sq < 0;
              dist_sq *= dist_sq;

              // if (v1[0] * vp[1] - vp[0] * v1[1] < 0 && v2[0] * vp[1] -
              // vp[0] * v2[1] < 0) { if (v1.signed_angle_deg(vp) <
              // v1.signed_angle_deg(v2) && v1.signed_angle_deg(vp) > 0) {
              if (begin[1] <= p[1]) {
                if (end[1] > p[1]) {
                  if (inside != (v[0] * v1[1] > v[1] * v1[0])) {
                    ++winding_number;
                  }
                }
              } else {
                if (end[1] <= p[1]) {
                  if (inside == (v[0] * v1[1] > v[1] * v1[0])) {
                    --winding_number;
                  }
                }
              }

            } else {
              dist_sq = std::min((p - begin).length_squared(), (p - end).length_squared());
              if (begin[1] <= p[1]) {
                if (end[1] > p[1]) {
                  if ((v[0] * (p[1] - begin[1]) > v[1] * (p[0] - begin[0]))) {
                    ++winding_number;
                  }
                }
              } else {
                if (end[1] <= p[1]) {
                  if ((v[0] * (p[1] - begin[1]) < v[1] * (p[0] - begin[0]))) {
                    --winding_number;
                  }
                }
              }
            }

          } else {
            // Just a straight line.
            if (begin[1] <= p[1]) {
              if (end[1] > p[1]) {
                if ((v[0] * (p[1] - begin[1]) > v[1] * (p[0] - begin[0]))) {
                  ++winding_number;
                }
              }
            } else {
              if (end[1] <= p[1]) {
                if ((v[0] * (p[1] - begin[1]) < v[1] * (p[0] - begin[0]))) {
                  --winding_number;
                }
              }
            }

            PN_stdfloat t = v.dot(p - begin) / length_sq;
            if (t <= 0.0) {
              dist_sq = (p - begin).length_squared();
            } else if (t >= 1.0) {
              dist_sq = (p - end).length_squared();
            } else {
              dist_sq = (p - (begin + v * t)).length_squared();
            }
          }

          min_dist_sq = std::min(min_dist_sq, dist_sq);
        }
      }
      // Determine the sign based on whether we're inside the contour.
      int sign = (winding_number != 0) ? 1 : -1;

      PN_stdfloat signed_dist = csqrt(min_dist_sq) * sign;
      image.set_gray(x, y, signed_dist * scale + (PN_stdfloat)0.5);
    }
  }
}

/**
 * Ask FreeType to extract the contours out of the outline description.
 */
void FreetypeFont::
decompose_outline(FT_Outline &outline) {
  FT_Outline_Funcs funcs;
  memset(&funcs, 0, sizeof(funcs));
  funcs.move_to = (FT_Outline_MoveTo_Func)outline_move_to;
  funcs.line_to = (FT_Outline_LineTo_Func)outline_line_to;
  funcs.conic_to = (FT_Outline_ConicTo_Func)outline_conic_to;
  funcs.cubic_to = (FT_Outline_CubicTo_Func)outline_cubic_to;

  WindingOrder wo = _winding_order;
  if (wo == WO_default) {
    // If we weren't told an explicit winding order, ask FreeType to figure it
    // out.  Sometimes it appears to guess wrong.
#ifdef FT_ORIENTATION_FILL_RIGHT
    if (FT_Outline_Get_Orientation(&outline) == FT_ORIENTATION_FILL_RIGHT) {
      wo = WO_right;
    } else {
      wo = WO_left;
    }
#else
    // Hmm.  Assign a right-winding (TTF) orientation if FreeType can't tell
    // us.
    wo = WO_right;
#endif  // FT_ORIENTATION_FILL_RIGHT
  }

  if (wo != WO_left) {
    FT_Outline_Reverse(&outline);
  }

  _contours.clear();
  FT_Outline_Decompose(&outline, &funcs, (void *)this);
}

/**
 * A callback from FT_Outline_Decompose().  It marks the beginning of a new
 * contour.
 */
int FreetypeFont::
outline_move_to(const FT_Vector *to, void *user) {
  FreetypeFont *self = (FreetypeFont *)user;

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  if (self->_contours.empty() ||
      !self->_contours.back()._points.empty()) {
    self->_contours.push_back(Contour());
  }
  self->_q = p;
  return 0;
}

/**
 * A callback from FT_Outline_Decompose().  It marks a straight line in the
 * contour.
 */
int FreetypeFont::
outline_line_to(const FT_Vector *to, void *user) {
  FreetypeFont *self = (FreetypeFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // Compute the tangent: this is just the vector from the last point.
  LVector2 t = (p - self->_q);
  t.normalize();

  if (self->_contours.back()._points.empty()) {
    self->_contours.back()._points.push_back(ContourPoint(self->_q, LVector2::zero(), t));
  } else {
    self->_contours.back()._points.back().connect_to(t);
  }

  self->_contours.back()._points.push_back(ContourPoint(p, t, LVector2::zero()));
  self->_q = p;
  return 0;
}

/**
 * A callback from FT_Outline_Decompose().  It marks a parabolic (3rd-order)
 * Bezier curve in the contour.
 */
int FreetypeFont::
outline_conic_to(const FT_Vector *control,
                 const FT_Vector *to, void *user) {
  FreetypeFont *self = (FreetypeFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2 c = LPoint2(control->x, control->y) * scale;
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(3);
  nce.reset(3);
  nce.set_vertex(0, LVecBase3(self->_q[0], self->_q[1], 0.0f));
  nce.set_vertex(1, LVecBase3(c[0], c[1], 0.0f));
  nce.set_vertex(2, LVecBase3(p[0], p[1], 0.0f));

  self->_q = p;

  PT(NurbsCurveResult) ncr = nce.evaluate();
  return self->outline_nurbs(ncr);
}

/**
 * A callback from FT_Outline_Decompose().  It marks a cubic (4th-order)
 * Bezier curve in the contour.
 */
int FreetypeFont::
outline_cubic_to(const FT_Vector *control1, const FT_Vector *control2,
                 const FT_Vector *to, void *user) {
  FreetypeFont *self = (FreetypeFont *)user;
  nassertr(!self->_contours.empty(), 1);

  // Convert from 26.6 pixel units to Panda units.
  PN_stdfloat scale = 1.0f / (64.0f * self->_font_pixels_per_unit);

  LPoint2 c1 = LPoint2(control1->x, control1->y) * scale;
  LPoint2 c2 = LPoint2(control2->x, control2->y) * scale;
  LPoint2 p = LPoint2(to->x, to->y) * scale;

  // The NurbsCurveEvaluator will evaluate the Bezier segment for us.
  NurbsCurveEvaluator nce;
  nce.local_object();
  nce.set_order(4);
  nce.reset(4);
  nce.set_vertex(0, LVecBase3(self->_q[0], self->_q[1], 0.0f));
  nce.set_vertex(1, LVecBase3(c1[0], c1[1], 0.0f));
  nce.set_vertex(2, LVecBase3(c2[0], c2[1], 0.0f));
  nce.set_vertex(3, LVecBase3(p[0], p[1], 0.0f));

  self->_q = p;

  PT(NurbsCurveResult) ncr = nce.evaluate();
  return self->outline_nurbs(ncr);
}

/**
 * Called internally by outline_cubic_to() and outline_conic_to().
 */
int FreetypeFont::
outline_nurbs(NurbsCurveResult *ncr) {
  // Sample it down so that the lines approximate the curve to within a
  // "pixel."
  ncr->adaptive_sample(1.0f / _font_pixels_per_unit);

  int num_samples = ncr->get_num_samples();

  bool needs_connect = false;
  int start = 1;
  if (_contours.back()._points.empty()) {
    // If we haven't got the first point of this contour yet, we must add it
    // now.
    start = 0;
  } else {
    needs_connect = true;
  }

  for (int i = start; i < num_samples; ++i) {
    PN_stdfloat st = ncr->get_sample_t(i);
    const LPoint3 &p = ncr->get_sample_point(i);

    PN_stdfloat st0 = st, st1 = st;
    if (i > 0) {
      st0 = ncr->get_sample_t(i - 1) * 0.1f + st * 0.9f;
    }
    if (i < num_samples - 1) {
      st1 = ncr->get_sample_t(i + 1) * 0.1f + st * 0.9f;
    }
    // Compute the tangent by deltaing nearby points.  Don't evaluate the
    // tangent from the NURBS, since that doesn't appear to be reliable.
    LPoint3 p0, p1;
    ncr->eval_point(st0, p0);
    ncr->eval_point(st1, p1);
    LVector3 t = p1 - p0;
    t.normalize();

    if (needs_connect) {
      _contours.back()._points.back().connect_to(LVector2(t[0], t[1]));
      needs_connect = false;
    }

    _contours.back()._points.push_back(ContourPoint(p[0], p[1], t[0], t[1]));

    if (i > 0) {
      // Approximate the curve using a circular arc.  This is used in the
      // signed distance field generation code.  We do this by sampling a
      // point in the middle of the segment and calculating the circle that
      // goes through the three points.
      LPoint2 v1 = ncr->get_sample_point(i - 1).get_xy();
      LPoint3 v2;
      ncr->eval_point((ncr->get_sample_t(i - 1) + ncr->get_sample_t(i)) / 2, v2);
      PN_stdfloat temp = v1.length_squared();
      PN_stdfloat bc = (v2[0]*v2[0] + v2[1]*v2[1] - temp) * (PN_stdfloat)0.5f;
      PN_stdfloat cd = (temp - p[0]*p[0] - p[1]*p[1]) * (PN_stdfloat)0.5f;
      PN_stdfloat det = (v2[0]-v1[0])*(v1[1]-p[1])-(v1[0]-p[0])*(v2[1]-v1[1]);
      if (!IS_NEARLY_ZERO(det)) {
        LPoint2 center;
        center[0] = (bc*(v1[1]-p[1])-cd*(v2[1]-v1[1]));
        center[1] = ((v2[0]-v1[0])*cd-(v1[0]-p[0])*bc);
        center /= det;
        _contours.back()._points.back()._center = center;
        _contours.back()._points.back()._radius = (center - v1).length();
      }
    }
  }

  return 0;
}

/**
 *
 */
ostream &
operator << (ostream &out, FreetypeFont::WindingOrder wo) {
  switch (wo) {
  case FreetypeFont::WO_default:
    return out << "default";
  case FreetypeFont::WO_left:
    return out << "left";
  case FreetypeFont::WO_right:
    return out << "right";

  case FreetypeFont::WO_invalid:
    return out << "invalid";
  }

  return out << "(**invalid FreetypeFont::WindingOrder(" << (int)wo << ")**)";
}

/**
 *
 */
istream &
operator >> (istream &in, FreetypeFont::WindingOrder &wo) {
  string word;
  in >> word;

  wo = FreetypeFont::string_winding_order(word);
  return in;
}


#endif  // HAVE_FREETYPE
