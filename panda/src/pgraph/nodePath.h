// Filename: nodePath.h
// Created by:  drose (25Feb02)
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

#ifndef NODEPATH_H
#define NODEPATH_H

#include "pandabase.h"

#include "pandaNode.h"
#include "renderState.h"
#include "transformState.h"
#include "nodePathComponent.h"

#include "pointerTo.h"
#include "referenceCount.h"
#include "notify.h"
#include "typedObject.h"

class NodePathCollection;
class TextureCollection;
class FindApproxLevel;
class FindApproxPath;
class Texture;
class Material;
class Fog;
class GlobPattern;

//
// A NodePath is the fundamental unit of high-level interaction with
// the scene graph.  It encapsulates the complete path down to a node
// from some other node, usually the root of the scene graph.  This is
// used to resolve ambiguities associated with instancing.
//
// NodePath also contains a number of handy high-level methods for
// common scene-graph manipulations, such as reparenting, and common
// state changes, such as repositioning.
//
// There are also a number of NodePath methods for finding nodes deep
// within the tree by name or by type.  These take a path string,
// which at its simplest consists of a series of node names separated
// by slashes, like a directory pathname.
//
// Each component of the path string may optionally consist of one of
// the following special names, instead of a node name:
//
//   *          -- matches exactly one node, with any name.
//   **         -- matches any sequence of zero or more nodes.
//   +typename  -- matches any node that is or derives from the given type.
//   -typename  -- matches any node that is the given type exactly.
//
// Furthermore, a node name may itself contain standard filename
// globbing characters, like *, ?, and [a-z], that will be accepted as
// a partial match.  (In fact, the '*' special name may be seen as
// just a special case of this.)  The globbing characters may not be
// used with the typename matches.
//
// The special characters "@@", appearing at the beginning of a node
// name, indicate a stashed node.  Normally, stashed nodes are not
// returned by a find (but see the special flags, below), but a
// stashed node may be found if it is explicitly named with its
// leading @@ characters.  By extension, "@@*" may be used to identify
// any stashed node.
//
// Examples:
//
// "room//graph" will look for a node named "graph", which is a child
// of an unnamed node, which is a child of a node named "room", which
// is a child of the starting path.
//
// "**/red*" will look for any node anywhere in the tree (below the
// starting path) with a name that begins with "red".
//
// "**/+PartBundleNode/**/head" will look for a node named "head",
// somewhere below a PartBundleNode anywhere in the tree.
//
//
// The search is always potentially ambiguous, even if the special
// wildcard operators are not used, because there may be multiple
// nodes in the tree with the same name.  In general, in the case of
// an ambiguity, the shortest path is preferred; when a method (such
// as extend_by) must choose only only one of several possible paths,
// it will choose the shortest available; on the other hand, when a
// method (such as find_all_matches) is to return all of the matching
// paths, it will sort them so that the shortest paths appear first in
// the output.
//
//
// Special flags.  The entire string may optionally be followed by the
// ";" character, followed by one or more of the following special
// control flags, with no intervening spaces or punctuation:
//
//    -h    Do not return hidden nodes.
//    +h    Do return hidden nodes.
//    -s    Do not return stashed nodes unless explicitly referenced with @@.
//    +s    Return stashed nodes even without any explicit @@ characters.
//
// The default flags are +h-s.
//


