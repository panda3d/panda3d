// Filename: displayRegion.h
// Created by:  mike (09Jan97)
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
#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H

#include "pandabase.h"

#include "drawableRegion.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "cullResult.h"
#include "pointerTo.h"
#include "pmutex.h"
#include "config_display.h"

#include "plist.h"

class GraphicsLayer;
class GraphicsChannel;
class GraphicsOutput;
class GraphicsPipe;
class CullHandler;
class Camera;
class PNMImage;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegion
// Description : A rectangular subregion within a window for rendering
//               into.  Typically, there is one DisplayRegion that
//               covers the whole window, but you may also create
//               smaller DisplayRegions for having different regions
//               within the window that represent different scenes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplayRegion : public ReferenceCount, public DrawableRegion {
public:
  DisplayRegion(GraphicsLayer *layer);
  DisplayRegion(GraphicsLayer *layer,
                const float l, const float r,
                const float b, const float t);
  DisplayRegion(GraphicsOutput *window, int xsize, int ysize);
private:
  DisplayRegion(const DisplayRegion &copy);
  void operator = (const DisplayRegion &copy);

public:
  ~DisplayRegion();

PUBLISHED:
  void get_dimensions(float &l, float &r, float &b, float &t) const;
  float get_left() const;
  float get_right() const;
  float get_bottom() const;
  float get_top() const;
  void set_dimensions(float l, float r, float b, float t);

  GraphicsLayer *get_layer() const;
  GraphicsChannel *get_channel() const;
  GraphicsOutput *get_window() const;
  GraphicsPipe *get_pipe() const;

  void set_camera(const NodePath &camera);
  NodePath get_camera() const;

  void set_active(bool active);
  INLINE bool is_active() const;

  void compute_pixels();
  void compute_pixels(int x_size, int y_size);
  void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  void get_region_pixels(int &xo, int &yo, int &w, int &h) const;
  void get_region_pixels_i(int &xo, int &yo, int &w, int &h) const;

  int get_pixel_width() const;
  int get_pixel_height() const;

  void output(ostream &out) const;

  Filename save_screenshot_default(const string &prefix = "screenshot");
  bool save_screenshot(const Filename &filename);
  bool get_screenshot(PNMImage &image);

private:
  void win_display_regions_changed();
  INLINE void do_compute_pixels(int x_size, int y_size);
  Mutex _lock;

  float _l;
  float _r;
  float _b;
  float _t;

  int _pl;
  int _pr;
  int _pb;
  int _pt;
  int _pbi;
  int _pti;

  GraphicsLayer *_layer;
  GraphicsOutput *_window;
  NodePath _camera;

  // This needs to be a PT(Camera) so we prevent the Camera node from
  // destructing while we hold its pointer.
  PT(Camera) _camera_node;

  bool _active;

  // This is used to cache the culling result from last frame's
  // drawing into this display region.  It should only be accessed or
  // modified by the GraphicsEngine, during the cull traversal.
  PT(CullResult) _cull_result;

  friend class GraphicsLayer;
  friend class GraphicsEngine;
};

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr);

#include "displayRegion.I"

#endif /* DISPLAYREGION_H */
