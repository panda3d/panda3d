/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file projectionScreen.cxx
 * @author drose
 * @date 2001-12-11
 */

#include "projectionScreen.h"
#include "geomNode.h"
#include "transformState.h"
#include "workingNodePath.h"
#include "switchNode.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"
#include "geomVertexRewriter.h"
#include "config_distort.h"
#include "cullTraverserData.h"

TypeHandle ProjectionScreen::_type_handle;

/**
 *
 */
ProjectionScreen::
ProjectionScreen(const std::string &name) : PandaNode(name)
{
  set_cull_callback();

  _texcoord_name = InternalName::get_texcoord();

  _has_undist_lut = false;
  _invert_uvs = project_invert_uvs;
  _texcoord_3d = false;
  _vignette_on = false;
  _vignette_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _computed_rel_top_mat = false;
  _stale = true;
  _auto_recompute = true;
}

/**
 *
 */
ProjectionScreen::
~ProjectionScreen() {
}

/**
 *
 */
ProjectionScreen::
ProjectionScreen(const ProjectionScreen &copy) :
  PandaNode(copy),
  _projector(copy._projector),
  _projector_node(copy._projector_node),
  _texcoord_name(copy._texcoord_name),
  _vignette_on(copy._vignette_on),
  _vignette_color(copy._vignette_color),
  _frame_color(copy._frame_color),
  _auto_recompute(copy._auto_recompute)
{
  _computed_rel_top_mat = false;
  _stale = true;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *ProjectionScreen::
make_copy() const {
  return new ProjectionScreen(*this);
}

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool ProjectionScreen::
cull_callback(CullTraverser *, CullTraverserData &data) {
  if (_auto_recompute) {
    recompute_if_stale(data.get_node_path());
  }
  return true;
}

/**
 * Specifies the LensNode that is to serve as the projector for this screen.
 * The relative position of the LensNode to the ProjectionScreen, as well as
 * the properties of the lens associated with the LensNode, determines the
 * UV's that will be assigned to the geometry within the ProjectionScreen.
 *
 * The NodePath must refer to a LensNode (or a Camera).
 */
void ProjectionScreen::
set_projector(const NodePath &projector) {
  _projector_node = nullptr;
  _projector = projector;
  if (!projector.is_empty()) {
    nassertv(projector.node()->is_of_type(LensNode::get_class_type()));
    _projector_node = DCAST(LensNode, projector.node());
    _stale = true;
  }
}

/**
 * Synthesizes a polygon mesh based on the projection area of the indicated
 * projector.  This generates and returns a new GeomNode but does not
 * automatically parent it to the ProjectionScreen node; see
 * regenerate_screen().
 *
 * The specified projector need not be the same as the projector given to the
 * ProjectionScreen with set_projector() (although this is often what you
 * want).
 *
 * num_x_verts and num_y_verts specify the number of vertices to make in the
 * grid across the horizontal and vertical dimension of the projector,
 * respectively; distance represents the approximate distance of the screen
 * from the lens center.
 *
 * The fill_ratio parameter specifies the fraction of the image to cover.  If
 * it is 1.0, the entire image is shown full-size; if it is 0.9, 10% of the
 * image around the edges is not part of the grid (and the grid is drawn
 * smaller by the same 10%).  This is intended to work around graphics drivers
 * that tend to show dark edges or other unsatisfactory artifacts around the
 * edges of textures: render the texture larger than necessary by a certain
 * fraction, and make the screen smaller by the inverse fraction.
 */
PT(GeomNode) ProjectionScreen::
generate_screen(const NodePath &projector, const std::string &screen_name,
                int num_x_verts, int num_y_verts, PN_stdfloat distance,
                PN_stdfloat fill_ratio) {
  nassertr(!projector.is_empty() &&
           projector.node()->is_of_type(LensNode::get_class_type()),
           nullptr);
  LensNode *projector_node = DCAST(LensNode, projector.node());
  nassertr(projector_node->get_lens() != nullptr, nullptr);

  // First, get the relative coordinate space of the projector.
  LMatrix4 rel_mat;
  NodePath this_np(this);
  rel_mat = projector.get_transform(this_np)->get_mat();

  // Create a GeomNode to hold this mesh.
  PT(GeomNode) geom_node = new GeomNode(screen_name);

  // Now compute all the vertices for the screen.  These are arranged in order
  // from left to right and bottom to top.
  int num_verts = num_x_verts * num_y_verts;
  Lens *lens = projector_node->get_lens();
  PN_stdfloat t = (distance - lens->get_near()) / (lens->get_far() - lens->get_near());

  PN_stdfloat x_scale = 2.0f / (num_x_verts - 1);
  PN_stdfloat y_scale = 2.0f / (num_y_verts - 1);

  PT(GeomVertexData) vdata = new GeomVertexData
    ("projectionScreen", GeomVertexFormat::get_v3n3(),
     Geom::UH_dynamic);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());

  for (int yi = 0; yi < num_y_verts; yi++) {
    for (int xi = 0; xi < num_x_verts; xi++) {
      LPoint2 film = LPoint2((PN_stdfloat)xi * x_scale - 1.0f,
                             (PN_stdfloat)yi * y_scale - 1.0f);

      // Reduce the image by the fill ratio.
      film *= fill_ratio;

      LPoint3 near_point, far_point;
      lens->extrude(film, near_point, far_point);
      LPoint3 point = near_point + t * (far_point - near_point);

      // Normals aren't often needed on projection screens, but you never
      // know.
      LVector3 norm;
      lens->extrude_vec(film, norm);

      vertex.add_data3(point * rel_mat);
      normal.add_data3(-normalize(norm * rel_mat));
    }
  }
  nassertr(vdata->get_num_rows() == num_verts, nullptr);

  // Now synthesize a triangle mesh.  We run triangle strips horizontally
  // across the grid.
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Fill up the index array into the vertices.  This lays out the order of
  // the vertices in each tristrip.
  int ti, si;
  for (ti = 1; ti < num_y_verts; ti++) {
    strip->add_vertex(ti * num_x_verts);
    for (si = 1; si < num_x_verts; si++) {
      strip->add_vertex((ti - 1) * num_x_verts + (si-1));
      strip->add_vertex(ti * num_x_verts + si);
    }
    strip->add_vertex((ti - 1) * num_x_verts + (num_x_verts-1));
    strip->close_primitive();
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  geom_node->add_geom(geom);

  _stale = true;
  ++_last_screen;
  return geom_node;
}

/**
 * Removes all the children from the ProjectionScreen node, and adds the newly
 * generated child returned by generate_screen().
 */
void ProjectionScreen::
regenerate_screen(const NodePath &projector, const std::string &screen_name,
                  int num_x_verts, int num_y_verts, PN_stdfloat distance,
                  PN_stdfloat fill_ratio) {
  // First, remove all existing children.
  remove_all_children();

  // And attach a new child.
  PT(GeomNode) geom_node =
    generate_screen(projector, screen_name, num_x_verts, num_y_verts,
                    distance, fill_ratio);
  add_child(geom_node);
}

/**
 * Generates a deep copy of the hierarchy at the ProjectionScreen node and
 * below, with vertices flattened into two dimensions as if they were seen by
 * the indicated camera node.
 *
 * This is useful for rendering an image as seen through a non-linear lens.
 * The resulting mesh will have vertices in the range [-1, 1] in both x and y,
 * and may be then rendered with an ordinary orthographic lens, to generate
 * the effect of seeing the image through the specified non-linear lens.
 *
 * The returned node has no parent; it is up to the caller to parent it
 * somewhere or store it so that it does not get dereferenced and deleted.
 */
PT(PandaNode) ProjectionScreen::
make_flat_mesh(const NodePath &this_np, const NodePath &camera) {
  nassertr(!this_np.is_empty() && this_np.node() == this, nullptr);
  nassertr(!camera.is_empty() &&
           camera.node()->is_of_type(LensNode::get_class_type()),
           nullptr);
  LensNode *camera_node = DCAST(LensNode, camera.node());
  nassertr(camera_node->get_lens() != nullptr, nullptr);

  // First, ensure the UV's are up-to-date.
  recompute_if_stale(this_np);

  PT(PandaNode) top = new PandaNode(get_name());

  LMatrix4 rel_mat;
  bool computed_rel_mat = false;
  make_mesh_children(top, this_np, camera, rel_mat, computed_rel_mat);

  return top;
}

/**
 * Recomputes all the UV's for geometry below the ProjectionScreen node, as if
 * the texture were projected from the associated projector.
 *
 * This function is normally called automatically whenever the relevant
 * properties change, so it should not normally need to be called directly by
 * the user.  However, it does no harm to call this if there is any doubt.
 */
void ProjectionScreen::
recompute() {
  NodePath this_np(NodePath::any_path(this));
  do_recompute(this_np);
}

/**
 * Calls recompute() only if the relative transform between the
 * ProjectionScreen and the projector has changed, or if any other relevant
 * property has changed.  Returns true if recomputed, false otherwise.
 */
bool ProjectionScreen::
recompute_if_stale() {
  NodePath this_np(NodePath::any_path(this));
  return recompute_if_stale(this_np);
}

/**
 * Calls recompute() only if the relative transform between the
 * ProjectionScreen and the projector has changed, or if any other relevant
 * property has changed.  Returns true if recomputed, false otherwise.
 */
bool ProjectionScreen::
recompute_if_stale(const NodePath &this_np) {
  nassertr(!this_np.is_empty() && this_np.node() == this, false);

  if (_projector_node != nullptr &&
      _projector_node->get_lens() != nullptr) {
    UpdateSeq lens_change = _projector_node->get_lens()->get_last_change();
    if (_stale || lens_change != _projector_lens_change) {
      recompute();
      return true;

    } else {
      // Get the relative transform to ensure it hasn't changed.
      CPT(TransformState) transform = this_np.get_transform(_projector);
      const LMatrix4 &top_mat = transform->get_mat();
      if (!_rel_top_mat.almost_equal(top_mat)) {
        _rel_top_mat = top_mat;
        _computed_rel_top_mat = true;
        do_recompute(this_np);
        return true;
      }
    }
  }

  return false;
}

/**
 * Starts the recomputation process.
 */
void ProjectionScreen::
do_recompute(const NodePath &this_np) {
  if (_projector_node != nullptr &&
      _projector_node->get_lens() != nullptr) {

    recompute_node(this_np, _rel_top_mat, _computed_rel_top_mat);
    // Make sure this flag is set to false for next time.
    _computed_rel_top_mat = false;

    _projector_lens_change = _projector_node->get_lens()->get_last_change();
    _stale = false;
  }
}

/**
 * Recurses over all geometry at the indicated node and below, looking for
 * GeomNodes that want to have new UV's computed.  When a new transform space
 * is encountered, a new relative matrix is computed.
 */
void ProjectionScreen::
recompute_node(const WorkingNodePath &np, LMatrix4 &rel_mat,
               bool &computed_rel_mat) {
  PandaNode *node = np.node();
  if (node->is_geom_node()) {
    recompute_geom_node(np, rel_mat, computed_rel_mat);
  }

  if (node->is_exact_type(SwitchNode::get_class_type())) {
    // We make a special case for switch nodes only.  Other kinds of selective
    // child nodes, like LOD's and sequence nodes, will get all of their
    // children traversed; switch nodes will only traverse the currently
    // active child.
    int i = DCAST(SwitchNode, node)->get_visible_child();
    if (i >= 0 && i < node->get_num_children()) {
      PandaNode *child = node->get_child(i);
      recompute_child(WorkingNodePath(np, child), rel_mat, computed_rel_mat);
    }

  } else {
    // A non-switch node.  Recurse on all children.
    int num_children = node->get_num_children();
    for (int i = 0; i < num_children; i++) {
      PandaNode *child = node->get_child(i);
      recompute_child(WorkingNodePath(np, child), rel_mat, computed_rel_mat);
    }
  }
}

/**
 * Works in conjunction with recompute_node() to recurse over the whole graph.
 * This is called on each child of a given node.
 */
void ProjectionScreen::
recompute_child(const WorkingNodePath &np, LMatrix4 &rel_mat,
                bool &computed_rel_mat) {
  PandaNode *child = np.node();

  const TransformState *transform = child->get_transform();
  if (!transform->is_identity()) {
    // This child node has a transform; therefore, we must recompute the
    // relative matrix from this point.
    LMatrix4 new_rel_mat;
    bool computed_new_rel_mat = false;

    if (distort_cat.is_spam()) {
      distort_cat.spam()
        << "Saving rel_mat " << (void *)&new_rel_mat << " at " << np << "\n";
    }

    recompute_node(np, new_rel_mat, computed_new_rel_mat);

  } else {
    // This child has no transform, so we can use the same transform space
    // from before.
    recompute_node(np, rel_mat, computed_rel_mat);
  }
}

/**
 * Recomputes the UV's just for the indicated GeomNode.
 */
void ProjectionScreen::
recompute_geom_node(const WorkingNodePath &np, LMatrix4 &rel_mat,
                    bool &computed_rel_mat) {
  GeomNode *node = DCAST(GeomNode, np.node());
  if (!computed_rel_mat) {
    // All right, time to compute the matrix.
    NodePath true_np = np.get_node_path();
    rel_mat = true_np.get_transform(_projector)->get_mat();
    computed_rel_mat = true;

    if (distort_cat.is_spam()) {
      distort_cat.spam()
        << "Computing rel_mat " << (void *)&rel_mat << " at " << np << "\n";
      distort_cat.spam()
        << "  " << rel_mat << "\n";
    }
  } else {
    if (distort_cat.is_spam()) {
      distort_cat.spam()
        << "Applying rel_mat " << (void *)&rel_mat << " to " << np << "\n";
    }
  }

  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    PT(Geom) geom = node->modify_geom(i);
    distort_cat.debug()
      << "  " << *node << " got geom " << geom
      << ", cache_ref = " << geom->get_cache_ref_count() << "\n";
    geom->test_ref_count_integrity();
    recompute_geom(geom, rel_mat);
  }
}