////////////////////////////////////////////////////////////////////
//       Class : NodePath
// Description : NodePath is the fundamental system for disambiguating
//               instances, and also provides a higher-level interface
//               for manipulating the scene graph.
//
//               A NodePath is a list of connected nodes from the root
//               of the graph to any sub-node.  Each NodePath
//               therefore unqiuely describes one instance of a node.
//
//               NodePaths themselves are lightweight objects that may
//               easily be copied and passed by value.  Their data is
//               stored as a series of NodePathComponents that are
//               stored on the nodes.  Holding a NodePath will keep a
//               reference count to all the nodes in the path.
//               However, if any node in the path is removed or
//               reparented (perhaps through a different NodePath),
//               the NodePath will automatically be updated to reflect
//               the changes.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePath {
PUBLISHED:
  // This enumeration is returned by get_error_type() for an empty
  // NodePath to report the reason it's empty.
  enum ErrorType {
    ET_ok = 0,     // i.e. not empty, or never assigned to anything.
    ET_not_found,  // returned from a failed find() or similar function.
    ET_removed,    // remove_node() was previously called on this NodePath.
    ET_fail,       // general failure return from some function.

    // Also see NodePathComponent::_next_key, which initializes
    // itself to the last enumerated type here plus one.
  };

  INLINE NodePath();
  INLINE NodePath(const string &top_node_name);
  INLINE NodePath(PandaNode *node);
  INLINE static NodePath any_path(PandaNode *node);
  INLINE NodePath(const NodePath &parent, PandaNode *child_node);

  INLINE NodePath(const NodePath &copy);
  INLINE void operator = (const NodePath &copy);

  INLINE static NodePath not_found();
  INLINE static NodePath removed();
  INLINE static NodePath fail();

  INLINE static void set_max_search_depth(int max_search_depth);
  INLINE static int get_max_search_depth();

  // Methods to query a NodePath's contents.
  INLINE bool is_empty() const;
  INLINE bool is_singleton() const;
  int get_num_nodes() const;
  PandaNode *get_node(int index) const;

  INLINE ErrorType get_error_type() const;

  PandaNode *get_top_node() const;
  INLINE PandaNode *node() const;

  INLINE int get_key() const;

  // Methods that return collections of NodePaths derived from or
  // related to this one.

  NodePathCollection get_children() const;
  INLINE int get_num_children() const;
  INLINE NodePath get_child(int n) const;

  INLINE bool has_parent() const;
  INLINE NodePath get_parent() const;

  NodePath find(const string &path) const;
  NodePath find_path_to(PandaNode *node) const;
  NodePathCollection find_all_matches(const string &path) const;
  NodePathCollection find_all_paths_to(PandaNode *node) const;

  // Methods that actually move nodes around in the scene graph.  The
  // optional "sort" parameter can be used to force a particular
  // ordering between sibling nodes, useful when dealing with LOD's
  // and similar switch nodes.  If the sort value is the same, nodes
  // will be arranged in the order they were added.
  void reparent_to(const NodePath &other, int sort = 0);
  void wrt_reparent_to(const NodePath &other, int sort = 0);
  NodePath instance_to(const NodePath &other, int sort = 0) const;
  NodePath instance_under_node(const NodePath &other, const string &name,
                                 int sort = 0) const;
  NodePath copy_to(const NodePath &other, int sort = 0) const;
  NodePath attach_new_node(PandaNode *node, int sort = 0) const;
  INLINE NodePath attach_new_node(const string &name, int sort = 0) const;
  void remove_node();
  void detach_node();

  // Handy ways to look at what's there, and other miscellaneous
  // operations.

  void output(ostream &out) const;

  INLINE void ls() const;
  INLINE void ls(ostream &out, int indent_level = 0) const;


  // Aggregate transform and state information.
  INLINE const RenderState *get_state() const;
  INLINE void set_state(const RenderState *state) const;
  CPT(RenderState) get_state(const NodePath &other) const;
  void set_state(const NodePath &other, const RenderState *state) const;
  INLINE CPT(RenderState) get_net_state() const;

  INLINE const TransformState *get_transform() const;
  INLINE void set_transform(const TransformState *transform) const;
  CPT(TransformState) get_transform(const NodePath &other) const;
  void set_transform(const NodePath &other, const TransformState *transform) const;
  INLINE CPT(TransformState) get_net_transform() const;


  // Methods that get and set the matrix transform: pos, hpr, scale,
  // in the local coordinate system.

  INLINE void set_pos(float x, float y, float z);
  void set_pos(const LVecBase3f &pos);
  void set_x(float x);
  void set_y(float y);
  void set_z(float z);
  LPoint3f get_pos() const;
  INLINE float get_x() const;
  INLINE float get_y() const;
  INLINE float get_z() const;

  INLINE void set_hpr(float h, float p, float r);
  void set_hpr(const LVecBase3f &hpr);
  void set_h(float h);
  void set_p(float p);
  void set_r(float r);
  LVecBase3f get_hpr() const;
  LVecBase3f get_hpr(float roll) const;
  INLINE float get_h() const;
  INLINE float get_p() const;
  INLINE float get_r() const;

  void set_quat(const LQuaternionf &quat);
  LQuaternionf get_quat() const;

  INLINE void set_scale(float scale);
  INLINE void set_scale(float sx, float sy, float sz);
  void set_scale(const LVecBase3f &scale);
  void set_sx(float sx);
  void set_sy(float sy);
  void set_sz(float sz);
  LVecBase3f get_scale() const;
  INLINE float get_sx() const;
  INLINE float get_sy() const;
  INLINE float get_sz() const;

  INLINE void set_pos_hpr(float x, float y, float z,
                          float h, float p, float r);
  void set_pos_hpr(const LVecBase3f &pos,
                   const LVecBase3f &hpr);

  INLINE void set_hpr_scale(float h, float p, float r,
                            float sx, float sy, float sz);
  void set_hpr_scale(const LVecBase3f &hpr,
                     const LVecBase3f &scale);
  INLINE void set_pos_hpr_scale(float x, float y, float z,
                                float h, float p, float r,
                                float sx, float sy, float sz);
  void set_pos_hpr_scale(const LVecBase3f &pos,
                         const LVecBase3f &hpr,
                         const LVecBase3f &scale);
  void set_pos_quat_scale(const LVecBase3f &pos,
                          const LQuaternionf &quat,
                          const LVecBase3f &scale);

  void set_mat(const LMatrix4f &mat);
  INLINE void clear_mat();
  INLINE bool has_mat() const;
  INLINE const LMatrix4f &get_mat() const;

  bool has_color_scale() const;
  void clear_color_scale();
  void set_color_scale(const LVecBase4f &scale);
  INLINE void set_color_scale(float sx, float sy, float sz, float sa);
  INLINE void set_sr(float sr);
  INLINE void set_sg(float sg);
  INLINE void set_sb(float sb);
  INLINE void set_sa(float sa);

  const LVecBase4f &get_color_scale() const;
  INLINE float get_sr() const;
  INLINE float get_sg() const;
  INLINE float get_sb() const;
  INLINE float get_sa() const;

  INLINE void look_at(float x, float y, float z);
  void look_at(const LPoint3f &point, const LVector3f &up = LVector3f::up());
  INLINE void heads_up(float x, float y, float z);
  void heads_up(const LPoint3f &point, const LVector3f &up = LVector3f::up());

  // Methods that get and set the matrix transforms relative to some
  // other node in the scene graph.  These perform an implicit wrt().

  INLINE void set_pos(const NodePath &other, float x, float y, float z);
  void set_pos(const NodePath &other, const LVecBase3f &pos);
  void set_x(const NodePath &other, float x);
  void set_y(const NodePath &other, float y);
  void set_z(const NodePath &other, float z);
  LPoint3f get_pos(const NodePath &other) const;
  INLINE float get_x(const NodePath &other) const;
  INLINE float get_y(const NodePath &other) const;
  INLINE float get_z(const NodePath &other) const;

  INLINE void set_hpr(const NodePath &other, float h, float p, float r);
  void set_hpr(const NodePath &other, const LVecBase3f &hpr);
  void set_h(const NodePath &other, float h);
  void set_p(const NodePath &other, float p);
  void set_r(const NodePath &other, float r);
  LVecBase3f get_hpr(const NodePath &other) const;
  LVecBase3f get_hpr(const NodePath &other, float roll) const;
  INLINE float get_h(const NodePath &other) const;
  INLINE float get_p(const NodePath &other) const;
  INLINE float get_r(const NodePath &other) const;

  void set_quat(const NodePath &other, const LQuaternionf &quat);
  LQuaternionf get_quat(const NodePath &other) const;

  INLINE void set_scale(const NodePath &other, float sx, float sy, float sz);
  void set_scale(const NodePath &other, const LVecBase3f &scale);
  void set_sx(const NodePath &other, float sx);
  void set_sy(const NodePath &other, float sy);
  void set_sz(const NodePath &other, float sz);
  LVecBase3f get_scale(const NodePath &other) const;
  INLINE float get_sx(const NodePath &other) const;
  INLINE float get_sy(const NodePath &other) const;
  INLINE float get_sz(const NodePath &other) const;

  INLINE void set_pos_hpr(const NodePath &other,
                          float x, float y, float z,
                          float h, float p, float r);
  void set_pos_hpr(const NodePath &other,
                   const LVecBase3f &pos,
                   const LVecBase3f &hpr);
  INLINE void set_hpr_scale(const NodePath &other,
                float h, float p, float r,
                float sx, float sy, float sz);
  void set_hpr_scale(const NodePath &other,
             const LVecBase3f &hpr,
             const LVecBase3f &scale);
  INLINE void set_pos_hpr_scale(const NodePath &other,
                                float x, float y, float z,
                                float h, float p, float r,
                                float sx, float sy, float sz);
  void set_pos_hpr_scale(const NodePath &other,
                         const LVecBase3f &pos,
                         const LVecBase3f &hpr,
                         const LVecBase3f &scale);
  void set_pos_quat_scale(const NodePath &other,
                          const LVecBase3f &pos,
                          const LQuaternionf &quat,
                          const LVecBase3f &scale);

  const LMatrix4f &get_mat(const NodePath &other) const;
  void set_mat(const NodePath &other, const LMatrix4f &mat);

  LPoint3f get_relative_point(const NodePath &other, const LVecBase3f &point);
  LVector3f get_relative_vector(const NodePath &other, const LVecBase3f &vec);

  INLINE void look_at(const NodePath &other,
                      float x, float y, float z);
  void look_at(const NodePath &other,
               const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
               const LVector3f &up = LVector3f::up());
  INLINE void heads_up(const NodePath &other,
                       float x, float y, float z);
  void heads_up(const NodePath &other,
                const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                const LVector3f &up = LVector3f::up());

  INLINE float get_distance(const NodePath &other) const;


  // Methods that affect appearance of geometry: color, texture, etc.
  // These affect the state at the bottom level only.

  void set_color(float r, float g, float b, float a = 1.0,
                 int priority = 0);
  void set_color(const Colorf &color, int priority = 0);
  void set_color_off(int priority = 0);
  void clear_color();
  bool has_color() const;
  Colorf get_color() const;

  void set_bin(const string &bin_name, int draw_order, int priority = 0);
  void clear_bin();
  bool has_bin() const;
  string get_bin_name() const;
  int get_bin_draw_order() const;

  void set_texture(Texture *tex, int priority = 0);
  void set_texture_off(int priority = 0);
  void clear_texture();
  bool has_texture() const;
  bool has_texture_off() const;
  Texture *get_texture() const;

  Texture *find_texture(const string &name) const;
  TextureCollection find_all_textures() const;
  TextureCollection find_all_textures(const string &name) const;

  void set_material(Material *tex, int priority = 0);
  void set_material_off(int priority = 0);
  void clear_material();
  bool has_material() const;
  PT(Material) get_material() const;

  void set_fog(Fog *fog, int priority = 0);
  void set_fog_off(int priority = 0);
  void clear_fog();
  bool has_fog() const;
  bool has_fog_off() const;
  Fog *get_fog() const;

  void set_render_mode_wireframe(int priority = 0);
  void set_render_mode_filled(int priority = 0);
  void clear_render_mode();
  bool has_render_mode() const;

  void set_two_sided(bool two_sided, int priority = 0);
  void clear_two_sided();
  bool has_two_sided() const;
  bool get_two_sided() const;
