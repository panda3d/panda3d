// Filename: qpnodePath.h
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

#ifndef qpNODEPATH_H
#define qpNODEPATH_H

#include "pandabase.h"

#include "pandaNode.h"
#include "renderState.h"
#include "transformState.h"
#include "qpnodePathComponent.h"

#include "pointerTo.h"
#include "referenceCount.h"
#include "notify.h"
#include "typedObject.h"

class qpNodePathCollection;
class qpFindApproxLevel;
class qpFindApproxPath;
class Texture;
class Material;
class Fog;

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
class EXPCL_PANDA qpNodePath {
PUBLISHED:
  // This enumeration is returned by get_error_type() for an empty
  // qpNodePath to report the reason it's empty.
  enum ErrorType {
    ET_ok = 0,     // i.e. not empty, or never assigned to anything.
    ET_not_found,  // returned from a failed find() or similar function.
    ET_removed,    // remove_node() was previously called on this qpNodePath.
    ET_fail,       // general failure return from some function.
  };

  INLINE qpNodePath();
  INLINE qpNodePath(const string &top_node_name);
  INLINE qpNodePath(PandaNode *top_node);
  INLINE qpNodePath(const qpNodePath &copy);
  INLINE qpNodePath(const qpNodePath &parent, PandaNode *child_node);
  INLINE void operator = (const qpNodePath &copy);

  INLINE static qpNodePath not_found();
  INLINE static qpNodePath removed();
  INLINE static qpNodePath fail();

  // Methods to query a qpNodePath's contents.
  INLINE bool is_empty() const;
  INLINE bool is_singleton() const;
  int get_num_nodes() const;
  PandaNode *get_node(int index) const;

  INLINE ErrorType get_error_type() const;

  PandaNode *get_top_node() const;
  INLINE PandaNode *node() const;

  // Methods that return collections of NodePaths derived from or
  // related to this one.

  qpNodePathCollection get_children() const;
  INLINE int get_num_children() const;
  INLINE qpNodePath get_child(int n) const;

  INLINE bool has_parent() const;
  INLINE qpNodePath get_parent() const;

  qpNodePath find(const string &path) const;
  qpNodePathCollection find_all_matches(const string &path) const;
  qpNodePathCollection find_all_paths_to(PandaNode *node) const;

  // Methods that actually move nodes around in the scene graph.  The
  // optional "sort" parameter can be used to force a particular
  // ordering between sibling nodes, useful when dealing with LOD's
  // and similar switch nodes.  If the sort value is the same, nodes
  // will be arranged in the order they were added.
  void reparent_to(const qpNodePath &other, int sort = 0);
  void wrt_reparent_to(const qpNodePath &other, int sort = 0);
  qpNodePath instance_to(const qpNodePath &other, int sort = 0) const;
  qpNodePath copy_to(const qpNodePath &other, int sort = 0) const;
  qpNodePath attach_new_node(PandaNode *node, int sort = 0) const;
  INLINE qpNodePath attach_new_node(const string &name, int sort = 0) const;
  void remove_node();

  // Handy ways to look at what's there, and other miscellaneous
  // operations.

  INLINE void output(ostream &out) const;

  INLINE void ls() const;
  INLINE void ls(ostream &out, int indent_level = 0) const;
  INLINE void ls_transitions() const;
  INLINE void ls_transforms() const;


  // Aggregate transform and state information.
  INLINE CPT(RenderState) get_state() const;
  INLINE void set_state(const RenderState *state) const;
  CPT(RenderState) get_state(const qpNodePath &other) const;
  void set_state(const qpNodePath &other, const RenderState *state) const;
  INLINE CPT(RenderState) get_net_state() const;

  INLINE CPT(TransformState) get_transform() const;
  INLINE void set_transform(const TransformState *transform) const;
  CPT(TransformState) get_transform(const qpNodePath &other) const;
  void set_transform(const qpNodePath &other, const TransformState *transform) const;
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

  void set_mat(const LMatrix4f &mat);
  INLINE void clear_mat();
  INLINE bool has_mat() const;
  INLINE const LMatrix4f &get_mat() const;

  INLINE bool has_color_scale() const;
  INLINE void clear_color_scale();

  void set_color_scale(const LVecBase4f &sv4);
  INLINE void set_color_scale(float sx, float sy, float sz, float sa);
  INLINE void set_sr(float sr);
  INLINE void set_sg(float sg);
  INLINE void set_sb(float sb);
  INLINE void set_sa(float sa);

  LVecBase4f get_color_scale() const;
  INLINE float get_sr() const;
  INLINE float get_sg() const;
  INLINE float get_sb() const;
  INLINE float get_sa() const;

  INLINE void look_at(float x, float y, float z);
  void look_at(const LPoint3f &point, const LVector3f &up = LVector3f::up());
  INLINE void heads_up(float x, float y, float z);
  void heads_up(const LPoint3f &point, const LVector3f &up = LVector3f::up());

  INLINE void look_at_preserve_scale(float x, float y, float z);
  void look_at_preserve_scale(const LPoint3f &point, const LVector3f &up = LVector3f::up());
  INLINE void heads_up_preserve_scale(float x, float y, float z);
  void heads_up_preserve_scale(const LPoint3f &point, const LVector3f &up = LVector3f::up());

  // Methods that get and set the matrix transforms relative to some
  // other node in the scene graph.  These perform an implicit wrt().

  INLINE void set_pos(const qpNodePath &other, float x, float y, float z);
  void set_pos(const qpNodePath &other, const LVecBase3f &pos);
  void set_x(const qpNodePath &other, float x);
  void set_y(const qpNodePath &other, float y);
  void set_z(const qpNodePath &other, float z);
  LPoint3f get_pos(const qpNodePath &other) const;
  INLINE float get_x(const qpNodePath &other) const;
  INLINE float get_y(const qpNodePath &other) const;
  INLINE float get_z(const qpNodePath &other) const;

