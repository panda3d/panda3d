// Filename: pgVirtualFrame.h
// Created by:  drose (17Aug05)
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

#ifndef PGVIRTUALFRAME_H
#define PGVIRTUALFRAME_H

#include "pandabase.h"

#include "pgItem.h"
#include "modelNode.h"

class TransformState;

////////////////////////////////////////////////////////////////////
//       Class : PGVirtualFrame
// Description : This represents a frame that is rendered as a window
//               onto another (possibly much larger) canvas.  You can
//               only see the portion of the canvas that is below the
//               window at any given time.
//
//               This works simply by automatically defining clipping
//               planes to be applied to a special child node, called
//               the canvas_node, of the PGVirtualFrame node.  Every
//               object that is parented to the canvas_node will be
//               clipped by the clip_frame.  Also, you can modify the
//               canvas_transform through convenience methods here,
//               which actually modifies the transform on the
//               canvas_node.
//
//               The net effect is that the virtual canvas is
//               arbitrarily large, and we can peek at it through the
//               clip_frame, and scroll through different parts of it
//               by modifying the canvas_transform.
//
//               See PGScrollFrame for a specialization of this class
//               that handles the traditional scrolling canvas, with
//               scroll bars.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGVirtualFrame : public PGItem {
PUBLISHED:
  PGVirtualFrame(const string &name = "");
  virtual ~PGVirtualFrame();

protected:
  PGVirtualFrame(const PGVirtualFrame &copy);
  virtual PandaNode *make_copy() const;
  virtual void r_copy_children(const PandaNode *from, InstanceMap &inst_map);

PUBLISHED:
  void setup(float width, float height);

  INLINE void set_clip_frame(float left, float right, float bottom, float top);
  void set_clip_frame(const LVecBase4f &clip_frame);
  INLINE const LVecBase4f &get_clip_frame() const;
  INLINE bool has_clip_frame() const;
  void clear_clip_frame();

  INLINE void set_canvas_transform(const TransformState *transform);
  INLINE const TransformState *get_canvas_transform() const;

  INLINE PandaNode *get_canvas_node() const;
  INLINE PandaNode *get_clip_plane_node() const;

protected:
  virtual void clip_frame_changed();

private:
  void setup_child_nodes();

private:
  bool _has_clip_frame;
  LVecBase4f _clip_frame;

  PT(ModelNode) _canvas_node;
  PT(PandaNode) _clip_plane_node;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PGItem::init_type();
    register_type(_type_handle, "PGVirtualFrame",
                  PGItem::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgVirtualFrame.I"

#endif