#if 0
  void set_alpha_test(RenderAttrib::PandaCompareFunc alpha_test_mode,float reference_alpha,int priority = 0);
  void clear_alpha_test();
  bool has_alpha_test() const;
  bool get_alpha_test() const;
#endif
  void set_depth_test(bool depth_test, int priority = 0);
  void clear_depth_test();
  bool has_depth_test() const;
  bool get_depth_test() const;

  void set_depth_write(bool depth_write, int priority = 0);
  void clear_depth_write();
  bool has_depth_write() const;
  bool get_depth_write() const;

  void do_billboard_axis(const NodePath &camera, float offset);
  void do_billboard_point_eye(const NodePath &camera, float offset);
  void do_billboard_point_world(const NodePath &camera, float offset);
  INLINE void set_billboard_axis(float offset = 0.0);
  INLINE void set_billboard_point_eye(float offset = 0.0);
  INLINE void set_billboard_point_world(float offset = 0.0);
  void set_billboard_axis(const NodePath &camera, float offset);
  void set_billboard_point_eye(const NodePath &camera, float offset);
  void set_billboard_point_world(const NodePath &camera, float offset);
  void clear_billboard();
  bool has_billboard() const;

  void set_compass(const NodePath &reference = NodePath());
  void clear_compass();
  bool has_compass() const;

  void set_transparency(bool transparency, int priority = 0);
  void clear_transparency();
  bool has_transparency() const;
  bool get_transparency() const;

  INLINE void adjust_all_priorities(int adjustment);

  // Variants on show and hide
  INLINE void show();
  INLINE void show(DrawMask camera_mask);
  INLINE void hide();
  INLINE void hide(DrawMask camera_mask);
  INLINE bool is_hidden(DrawMask camera_mask = DrawMask::all_on()) const;
  NodePath get_hidden_ancestor(DrawMask camera_mask = DrawMask::all_on()) const;

  INLINE bool stash();
  INLINE bool unstash();
  INLINE bool is_stashed() const;
  NodePath get_stashed_ancestor() const;

  // Comparison methods
  INLINE bool operator == (const NodePath &other) const;
  INLINE bool operator != (const NodePath &other) const;
  INLINE bool operator < (const NodePath &other) const;
  INLINE int compare_to(const NodePath &other) const;

  // Miscellaneous
  bool verify_complete() const;

  void prepare_scene(GraphicsStateGuardianBase *gsg, bool force_retained_mode = false);

  void show_bounds();
  void hide_bounds();
  PT(BoundingVolume) get_bounds() const;
  void force_recompute_bounds();
  void write_bounds(ostream &out) const;
  bool calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point);

  int flatten_light();
  int flatten_medium();
  int flatten_strong();

  bool write_bam_file(const string &filename) const;

