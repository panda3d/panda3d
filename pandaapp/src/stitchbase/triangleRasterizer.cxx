// Filename: triangleRasterizer.cxx
// Created by:  drose (06Nov99)
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

#include "triangleRasterizer.h"
#include "stitchImage.h"

// Inline function declared up here for the forward reference.
inline void TriangleRasterizer::
filter_pixel(RGBColord &rgb, double &alpha,
             double s, double t,
             double dsdx, double dtdx, double dsdy, double dtdy) {
  filter_pixel(rgb, alpha, s, t,
               max(max(dsdx, dtdx), max(dsdy, dtdy)) / 2.0);
}


TriangleRasterizer::Edge::
Edge(const RasterizerVertex *v0, const RasterizerVertex *v1) :
  _v0(v0), _v1(v1)
{
  _dx = v1->_p[0] - v0->_p[0];
  _dy = v1->_p[1] - v0->_p[1];
}

TriangleRasterizer::
TriangleRasterizer() {
  _output = NULL;
  _input = NULL;
  _read_input = false;
  _texture = NULL;
  _filter_output = false;
  _untextured_color.set(1.0, 1.0, 1.0, 1.0);
}

void TriangleRasterizer::
draw_triangle(const RasterizerVertex *v0,
              const RasterizerVertex *v1,
              const RasterizerVertex *v2) {
  if ((v0->_visibility & v1->_visibility & v2->_visibility) != 0) {
    // All three vertices are out of bounds in the same direction, so
    // the triangle is completely out of bounds.  Don't bother trying
    // to draw it.
    return;
  }
  if (v0->_visibility < 0 || v1->_visibility < 0 || v2->_visibility < 0) {
    // At least one vertex is totally bogus, so throw up our hands on
    // the triangle.
    return;
  }

  assert(_output != NULL);
  if (!_read_input) {
    read_input();
  }

  double oneOverArea;
  const RasterizerVertex *vMin, *vMid, *vMax;
  /* Y(vMin)<=Y(vMid)<=Y(vMax) */

   /* find the order of the 3 vertices along the Y axis */
  {
    double y0 = v0->_p[1];
    double y1 = v1->_p[1];
    double y2 = v2->_p[1];

    if (y0<=y1) {
      if (y1<=y2) {
        vMin = v0;   vMid = v1;   vMax = v2;   /* y0<=y1<=y2 */
      } else if (y2<=y0) {
        vMin = v2;   vMid = v0;   vMax = v1;   /* y2<=y0<=y1 */
      } else {
        vMin = v0;   vMid = v2;   vMax = v1;   /* y0<=y2<=y1 */
      }
    } else {
      if (y0<=y2) {
        vMin = v1;   vMid = v0;   vMax = v2;   /* y1<=y0<=y2 */
      } else if (y2<=y1) {
        vMin = v2;   vMid = v1;   vMax = v0;   /* y2<=y1<=y0 */
      } else {
        vMin = v1;   vMid = v2;   vMax = v0;   /* y1<=y2<=y0 */
      }
    }
  }

  /* vertex/edge relationship */
  Edge eMaj(vMin, vMax);
  Edge eTop(vMid, vMax);
  Edge eBot(vMin, vMid);

  /* compute oneOverArea */
  {
    double area = eMaj._dx * eBot._dy - eBot._dx * eMaj._dy;

    // We can't cull very small triangles; we might generate small
    // triangles through normal operations.

    /*
    if (area>-0.05 && area<0.05) {
      return;  // very small; CULLED
    }
    */
    oneOverArea = 1.0 / area;
  }

  /* Edge setup.  For a triangle strip these could be reused... */
  {
    /* fixed point Y coordinates */
    FixedPoint vMin_fx = FloatToFixed(vMin->_p[0] + 0.5);
    FixedPoint vMin_fy = FloatToFixed(vMin->_p[1] - 0.5);
    FixedPoint vMid_fx = FloatToFixed(vMid->_p[0] + 0.5);
    FixedPoint vMid_fy = FloatToFixed(vMid->_p[1] - 0.5);
    FixedPoint vMax_fy = FloatToFixed(vMax->_p[1] - 0.5);

    eMaj._fsy = FixedCeil(vMin_fy);
    eMaj._lines = FixedToInt(vMax_fy + FIXED_ONE - FIXED_EPSILON - eMaj._fsy);
    if (eMaj._lines > 0) {
      double dxdy = eMaj._dx / eMaj._dy;
      eMaj._fdxdy = SignedFloatToFixed(dxdy);
      eMaj._adjy = (double) (eMaj._fsy - vMin_fy);  /* SCALED! */
      eMaj._fx0 = vMin_fx;
      eMaj._fsx = eMaj._fx0 + (FixedPoint) (eMaj._adjy * dxdy);
    }
    else {
      return;  /*CULLED*/
    }

    eTop._fsy = FixedCeil(vMid_fy);
    eTop._lines = FixedToInt(vMax_fy + FIXED_ONE - FIXED_EPSILON - eTop._fsy);
    if (eTop._lines > 0) {
      double dxdy = eTop._dx / eTop._dy;
      eTop._fdxdy = SignedFloatToFixed(dxdy);
      eTop._adjy = (double) (eTop._fsy - vMid_fy); /* SCALED! */
      eTop._fx0 = vMid_fx;
      eTop._fsx = eTop._fx0 + (FixedPoint) (eTop._adjy * dxdy);
    }

    eBot._fsy = FixedCeil(vMin_fy);
    eBot._lines = FixedToInt(vMid_fy + FIXED_ONE - FIXED_EPSILON - eBot._fsy);
    if (eBot._lines > 0) {
      double dxdy = eBot._dx / eBot._dy;
      eBot._fdxdy = SignedFloatToFixed(dxdy);
      eBot._adjy = (double) (eBot._fsy - vMin_fy);  /* SCALED! */
      eBot._fx0 = vMin_fx;
      eBot._fsx = eBot._fx0 + (FixedPoint) (eBot._adjy * dxdy);
    }
  }

  /*
    * Conceptually, we view a triangle as two subtriangles
    * separated by a perfectly horizontal line.  The edge that is
    * intersected by this line is one with maximal absolute dy; we
    * call it a ``major'' edge.  The other two edges are the
    * ``top'' edge (for the upper subtriangle) and the ``bottom''
    * edge (for the lower subtriangle).  If either of these two
    * edges is horizontal or very close to horizontal, the
    * corresponding subtriangle might cover zero sample points;
    * we take care to handle such cases, for performance as well
    * as correctness.
    *
    * By stepping rasterization parameters along the major edge,
    * we can avoid recomputing them at the discontinuity where
    * the top and bottom edges meet.  However, this forces us to
    * be able to scan both left-to-right and right-to-left.
    * Also, we must determine whether the major edge is at the
    * left or right side of the triangle.  We do this by
    * computing the magnitude of the cross-product of the major
    * and top edges.  Since this magnitude depends on the sine of
    * the angle between the two edges, its sign tells us whether
    * we turn to the left or to the right when travelling along
    * the major edge to the top edge, and from this we infer
    * whether the major edge is on the left or the right.
    *
    * Serendipitously, this cross-product magnitude is also a
    * value we need to compute the iteration parameter
    * derivatives for the triangle, and it can be used to perform
    * backface culling because its sign tells us whether the
    * triangle is clockwise or counterclockwise.  In this code we
    * refer to it as ``area'' because it's also proportional to
    * the pixel area of the triangle.
    */

  {
    int ltor;           /* true if scanning left-to-right */

    // For interpolating the alpha value.
    double dadx, dady;
    FixedPoint fdadx;

    // For interpolating texture coordinates.
    double dsdx, dsdy;
    FixedPoint fdsdx;
    double dtdx, dtdy;
    FixedPoint fdtdx;

    // Set up values for texture coordinates.

    double twidth, theight;
    if (_texture != NULL) {
      twidth = (double) _texture->get_x_size();
      theight = (double) _texture->get_y_size();
    } else {
      twidth = 1.0;
      theight = 1.0;
    }

    ltor = (oneOverArea < 0.0);

    // More alpha setup.
    {
      double eMaj_da, eBot_da;
      eMaj_da = vMax->_alpha - vMin->_alpha;
      eBot_da = vMid->_alpha - vMin->_alpha;
      dadx = oneOverArea * (eMaj_da * eBot._dy - eMaj._dy * eBot_da);
      fdadx = SignedFloatToFixed(dadx);
      dady = oneOverArea * (eMaj._dx * eBot_da - eMaj_da * eBot._dx);
    }

    // Texture coordinates.
    {
      double eMaj_ds, eBot_ds;
      eMaj_ds = (vMax->_uv[0] - vMin->_uv[0]) * twidth;
      eBot_ds = (vMid->_uv[0] - vMin->_uv[0]) * twidth;

      dsdx = oneOverArea * (eMaj_ds * eBot._dy - eMaj._dy * eBot_ds);
      fdsdx = SignedFloatToFixed(dsdx);
      dsdy = oneOverArea * (eMaj._dx * eBot_ds - eMaj_ds * eBot._dx);
    }
    {
      double eMaj_dt, eBot_dt;
      eMaj_dt = (vMax->_uv[1] - vMin->_uv[1]) * theight;
      eBot_dt = (vMid->_uv[1] - vMin->_uv[1]) * theight;

      dtdx = oneOverArea * (eMaj_dt * eBot._dy - eMaj._dy * eBot_dt);
      fdtdx = SignedFloatToFixed(dtdx);
      dtdy = oneOverArea * (eMaj._dx * eBot_dt - eMaj_dt * eBot._dx);
    }

    /*
     * We always sample at pixel centers.  However, we avoid
     * explicit half-pixel offsets in this code by incorporating
     * the proper offset in each of x and y during the
     * transformation to window coordinates.
     *
     * We also apply the usual rasterization rules to prevent
     * cracks and overlaps.  A pixel is considered inside a
     * subtriangle if it meets all of four conditions: it is on or
     * to the right of the left edge, strictly to the left of the
     * right edge, on or below the top edge, and strictly above
     * the bottom edge.  (Some edges may be degenerate.)
     *
     * The following discussion assumes left-to-right scanning
     * (that is, the major edge is on the left); the right-to-left
     * case is a straightforward variation.
     *
     * We start by finding the half-integral y coordinate that is
     * at or below the top of the triangle.  This gives us the
     * first scan line that could possibly contain pixels that are
     * inside the triangle.
     *
     * Next we creep down the major edge until we reach that y,
     * and compute the corresponding x coordinate on the edge.
     * Then we find the half-integral x that lies on or just
     * inside the edge.  This is the first pixel that might lie in
     * the interior of the triangle.  (We won't know for sure
     * until we check the other edges.)
     *
     * As we rasterize the triangle, we'll step down the major
     * edge.  For each step in y, we'll move an integer number
     * of steps in x.  There are two possible x step sizes, which
     * we'll call the ``inner'' step (guaranteed to land on the
     * edge or inside it) and the ``outer'' step (guaranteed to
     * land on the edge or outside it).  The inner and outer steps
     * differ by one.  During rasterization we maintain an error
     * term that indicates our distance from the true edge, and
     * select either the inner step or the outer step, whichever
     * gets us to the first pixel that falls inside the triangle.
     *
     * All parameters (z, red, etc.) as well as the buffer
     * addresses for color and z have inner and outer step values,
     * so that we can increment them appropriately.  This method
     * eliminates the need to adjust parameters by creeping a
     * sub-pixel amount into the triangle at each scanline.
     */

    {
      int subTriangle;
      FixedPoint fx, fxLeftEdge, fxRightEdge, fdxLeftEdge, fdxRightEdge;
      FixedPoint fdxOuter;
      int idxOuter;
      double dxOuter;
      FixedPoint fError, fdError;
      double adjx, adjy;
      FixedPoint fy;
      int iy;

      // Alpha.
      FixedPoint fa, fdaOuter, fdaInner;

      // Texture coordinates.
      FixedPoint fs, fdsOuter, fdsInner;
      FixedPoint ft, fdtOuter, fdtInner;

      for (subTriangle=0; subTriangle<=1; subTriangle++) {
        Edge *eLeft, *eRight;
        int setupLeft, setupRight;
        int lines;

        if (subTriangle==0) {
          /* bottom half */
          if (ltor) {
            eLeft = &eMaj;
            eRight = &eBot;
            lines = eRight->_lines;
            setupLeft = 1;
            setupRight = 1;
          }
          else {
            eLeft = &eBot;
            eRight = &eMaj;
            lines = eLeft->_lines;
            setupLeft = 1;
            setupRight = 1;
          }
        }
        else {
          /* top half */
          if (ltor) {
            eLeft = &eMaj;
            eRight = &eTop;
            lines = eRight->_lines;
            setupLeft = 0;
            setupRight = 1;
          }
          else {
            eLeft = &eTop;
            eRight = &eMaj;
            lines = eLeft->_lines;
            setupLeft = 1;
            setupRight = 0;
          }
          if (lines==0) return;
        }

        if (setupLeft && eLeft->_lines>0) {
          const RasterizerVertex *vLower;
          FixedPoint fsx = eLeft->_fsx;
          fx = FixedCeil(fsx);
          fError = fx - fsx - FIXED_ONE;
          fxLeftEdge = fsx - FIXED_EPSILON;
          fdxLeftEdge = eLeft->_fdxdy;
          fdxOuter = FixedFloor(fdxLeftEdge - FIXED_EPSILON);
          fdError = fdxOuter - fdxLeftEdge + FIXED_ONE;
          idxOuter = FixedToInt(fdxOuter);
          dxOuter = (double) idxOuter;

          fy = eLeft->_fsy;
          iy = FixedToInt(fy);

          adjx = (double)(fx - eLeft->_fx0);  /* SCALED! */
          adjy = eLeft->_adjy;           /* SCALED! */

          vLower = eLeft->_v0;

          /*
           * Now we need the set of parameter (z, color, etc.) values at
           * the point (fx, fy).  This gives us properly-sampled parameter
           * values that we can step from pixel to pixel.  Furthermore,
           * although we might have intermediate results that overflow
           * the normal parameter range when we step temporarily outside
           * the triangle, we shouldn't overflow or underflow for any
           * pixel that's actually inside the triangle.
           */

          // Interpolate alpha
          fa = (FixedPoint)(vLower->_alpha * FIXED_SCALE + dadx * adjx + dady * adjy)
            + FIXED_HALF;
          fdaOuter = SignedFloatToFixed(dady + dxOuter * dadx);
          // Interpolate texture coordinates
          {
            double s0, t0;
            s0 = vLower->_uv[0] * twidth;
            fs = (FixedPoint)(s0 * FIXED_SCALE + dsdx * adjx + dsdy * adjy) + FIXED_HALF;
            fdsOuter = SignedFloatToFixed(dsdy + dxOuter * dsdx);
            t0 = vLower->_uv[1] * theight;
            ft = (FixedPoint)(t0 * FIXED_SCALE + dtdx * adjx + dtdy * adjy) + FIXED_HALF;
            fdtOuter = SignedFloatToFixed(dtdy + dxOuter * dtdx);
          }

        } /*if setupLeft*/


        if (setupRight && eRight->_lines>0) {
          fxRightEdge = eRight->_fsx - FIXED_EPSILON;
          fdxRightEdge = eRight->_fdxdy;
        }

        if (lines==0) {
          continue;
        }

        /* Rasterize setup */
        fdaInner = fdaOuter + fdadx;
        fdsInner = fdsOuter + fdsdx;
        fdtInner = fdtOuter + fdtdx;

        while (lines>0) {
          if (iy >= 0 && iy < _output->get_y_size()) {
            /* initialize the span interpolants to the leftmost value */
            /* ff = fixed-pt fragment */
            FixedPoint ffa = fa;
            FixedPoint ffs = fs,  fft = ft;

            int left = FixedToInt(fxLeftEdge);
            int right = FixedToInt(fxRightEdge);

            // Alpha
            {
              //              FixedPoint ffaend = ffa+(right-left-1)*fdadx;
              //              if (ffaend<0) ffa -= ffaend;
              //              if (ffa<0) ffa = 0;
            }

            // Rasterize left to right at row iy.
            if (right > left) {
              ffs -= FIXED_HALF; /* off-by-one error? */
              fft -= FIXED_HALF;
              ffa -= FIXED_HALF;
              for (int ix = left; ix < right; ix++) {
                if (ix >= 0 && ix < _output->get_x_size()) {
                  RGBColord rgb;
                  double alpha;
                  filter_pixel(rgb, alpha,
                               FixedToFloat(ffs), FixedToFloat(fft),
                               dsdx, dtdx, dsdy, dtdy);
                  alpha *= FixedToFloat(ffa);
                  _output->blend(ix, iy, rgb, alpha);
                }

                ffs += fdsdx;
                fft += fdtdx;
                ffa += fdadx;
              }
            }
          }

          /*
           * Advance to the next scan line.  Compute the
           * new edge coordinates, and adjust the
           * pixel-center x coordinate so that it stays
           * on or inside the major edge.
           */
          iy++;
          lines--;

          fxLeftEdge += fdxLeftEdge;
          fxRightEdge += fdxRightEdge;

          fError += fdError;
          if (fError >= 0) {
            fError -= FIXED_ONE;

            fa += fdaOuter;
            fs += fdsOuter;
            ft += fdtOuter;
          } else {
            fa += fdaInner;
            fs += fdsInner;
            ft += fdtInner;
          }
        } /*while lines>0*/

      } /* for subTriangle */

    }
  }
}

