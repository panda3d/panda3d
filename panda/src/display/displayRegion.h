// Filename: displayRegion.h
// Created by:  mike (09Jan97)
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
#ifndef DISPLAYREGION_H
#define DISPLAYREGION_H

#include "pandabase.h"

#include "referenceCount.h"
#include "nodePath.h"
#include "cullResult.h"
#include "pointerTo.h"

#include "plist.h"

class GraphicsLayer;
class GraphicsChannel;
class GraphicsWindow;
class GraphicsPipe;
class CullHandler;
class Camera;

////////////////////////////////////////////////////////////////////
//       Class : DisplayRegion
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DisplayRegion : public ReferenceCount {
public:
  DisplayRegion(GraphicsLayer *);
  DisplayRegion(GraphicsLayer *,
                const float l, const float r,
                const float b, const float t);
  DisplayRegion(int xsize, int ysize);
private:
  DisplayRegion(const DisplayRegion &);
  void operator = (const DisplayRegion &);

public:
  ~DisplayRegion();

PUBLISHED:
  INLINE void get_dimensions(float &l, float &r, float &b, float &t) const;
  INLINE float get_left() const;
  INLINE float get_right() const;
  INLINE float get_bottom() const;
  INLINE float get_top() const;
  void set_dimensions(float l, float r, float b, float t);

  INLINE GraphicsLayer *get_layer() const;
  GraphicsChannel *get_channel() const;
  GraphicsWindow *get_window() const;
  GraphicsPipe *get_pipe() const;

  void set_camera(const NodePath &camera);
  INLINE const NodePath &get_camera() const;

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void compute_pixels(const int x, const int y);
  INLINE void get_pixels(int &pl, int &pr, int &pb, int &pt) const;
  INLINE void get_region_pixels(int &xo, int &yo, int &w, int &h) const;

  INLINE int get_pixel_width() const;
  INLINE int get_pixel_height() const;

  void output(ostream &out) const;

public:
  void win_display_regions_changed();

protected:

  float _l;
  float _r;
  float _b;
  float _t;

  int _pl;
  int _pr;
  int _pb;
  int _pt;

  GraphicsLayer *_layer;
  NodePath _camera;
  Camera *_camera_node;

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