private:
  void uncollapse_head() const;
  static void find_common_ancestor(const NodePath &a, const NodePath &b,
                                   int &a_count, int &b_count);

  CPT(RenderState) r_get_net_state(NodePathComponent *comp) const;
  CPT(RenderState) r_get_partial_state(NodePathComponent *comp, int n) const;
  CPT(TransformState) r_get_net_transform(NodePathComponent *comp) const;
  CPT(TransformState) r_get_partial_transform(NodePathComponent *comp, int n) const;

  void find_matches(NodePathCollection &result,
                    const string &approx_path_str,
                    int max_matches) const;
  void find_matches(NodePathCollection &result,
                    FindApproxPath &approx_path,
                    int max_matches) const;
  void r_find_matches(NodePathCollection &result,
                      const FindApproxLevel &level,
                      int max_matches, int num_levels_remaining) const;

  void r_adjust_all_priorities(PandaNode *node, int adjustment);

  void r_force_recompute_bounds(PandaNode *node);

  typedef pset<Texture *> Textures;
  Texture *r_find_texture(PandaNode *node, const RenderState *state,
                          const GlobPattern &glob) const;
  void r_find_all_textures(PandaNode *node, const RenderState *state,
                           Textures &textures) const;
  void r_prepare_scene(PandaNode *node, const RenderState *state,
                       GraphicsStateGuardianBase *gsg, bool do_retained_mode);

  PT(NodePathComponent) _head;
  ErrorType _error_type;
  static int _max_search_depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "NodePath");
  }

private:
  static TypeHandle _type_handle;

  friend class NodePathCollection;
  friend class WorkingNodePath;
};

INLINE ostream &operator << (ostream &out, const NodePath &node_path);

#include "nodePath.I"

#endif