/**
 * Recomputes the UV's just for the indicated Geom.
 */
void ProjectionScreen::
recompute_geom(Geom *geom, const LMatrix4 &rel_mat) {
  static const LMatrix4 lens_to_uv
    (0.5f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.0f, 1.0f);

  static const LMatrix4 lens_to_uv_inverted
    (0.5f, 0.0f, 0.0f, 0.0f,
     0.0f,-0.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.0f, 1.0f);

  Thread *current_thread = Thread::get_current_thread();

  Lens *lens = _projector_node->get_lens();
  nassertv(lens != nullptr);

  const LMatrix4 &to_uv = _invert_uvs ? lens_to_uv_inverted : lens_to_uv;

  // Iterate through all the vertices in the Geom.
  CPT(GeomVertexData) vdata = geom->get_animated_vertex_data(true, current_thread);

  CPT(GeomVertexFormat) vformat = vdata->get_format();
  if (!vformat->has_column(_texcoord_name) || (_texcoord_3d && vformat->get_column(_texcoord_name)->get_num_components() < 3)) {
    // We need to add a new column for the new texcoords.
    vdata = vdata->replace_column
      (_texcoord_name, 3, Geom::NT_stdfloat, Geom::C_texcoord);
    geom->set_vertex_data(vdata);
  }
  if (_vignette_on && !vdata->has_column(InternalName::get_color())) {
    // We need to add a column for color.
    vdata = vdata->replace_column
      (InternalName::get_color(), 1, Geom::NT_packed_dabc, Geom::C_color);
    geom->set_vertex_data(vdata);
  }

  // Clear the vdata pointer so we don't force a copy in the below.
  vdata.clear();

  PT(GeomVertexData) modify_vdata = geom->modify_vertex_data();

  // Maybe the vdata has animation that we should consider.
  CPT(GeomVertexData) animated_vdata = geom->get_animated_vertex_data(true, current_thread);

  GeomVertexWriter texcoord(modify_vdata, _texcoord_name, current_thread);
  GeomVertexWriter color(modify_vdata, current_thread);
  GeomVertexReader vertex(animated_vdata, InternalName::get_vertex(), current_thread);

  if (_vignette_on) {
    color.set_column(InternalName::get_color());
  }

  while (!vertex.is_at_end()) {
    LVertex vert = vertex.get_data3();

    // For each vertex, project to the film plane.
    LPoint3 vert3d = vert * rel_mat;
    LPoint3 film(0.0f, 0.0f, 0.0f);
    bool good = lens->project(vert3d, film);

    // Now the lens gives us coordinates in the range [-1, 1]. Rescale these
    // to [0, 1].
    LPoint3 uvw = film * to_uv;

    if (good && _has_undist_lut) {
      LPoint3f p;
      if (!_undist_lut.calc_bilinear_point(p, uvw[0], 1.0 - uvw[1])) {
        // Point is missing.

        // We're better off keeping the point where it is, undistorted--it's
        // probably close to where it should be--than we are changing it
        // arbitrarily to (0, 0), which might be far away from where it should
        // be.  uvw.set(0, 0, 0);
        good = false;

      } else {
        uvw = LCAST(PN_stdfloat, p);
        uvw[1] = 1.0 - uvw[1];
      }
    }
    texcoord.set_data3(uvw);

    // If we have vignette color in effect, color the vertex according to
    // whether it fell in front of the lens or not.
    if (_vignette_on) {
      if (good) {
        color.set_data4(_frame_color);
      } else {
        color.set_data4(_vignette_color);
      }
    }
  }
}