void TriangleRasterizer::
draw_pixel(const RasterizerVertex *v0, double radius) {
  if (v0->_visibility != 0) {
    // The pixel is off the screen.
    return;
  }
  int ix = (int)v0->_p[0];
  int iy = (int)v0->_p[1];

  if (iy >= 0 && iy < _output->get_y_size() &&
      ix >= 0 && ix < _output->get_x_size()) {
    if (!_read_input) {
      read_input();
    }

    RGBColord rgb;
    double alpha;
    if (_texture == NULL) {
      filter_pixel(rgb, alpha, v0->_uv[0], v0->_uv[1], radius);
    } else {
      filter_pixel(rgb, alpha,
                   v0->_uv[0] * (_texture->get_x_size() - 1),
                   v0->_uv[1] * (_texture->get_y_size() - 1),
                   radius * (_texture->get_x_size() - 1));
    }
    alpha *= v0->_alpha;
    _output->blend(ix, iy, rgb, alpha);
  }
}

void TriangleRasterizer::
filter_pixel(RGBColord &rgb, double &alpha,
             double s, double t, double radius) {
  if (_texture == NULL) {
    rgb.set(_untextured_color[0],
            _untextured_color[1],
            _untextured_color[2]);
    alpha = _untextured_color[3];
    return;
  }

  int ri = (int)radius;
  int si = (int)(s + 0.5);
  int ti = _texture->get_y_size() - 1 - (int)(t + 0.5);

  rgb.set(0.0, 0.0, 0.0);
  alpha = 0.0;

  if (!_filter_output) {
    if (si >= 0 && si < _texture->get_x_size() &&
        ti >= 0 && ti < _texture->get_y_size()) {
      rgb = _texture->get_xel(si, ti);
      alpha = 1.0;
    }
    return;
  }

  int num_total = 0;
  int num_visible = 0;
  for (int yr = -ri; yr <= ri; yr++) {
    int tii = ti + yr;
    for (int xr = -ri; xr <= ri; xr++) {
      int sii = si + xr;
      if (sii >= 0 && sii < _texture->get_x_size() &&
          tii >= 0 && tii < _texture->get_y_size()) {
        rgb += _texture->get_xel(sii, tii);
        num_visible++;
      }
      num_total++;
    }
  }

  if (num_visible != 0) {
    rgb /= (double)num_visible;
    alpha = 1.0;
  }

  // We would do this to antialias the edge of the image.  However, it
  // seems to cause problems at seams, so we won't do it.
  /*
  if (num_total != 0) {
    alpha = (double)num_visible / (double)num_total;
  }
  */
}

void TriangleRasterizer::
read_input() {
  if (_input != NULL) {
    if (!_input->read_file()) {
      nout << "Unable to read image.\n";
    } else {
      _texture = _input->_data;
    }
  }
  _read_input = true;
}
