// Filename: camera.h
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
#ifndef CAMERA_H
#define CAMERA_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "projectionNode.h"

#include <pt_Node.h>

#include "pvector.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : Camera
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Camera : public ProjectionNode {
PUBLISHED:
  Camera(const string &name = "");

public:
  Camera(const Camera &copy);
  void operator = (const Camera &copy);
  virtual ~Camera();

  virtual Node *make_copy() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;

  //virtual void output(ostream &out) const;
  virtual void config() { }

PUBLISHED:
  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void set_scene(Node *scene);
  INLINE Node *get_scene() const;

  int get_num_drs() const;
  DisplayRegion *get_dr(int index) const;

  void set_projection(const Projection &projection);

  void set_fov(float hfov);
  void set_fov(float hfov, float vfov);
  void set_hfov(float hfov);
  void set_vfov(float vfov);
  void set_aspect(float aspect);
  void set_near_far(float cnear, float cfar);
  void set_near(float cnear);
  void set_far(float cfar);

private:
  void add_display_region(DisplayRegion *display_region);
  void remove_display_region(DisplayRegion *display_region);

  bool _active;
  PT_Node _scene;

  typedef pvector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;

  static const std::string _CHANGE_CAM;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ProjectionNode::init_type();
    register_type(_type_handle, "Camera",
                  ProjectionNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

friend class DisplayRegion;
};

#include "camera.I"

#endif
