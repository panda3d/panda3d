// Filename: nodePath.h
// Created by:  drose (05Mar00)
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

#include <pandabase.h>

#include "nodePathBase.h"
#include "nodePathCollection.h"

#include <allTransitionsWrapper.h>
#include <renderRelation.h>
#include <dataRelation.h>
#include <transformTransition.h>
#include <colorMatrixTransition.h>
#include <alphaTransformTransition.h>
#include <colorTransition.h>
#include <textureTransition.h>
#include <materialTransition.h>
#include <material.h>
#include <fogTransition.h>
#include <compose_matrix.h>
#include <renderModeTransition.h>
#include <cullFaceTransition.h>
#include <transparencyTransition.h>
#include <billboardTransition.h>
#include <geomBinTransition.h>
#include <pruneTransition.h>
#include <namedNode.h>

#include "pmap.h"

class FindApproxLevel;
class FindApproxPath;
class Texture;
class Fog;
class Camera;
class AllTransitionsWrapper;
class GraphicsStateGuardianBase;

//
// A NodePath is the fundamental unit of high-level interaction with
// the scene graph.  It encapsulates the complete path down to a node
// from some other node, usually the root of the scene graph.  This
// conveniently resolves ambiguities associated with instancing and
// protects the user from having to keep track of nodes and arcs
// separately.
//
// NodePath also contains a number of handy methods for common
// scene-graph manipulations, such as reparenting, and common state
// changes, such as repositioning.
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
// Description : A NodePath represents a complete path through the
//               scene graph between two particular nodes (e.g. render
//               to a GeomNode).  It thus unambiguously defines
//               instances, especially when it is rooted at the top of
//               the graph.
//
//               This class does not have any data members or
//               additional virtual functions; it's just interface.
//               All of the data is defined in the base class,
//               NodePathBase.  This is important so that
//               NodePathCollection can work correctly without knowing
//               exactly what a NodePath is.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NodePath : public NodePathBase {
PUBLISHED:
  INLINE NodePath(TypeHandle graph_type = RenderRelation::get_class_type());
  INLINE NodePath(Node *top_node, TypeHandle graph_type = RenderRelation::get_class_type());
  INLINE NodePath(const ArcChain &chain, TypeHandle graph_type);

  INLINE NodePath(const NodePathBase &copy);
  INLINE void operator = (const NodePath &copy);
  INLINE ~NodePath();

  INLINE bool operator == (const NodePath &other) const;
  INLINE bool operator != (const NodePath &other) const;
  INLINE int compare_to(const NodePath &other) const;

  INLINE void set_graph_type(TypeHandle graph_type);
  INLINE TypeHandle get_graph_type() const;

  INLINE static void set_max_search_depth(int max_search_depth);
  INLINE static int get_max_search_depth();


  // Methods to extend or shorten a NodePath.

  bool extend_by(Node *dnode);
  bool extend_by(NodeRelation *darc);
  bool extend_by(const NodePath &other);
  bool extend_by(const string &path);
  bool extend_down_to(Node *dnode);

  void shorten(int num_nodes = 1);
  void clear();


  // Methods to query a NodePath's contents.

  INLINE bool is_empty() const;
  INLINE bool is_singleton() const;
  INLINE bool has_arcs() const;
  int get_num_nodes() const;
  Node *get_node(int index) const;

  INLINE int get_num_arcs() const;
  NodeRelation *get_arc(int index) const;

  Node *get_top_node() const;
  INLINE Node *node() const;
  INLINE NodeRelation *arc() const;


  // Methods to manage the disconnected NodePaths that can result if
  // two NodePaths independently reference the same path, and one of
  // them is reparented.

  int share_with(const NodePath &other);
  bool verify_connectivity() const;
  bool amputate_badness();
  bool repair_connectivity(const NodePath &top);


  // Methods that return collections of NodePaths derived from or
  // related to this one.

  NodePathCollection get_siblings() const;
  NodePathCollection get_children() const;
  INLINE int get_num_children() const;
  INLINE NodePath get_child(int n) const;

  INLINE bool has_parent() const;
  INLINE NodePath get_parent() const;

  INLINE NodePath find_path_down_to(Node *dnode) const;
  INLINE NodePath find(const string &path) const;

  NodePathCollection
  find_all_paths_down_to(Node *dnode) const;

  NodePathCollection
  find_all_matches(const string &path) const;

  NodePath find_singular_transform() const;


  // Methods that actually move nodes around in the scene graph.  The
  // optional "sort" parameter can be used to force a particular
  // ordering between sibling nodes, useful when dealing with LOD's
  // and similar switch nodes.  If the sort value is the same, bnodes
  // will be arranged in the order they were added.
  void reparent_to(const NodePath &other, int sort = 0);
  void wrt_reparent_to(const NodePath &other, int sort = 0);
  NodePath instance_to(const NodePath &other, int sort = 0) const;
  NodePath copy_to(const NodePath &other, int sort = 0) const;
  NodePath attach_new_node(Node *dnode, int sort = 0) const;
  INLINE NodePath attach_new_node(const string &name, int sort = 0) const;
  void remove_node();


  // Handy ways to look at what's there, and other miscellaneous
  // operations.

  string as_string(int start_at_node = 0) const;
  INLINE void output(ostream &out) const;
  void write_transitions(ostream &out, int indent_level = 0) const;

  AllTransitionsWrapper get_net_transitions() const;

  INLINE void ls() const;
  INLINE void ls(ostream &out, int indent_level = 0) const;
  INLINE void ls_transitions() const;
  INLINE void ls_transitions(ostream &out, int indent_level = 0) const;
  void analyze() const;

  int flatten_light();
  int flatten_medium(int max_children = 1);
  int flatten_strong(int max_children = 1);

  bool write_bam_file(const string &filename) const;

  // The remaining methods operate on scene graph attributes,
  // according to the arc above the bottom node.  In general, it is
  // valid to call these only if !is_empty() && !is_singleton().


  // Methods that get and set the matrix transform: pos, hpr, scale,
  // in the local coordinate system.  These affect the transform at
  // the bottom level only.

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
  void set_scale(const LVecBase3f &sv3);
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
  INLINE void set_pos_hpr_scale(float x, float y, float z,
                                float h, float p, float r,
                                float sx, float sy, float sz);
  void set_pos_hpr_scale(const LVecBase3f &pos,
                         const LVecBase3f &hpr,
                         const LVecBase3f &scale);

  void set_mat(const LMatrix4f &mat);
  INLINE void clear_mat();
  INLINE bool has_mat() const;
  INLINE LMatrix4f get_mat() const;

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

  INLINE void print_pos() const;
  INLINE void print_hpr() const;
  INLINE void print_scale() const;
  INLINE void print_pos_hpr() const;
  INLINE void print_pos_hpr_scale() const;
  INLINE void print_mat() const;
  INLINE void print_color_scale() const;

  // Methods that get and set the matrix transforms relative to some
  // other node in the scene graph.  These perform an implicit wrt().

  INLINE void set_pos(const NodePath &other, float x, float y, float z);
  void set_pos(const NodePath &other, const LVecBase3f &pos);
  void set_x(const NodePath &other, float x);
  void set_y(const NodePath &other, float y);
  void set_z(const NodePath &other, float z);
  INLINE LPoint3f get_pos(const NodePath &other) const;
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
  INLINE void set_pos_hpr_scale(const NodePath &other,
                                float x, float y, float z,
                                float h, float p, float r,
                                float sx, float sy, float sz);
  void set_pos_hpr_scale(const NodePath &other,
                         const LVecBase3f &pos,
                         const LVecBase3f &hpr,
                         const LVecBase3f &scale);

  void set_mat(const NodePath &other, const LMatrix4f &mat);
  LMatrix4f get_mat(const NodePath &other) const;

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

  INLINE void look_at_preserve_scale(const NodePath &other,
                                     float x, float y, float z);
  void look_at_preserve_scale(const NodePath &other,
                              const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                              const LVector3f &up = LVector3f::up());
  INLINE void heads_up_preserve_scale(const NodePath &other,
                                      float x, float y, float z);
  void heads_up_preserve_scale(const NodePath &other,
                               const LPoint3f &point = LPoint3f(0.0, 0.0, 0.0),
                               const LVector3f &up = LVector3f::up());

  INLINE void print_pos(const NodePath &other) const;
  INLINE void print_hpr(const NodePath &other) const;
  INLINE void print_scale(const NodePath &other) const;
  INLINE void print_pos_hpr(const NodePath &other) const;
  INLINE void print_pos_hpr_scale(const NodePath &other) const;
  INLINE void print_mat(const NodePath &other) const;

  INLINE float get_distance(const NodePath &other) const;


  // Methods that affect appearance of geometry: color, texture, etc.
  // These affect the state at the bottom level only.

  INLINE void set_color(float r, float g, float b, float a = 1.0,
                        int priority = 0);
  void set_color(const Colorf &color, int priority = 0);
  void set_color_off(int priority = 0);
  INLINE void clear_color();
  INLINE bool has_color() const;
  Colorf get_color() const;

  void set_bin(const string &bin_name, int draw_order, int priority = 0);
  INLINE void clear_bin();
  INLINE bool has_bin() const;
  string get_bin_name() const;
  int get_bin_draw_order() const;

  void set_texture(Texture *tex, int priority = 0);
  void set_texture_off(int priority = 0);
  INLINE void clear_texture();
  bool has_texture() const;
  bool has_texture_off() const;
  Texture *get_texture() const;

  void set_material(Material *tex, int priority = 0);
  INLINE void clear_material();
  bool has_material() const;
  PT(Material) get_material() const;

  void set_fog(Fog *fog, int priority = 0);
  void set_fog_off(int priority = 0);
  INLINE void clear_fog();
  bool has_fog() const;
  bool has_fog_off() const;
  Fog *get_fog() const;

  void set_render_mode_wireframe(int priority = 0);
  void set_render_mode_filled(int priority = 0);
  INLINE void clear_render_mode();
  INLINE bool has_render_mode() const;

  void set_two_sided(bool two_sided, int priority = 0);
  INLINE void clear_two_sided();
  INLINE bool has_two_sided() const;
  bool get_two_sided() const;

  void do_billboard_axis(const NodePath &camera, float offset);
  void do_billboard_point_eye(const NodePath &camera, float offset);
  void do_billboard_point_world(const NodePath &camera, float offset);
  INLINE void set_billboard_axis(float offset = 0.0);
  INLINE void set_billboard_point_eye(float offset = 0.0);
  INLINE void set_billboard_point_world(float offset = 0.0);
  INLINE void clear_billboard();
  INLINE bool has_billboard() const;

  void set_transparency(bool transparency, int priority = 0);
  INLINE void clear_transparency();
  INLINE bool has_transparency() const;
  bool get_transparency() const;

  INLINE void adjust_all_priorities(int adjustment);

  INLINE void show();
  INLINE void hide();
  INLINE void show_collision_solids();
  INLINE void hide_collision_solids();
  INLINE bool is_hidden() const;
  NodePath get_hidden_ancestor() const;

  INLINE void stash();
  INLINE void unstash();
  INLINE bool is_stashed() const;
  NodePath get_stashed_ancestor() const;

  void prepare_scene(GraphicsStateGuardianBase *gsg);
  INLINE void clear_wrt_cache();

  void show_bounds();
  void hide_bounds();
  PT(BoundingVolume) get_bounds() const;
  void write_bounds(ostream &out) const;


