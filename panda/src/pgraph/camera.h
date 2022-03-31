/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file camera.h
 * @author drose
 * @date 2002-02-26
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "pandabase.h"

#include "lensNode.h"
#include "perspectiveLens.h"
#include "nodePath.h"
#include "weakNodePath.h"
#include "drawMask.h"
#include "renderState.h"
#include "pointerTo.h"
#include "pmap.h"
#include "auxSceneData.h"

class DisplayRegion;

/**
 * A node that can be positioned around in the scene graph to represent a
 * point of view for rendering a scene.
 */
class EXPCL_PANDA_PGRAPH Camera : public LensNode {
PUBLISHED:
  explicit Camera(const std::string &name, Lens *lens = new PerspectiveLens());
  Camera(const Camera &copy);

public:
  virtual ~Camera();

  virtual PandaNode *make_copy() const;
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;

PUBLISHED:
  INLINE void set_active(bool active);
  INLINE bool is_active() const;
  MAKE_PROPERTY(active, is_active, set_active);

  INLINE void set_scene(const NodePath &scene);
  INLINE const NodePath &get_scene() const;
  MAKE_PROPERTY(scene, get_scene, set_scene);

  INLINE size_t get_num_display_regions() const;
  INLINE DisplayRegion *get_display_region(size_t n) const;
  MAKE_SEQ(get_display_regions, get_num_display_regions, get_display_region);
  MAKE_SEQ_PROPERTY(display_regions, get_num_display_regions, get_display_region);

  INLINE void set_camera_mask(DrawMask mask);
  INLINE DrawMask get_camera_mask() const;
  MAKE_PROPERTY(camera_mask, get_camera_mask, set_camera_mask);

  INLINE void set_cull_center(const NodePath &cull_center);
  INLINE const NodePath &get_cull_center() const;
  MAKE_PROPERTY(cull_center, get_cull_center, set_cull_center);

  INLINE void set_cull_bounds(BoundingVolume *cull_bounds);
  INLINE BoundingVolume *get_cull_bounds() const;
  MAKE_PROPERTY(cull_bounds, get_cull_bounds, set_cull_bounds);

  INLINE void set_lod_center(const NodePath &lod_center);
  INLINE const NodePath &get_lod_center() const;
  MAKE_PROPERTY(lod_center, get_lod_center, set_lod_center);

  INLINE void set_initial_state(const RenderState *state);
  INLINE CPT(RenderState) get_initial_state() const;
  MAKE_PROPERTY(initial_state, get_initial_state, set_initial_state);

  INLINE void set_tag_state_key(const std::string &tag_state_key);
  INLINE const std::string &get_tag_state_key() const;
  MAKE_PROPERTY(tag_state_key, get_tag_state_key, set_tag_state_key);

  INLINE void set_lod_scale(PN_stdfloat value);
  INLINE PN_stdfloat get_lod_scale() const;
  MAKE_PROPERTY(lod_scale, get_lod_scale, set_lod_scale);

  void set_tag_state(const std::string &tag_state, const RenderState *state);
  void clear_tag_state(const std::string &tag_state);
  void clear_tag_states();
  bool has_tag_state(const std::string &tag_state) const;
  CPT(RenderState) get_tag_state(const std::string &tag_state) const;
  MAKE_MAP_PROPERTY(tag_states, has_tag_state, get_tag_state,
                    set_tag_state, clear_tag_state);

  void set_aux_scene_data(const NodePath &node_path, AuxSceneData *data);
  bool clear_aux_scene_data(const NodePath &node_path);
  AuxSceneData *get_aux_scene_data(const NodePath &node_path) const;
  void list_aux_scene_data(std::ostream &out) const;
  int cleanup_aux_scene_data(Thread *current_thread = Thread::get_current_thread());
  MAKE_MAP_PROPERTY(aux_scene_data, get_aux_scene_data, get_aux_scene_data,
                    set_aux_scene_data, clear_aux_scene_data);

private:
  void add_display_region(DisplayRegion *display_region);
  void remove_display_region(DisplayRegion *display_region);

  bool _active;
  NodePath _scene;
  NodePath _cull_center;
  PT(BoundingVolume) _cull_bounds;
  NodePath _lod_center;

  DrawMask _camera_mask;
  PN_stdfloat _lod_scale;

  typedef pvector<DisplayRegion *> DisplayRegions;
  DisplayRegions _display_regions;

  CPT(RenderState) _initial_state;
  std::string _tag_state_key;

  typedef pmap<std::string, CPT(RenderState) > TagStates;
  TagStates _tag_states;

  typedef pmap<NodePath, PT(AuxSceneData) > AuxData;
  AuxData _aux_data;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

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
