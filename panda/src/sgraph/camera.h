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
#include "pandabase.h"

#include "lensNode.h"
#include "pt_Node.h"
#include "pvector.h"

class DisplayRegion;
class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : Camera
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Camera : public LensNode {
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

  INLINE void set_qpscene(PandaNode *scene);
  INLINE PandaNode *get_qpscene() const;

  int get_num_drs() const;
  DisplayRegion *get_dr(int index) const;

private:
  void add_display_region(DisplayRegion *display_region);
  void remove_display_region(DisplayRegion *display_region);

  bool _active;
  PT_Node _scene;
  PandaNode *_qpscene;

  typedef pvector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;

  static const std::string _CHANGE_CAM;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LensNode::init_type();
    register_type(_type_handle, "Camera",
                  LensNode::get_class_type());
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