private:
  bool r_extend_by(const ArcComponent *other);
  int r_as_string(const ArcComponent *comp, string &result,
                  int skip_nodes) const;
  void r_write_transitions(const ArcComponent *comp,
                           ostream &out, int indent_level) const;
  void r_get_net_transitions(const ArcComponent *comp,
                             AllTransitionsWrapper &trans) const;
  string format_node_name(Node *dnode) const;
  string format_arc_name(NodeRelation *arc) const;

  void find_matches(NodePathCollection &result,
                    const string &approx_path_str,
                    int max_matches) const;
  void find_matches(NodePathCollection &result,
                    FindApproxPath &approx_path,
                    int max_matches) const;
  void r_find_matches(NodePathCollection &result,
                      const FindApproxLevel &level,
                      int max_matches, int num_levels_remaining) const;

  void r_list_descendants(ostream &out, int indent_level) const;
  void r_list_transitions(ostream &out, int indent_level) const;

  void r_adjust_all_priorities(NodeRelation *arc, int adjustment);
  void r_clear_wrt_cache(NodeRelation *arc);

  // It's important that there are no data members in this class.  Put
  // them in NodePathBase instead.

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodePathBase::init_type();
    register_type(_type_handle, "NodePath",
                  NodePathBase::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class NodePathCollection;
};

INLINE ostream &operator << (ostream &out, const NodePath &path) {
  path.output(out);
  return out;
}

#include "nodePath.I"

#endif