/**
 * Recurses over all geometry at the indicated node and below, and generates a
 * corresponding node hierarchy with all the geometry copied, but flattened
 * into 2-d, as seen from the indicated camera.  Returns the newly created
 * node, or NULL if no node was created.
 */
PandaNode *ProjectionScreen::
make_mesh_node(PandaNode *result_parent, const WorkingNodePath &np,
               const NodePath &camera,
               LMatrix4 &rel_mat, bool &computed_rel_mat) {
  PandaNode *node = np.node();

  PT(PandaNode) new_node;
  if (node->is_geom_node()) {
    new_node = make_mesh_geom_node(np, camera, rel_mat, computed_rel_mat);
  } else if (node->safe_to_flatten()) {
    new_node = node->make_copy();
    new_node->clear_transform();
  } else {
    // If we can't safely flatten the node, just make a plain node in its
    // place.
    new_node = new PandaNode(node->get_name());
    new_node->set_state(node->get_state());
  }

  // Now attach the new node to the result.
  result_parent->add_child(new_node);
  make_mesh_children(new_node, np, camera, rel_mat, computed_rel_mat);
  return new_node;
}

/**
 * Walks over the list of children for the indicated node, calling
 * make_mesh_node() on each one.
 */
void ProjectionScreen::
make_mesh_children(PandaNode *new_node, const WorkingNodePath &np,
                   const NodePath &camera,
                   LMatrix4 &rel_mat, bool &computed_rel_mat) {
  PandaNode *node = np.node();
  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    PandaNode *new_child;

    const TransformState *transform = child->get_transform();
    if (!transform->is_identity()) {
      // This child node has a transform; therefore, we must recompute the
      // relative matrix from this point.
      LMatrix4 new_rel_mat;
      bool computed_new_rel_mat = false;
      new_child = make_mesh_node(new_node, WorkingNodePath(np, child), camera,
                                 new_rel_mat, computed_new_rel_mat);

    } else {
      // This child has no transform, so we can use the same transform space
      // from before.
      new_child = make_mesh_node(new_node, WorkingNodePath(np, child), camera,
                                 rel_mat, computed_rel_mat);
    }

    if (new_child != nullptr) {
      // Copy all of the render state (except TransformState) to the new arc.
      new_child->set_state(child->get_state());
    }
  }
}

