// Filename: graphicsLayer.h
// Created by:  drose (18Apr00)
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

#ifndef GRAPHICSLAYER_H
#define GRAPHICSLAYER_H

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "displayRegion.h"

#include <typedReferenceCount.h>
#include <pointerTo.h>

#include "pvector.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class GraphicsChannel;
class GraphicsWindow;
class GraphicsPipe;
class CullHandler;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsLayer
// Description : A layer is a collection of non-overlapping
//               DisplayRegions within a Channel that will be rendered
//               together.  When the channel renders, it will render
//               all of its layers in index number order; each layer
//               may overlap some or all of its DisplayRegions with
//               other layers, and they will be drawn sequentially
//               without clearing the framebuffer between layers.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GraphicsLayer : public TypedReferenceCount {
protected:

  GraphicsLayer();
  GraphicsLayer(GraphicsChannel *);

public:
  virtual ~GraphicsLayer();

PUBLISHED:
  DisplayRegion *make_display_region();
  DisplayRegion *make_display_region(float l, float r,
                                     float b, float t);

  int get_num_drs() const;
  DisplayRegion *get_dr(int index) const;
  void remove_dr(int index);
  bool remove_dr(DisplayRegion *display_region);

  INLINE GraphicsChannel *get_channel() const;
  GraphicsWindow *get_window() const;
  GraphicsPipe *get_pipe() const;

public:
  void channel_resized(int x, int y);

  void win_display_regions_changed();

PUBLISHED:
  INLINE void set_active(bool active);
  INLINE bool is_active() const;

private:
  GraphicsChannel *_channel;
  bool _is_active;

  typedef pvector< PT(DisplayRegion) > DisplayRegions;
  DisplayRegions _display_regions;

private:

  GraphicsLayer(const GraphicsLayer&);
  GraphicsLayer& operator=(const GraphicsLayer&);


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "GraphicsLayer",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsChannel;
  friend class GraphicsWindow;
};

#include "graphicsLayer.I"

#endif /* GRAPHICSLAYER_H */