  INLINE void set_hpr(const qpNodePath &other, float h, float p, float r);
  void set_hpr(const qpNodePath &other, const LVecBase3f &hpr);
  void set_h(const qpNodePath &other, float h);
  void set_p(const qpNodePath &other, float p);
  void set_r(const qpNodePath &other, float r);
  LVecBase3f get_hpr(const qpNodePath &other) const;
  LVecBase3f get_hpr(const qpNodePath &other, float roll) const;
  INLINE float get_h(const qpNodePath &other) const;
  INLINE float get_p(const qpNodePath &other) const;
  INLINE float get_r(const qpNodePath &other) const;

  INLINE void set_scale(const qpNodePath &other, float sx, float sy, float sz);
  void set_scale(const qpNodePath &other, const LVecBase3f &scale);
  void set_sx(const qpNodePath &other, float sx);
  void set_sy(const qpNodePath &other, float sy);
  void set_sz(const qpNodePath &other, float sz);
  LVecBase3f get_scale(const qpNodePath &other) const;
  INLINE float get_sx(const qpNodePath &other) const;
  INLINE float get_sy(const qpNodePath &other) const;
  INLINE float get_sz(const qpNodePath &other) const;

  INLINE void set_pos_hpr(const qpNodePath &other,
                          float x, float y, float z,
                          float h, float p, float r);
  void set_pos_hpr(const qpNodePath &other,
                   const LVecBase3f &pos,
                   const LVecBase3f &hpr);
  INLINE void set_hpr_scale(const qpNodePath &other,
			    float h, float p, float r,
			    float sx, float sy, float sz);
  void set_hpr_scale(const qpNodePath &other,
		     const LVecBase3f &hpr,
		     const LVecBase3f &scale);
  INLINE void set_pos_hpr_scale(const qpNodePath &other,
                                float x, float y, float z,
                                float h, float p, float r,
                                float sx, float sy, float sz);
  void set_pos_hpr_scale(const qpNodePath &other,
                         const LVecBase3f &pos,
                         const LVecBase3f &hpr,
                         const LVecBase3f &scale);

  const LMatrix4f &get_mat(const qpNodePath &other) const;
  void set_mat(const qpNodePath &other, const LMatrix4f &mat);

  LPoint3f get_relative_point(const qpNodePath &other, const LVecBase3f &point);

  INLINE void look_at(const qpNodePath &other,
                      float x, float y, float z);
  void look_at(const qpNodePath &other,
               const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
               const LVector3f &up = LVector3f::up());
  INLINE void heads_up(const qpNodePath &other,
                       float x, float y, float z);
  void heads_up(const qpNodePath &other,
                const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                const LVector3f &up = LVector3f::up());

  INLINE void look_at_preserve_scale(const qpNodePath &other,
                                     float x, float y, float z);
  void look_at_preserve_scale(const qpNodePath &other,
                              const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                              const LVector3f &up = LVector3f::up());
  INLINE void heads_up_preserve_scale(const qpNodePath &other,
                                      float x, float y, float z);
  void heads_up_preserve_scale(const qpNodePath &other,
                               const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                               const LVector3f &up = LVector3f::up());

  INLINE float get_distance(const qpNodePath &other) const;


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

  void do_billboard_axis(const qpNodePath &camera, float offset);
  void do_billboard_point_eye(const qpNodePath &camera, float offset);
  void do_billboard_point_world(const qpNodePath &camera, float offset);
  void set_billboard_axis(float offset = 0.0);
  void set_billboard_point_eye(float offset = 0.0);
  void set_billboard_point_world(float offset = 0.0);
  void clear_billboard();
  bool has_billboard() const;

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
  qpNodePath get_hidden_ancestor(DrawMask camera_mask = DrawMask::all_on()) const;

  INLINE bool stash();
  INLINE bool unstash();
  INLINE bool is_stashed() const;
  qpNodePath get_stashed_ancestor() const;

  // Comparison methods
  INLINE bool operator == (const qpNodePath &other) const;
  INLINE bool operator != (const qpNodePath &other) const;
  INLINE bool operator < (const qpNodePath &other) const;
  INLINE int compare_to(const qpNodePath &other) const;

  bool verify_complete() const;

private:
  void uncollapse_head() const;
  static void find_common_ancestor(const qpNodePath &a, const qpNodePath &b,
                                   int &a_count, int &b_count);

  CPT(RenderState) r_get_net_state(qpNodePathComponent *comp) const;
  CPT(RenderState) r_get_partial_state(qpNodePathComponent *comp, int n) const;
  CPT(TransformState) r_get_net_transform(qpNodePathComponent *comp) const;
  CPT(TransformState) r_get_partial_transform(qpNodePathComponent *comp, int n) const;
  void r_output(ostream &out, qpNodePathComponent *comp) const;

  void find_matches(qpNodePathCollection &result,
                    const string &approx_path_str,
                    int max_matches) const;
  void find_matches(qpNodePathCollection &result,
                    qpFindApproxPath &approx_path,
                    int max_matches) const;
  void r_find_matches(qpNodePathCollection &result,
                      const qpFindApproxLevel &level,
                      int max_matches, int num_levels_remaining) const;

  void r_adjust_all_priorities(PandaNode *node, int adjustment);

  PT(qpNodePathComponent) _head;
  ErrorType _error_type;
  static int _max_search_depth;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "qpNodePath");
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const qpNodePath &node_path);

#include "qpnodePath.I"

#endif
