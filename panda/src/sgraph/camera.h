// Filename: camera.h
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
#ifndef CAMERA_H
#define CAMERA_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "projectionNode.h"

#include <pt_Node.h>

#include <vector>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
class DisplayRegion;

////////////////////////////////////////////////////////////////////
//       Class : Camera
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Camera : public ProjectionNode {
public:
  Camera(const string &name = "");
  Camera(const Camera &copy);
  void operator = (const Camera &copy);
  virtual ~Camera();

  virtual Node *make_copy() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
 
  //virtual void output(ostream &out) const;
  virtual void config() { }

  INLINE void set_active(bool active);
  INLINE bool is_active() const;

  INLINE void set_scene(Node *scene);
  INLINE Node *get_scene() const;

  int get_num_drs() const;
  DisplayRegion *get_dr(int index) const;

  bool is_in_view(const LPoint3f &pos);

  void get_perspective_params(float &yfov, float &aspect, float &cnear, 
			      float &cfar) const;
  void get_perspective_params(float &xfov, float &yfov, float &aspect,
			      float &cnear, float &cfar) const;
  float get_hfov(void) const;
  float get_vfov(void) const;
  void set_fov(float hfov);
  void set_fov(float hfov, float vfov);
  void set_hfov(float hfov);
  void set_vfov(float vfov);
  float get_aspect(void) const;
  void set_aspect(float aspect);
  void get_near_far(float &cnear, float &cfar) const;
  void set_near_far(float cnear, float cfar);
  float get_near(void) const;
  void set_near(float cnear);
  float get_far(void) const;
  void set_far(float cfar);

private:
  void add_display_region(DisplayRegion *display_region);
  void remove_display_region(DisplayRegion *display_region);

  bool _active;
  PT_Node _scene;

  typedef vector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;

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