/**
 * Makes a new GeomNode, just like the given one, except flattened into two
 * dimensions as seen by the indicated camera.
 */
PT(GeomNode) ProjectionScreen::
make_mesh_geom_node(const WorkingNodePath &np, const NodePath &camera,
                    LMatrix4 &rel_mat, bool &computed_rel_mat) {
  GeomNode *node = DCAST(GeomNode, np.node());
  PT(GeomNode) new_node = new GeomNode(node->get_name());
  LensNode *lens_node = DCAST(LensNode, camera.node());

  if (!computed_rel_mat) {
    // All right, time to compute the matrix.
    NodePath true_np = np.get_node_path();
    rel_mat = true_np.get_transform(camera)->get_mat();
    computed_rel_mat = true;
  }

  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    const Geom *geom = node->get_geom(i);
    PT(Geom) new_geom =
      make_mesh_geom(geom, lens_node->get_lens(), rel_mat);
    if (new_geom != nullptr) {
      new_node->add_geom(new_geom, node->get_geom_state(i));
    }
  }

  return new_node;
}

/**
 * Makes a new Geom, just like the given one, except flattened into two
 * dimensions as seen by the indicated lens.  Any triangle in the original
 * mesh that involves an unprojectable vertex is eliminated.
 */
PT(Geom) ProjectionScreen::
make_mesh_geom(const Geom *geom, Lens *lens, LMatrix4 &rel_mat) {
  static const LMatrix4 lens_to_uv
    (0.5f, 0.0f, 0.0f, 0.0f,
     0.0f, 0.5f, 0.0f, 0.0f,
     0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, 0.5f, 0.0f, 1.0f);
  static const LMatrix4 uv_to_lens = invert(lens_to_uv);

  Thread *current_thread = Thread::get_current_thread();
  PT(Geom) new_geom = geom->make_copy();
  new_geom->set_vertex_data(new_geom->get_animated_vertex_data(false, current_thread));
  PT(GeomVertexData) vdata = new_geom->modify_vertex_data();
  GeomVertexRewriter vertex(vdata, InternalName::get_vertex());
  while (!vertex.is_at_end()) {
    LVertex vert = vertex.get_data3();

    // Project each vertex into the film plane, but use three dimensions so
    // the Z coordinate remains meaningful.
    LPoint3 vert3d = vert * rel_mat;
    LPoint3 film(0.0f, 0.0f, 0.0f);
    bool good = lens->project(vert3d, film);

    if (good && _has_undist_lut) {

      // Now the lens gives us coordinates in the range [-1, 1]. Rescale these
      // to [0, 1].
      LPoint3 uvw = film * lens_to_uv;

      LPoint3f p;
      if (!_undist_lut.calc_bilinear_point(p, uvw[0], 1.0 - uvw[1])) {
        // Point is missing.
        uvw.set(0, 0, 0);
        good = false;
      } else {
        uvw = LCAST(PN_stdfloat, p);
        uvw[1] = 1.0 - uvw[1];
      }

      film = uvw * uv_to_lens;
    }

    vertex.set_data3(film);
  }

  return new_geom;
}
