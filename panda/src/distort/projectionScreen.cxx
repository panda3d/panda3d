// Filename: projectionScreen.cxx
// Created by:  drose (11Dec01)
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

TypeHandle ProjectionScreen::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ProjectionScreen::
ProjectionScreen(const string &name) : PandaNode(name)
{
  _texcoord_name = InternalName::get_texcoord();

  _invert_uvs = project_invert_uvs;
  _vignette_on = false;
  _vignette_color.set(0.0f, 0.0f, 0.0f, 1.0f);
  _frame_color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _computed_rel_top_mat = false;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
ProjectionScreen::
~ProjectionScreen() {
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
ProjectionScreen::
ProjectionScreen(const ProjectionScreen &copy) :
  PandaNode(copy),
  _projector(copy._projector),
  _projector_node(copy._projector_node),
  _texcoord_name(copy._texcoord_name),
  _vignette_on(copy._vignette_on),
  _vignette_color(copy._vignette_color),
  _frame_color(copy._frame_color)
{
  _computed_rel_top_mat = false;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *ProjectionScreen::
make_copy() const {
  return new ProjectionScreen(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool ProjectionScreen::
has_cull_callback() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool ProjectionScreen::
cull_callback(CullTraverser *, CullTraverserData &data) {
  recompute_if_stale(data._node_path.get_node_path());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::set_projector
//       Access: Published
//  Description: Specifies the LensNode that is to serve as the
//               projector for this screen.  The relative position of
//               the LensNode to the ProjectionScreen, as well as the
//               properties of the lens associated with the LensNode,
//               determines the UV's that will be assigned to the
//               geometry within the ProjectionScreen.
//
//               The NodePath must refer to a LensNode (or a Camera).
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
set_projector(const NodePath &projector) {
  _projector_node = (LensNode *)NULL;
  _projector = projector;
  if (!projector.is_empty()) {
    nassertv(projector.node()->is_of_type(LensNode::get_class_type()));
    _projector_node = DCAST(LensNode, projector.node());
    _stale = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::generate_screen
//       Access: Published
//  Description: Synthesizes a polygon mesh based on the projection
//               area of the indicated projector.  This generates and
//               returns a new GeomNode but does not automatically
//               parent it to the ProjectionScreen node; see
//               regenerate_screen().
//
//               The specified projector need not be the same as the
//               projector given to the ProjectionScreen with
//               set_projector() (although this is often what you
//               want).
//
//               num_x_verts and num_y_verts specify the number of
//               vertices to make in the grid across the horizontal
//               and vertical dimension of the projector,
//               respectively; distance represents the approximate
//               distance of the screen from the lens center.
//
//               The fill_ratio parameter specifies the fraction of
//               the image to cover.  If it is 1.0, the entire image
//               is shown full-size; if it is 0.9, 10% of the image
//               around the edges is not part of the grid (and the
//               grid is drawn smaller by the same 10%).  This is
//               intended to work around graphics drivers that tend to
//               show dark edges or other unsatisfactory artifacts
//               around the edges of textures: render the texture
//               larger than necessary by a certain fraction, and make
//               the screen smaller by the inverse fraction.
////////////////////////////////////////////////////////////////////
PT(GeomNode) ProjectionScreen::
generate_screen(const NodePath &projector, const string &screen_name,
                int num_x_verts, int num_y_verts, float distance,
                float fill_ratio) {
  nassertr(!projector.is_empty() && 
           projector.node()->is_of_type(LensNode::get_class_type()),
           NULL);
  LensNode *projector_node = DCAST(LensNode, projector.node());
  nassertr(projector_node->get_lens() != NULL, NULL);

  // First, get the relative coordinate space of the projector.
  LMatrix4f rel_mat;
  NodePath this_np(this);
  rel_mat = projector.get_transform(this_np)->get_mat();

  // Create a GeomNode to hold this mesh.
  PT(GeomNode) geom_node = new GeomNode(screen_name);

  // Now compute all the vertices for the screen.  These are arranged
  // in order from left to right and bottom to top.
  int num_verts = num_x_verts * num_y_verts;
  Lens *lens = projector_node->get_lens();
  float t = (distance - lens->get_near()) / (lens->get_far() - lens->get_near());

  float x_scale = 2.0f / (num_x_verts - 1);
  float y_scale = 2.0f / (num_y_verts - 1);

  PT(GeomVertexData) vdata = new GeomVertexData
    ("projectionScreen", GeomVertexFormat::get_v3n3(),
     Geom::UH_dynamic);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  
  for (int yi = 0; yi < num_y_verts; yi++) {
    for (int xi = 0; xi < num_x_verts; xi++) {
      LPoint2f film = LPoint2f((float)xi * x_scale - 1.0f,
                               (float)yi * y_scale - 1.0f);
      
      // Reduce the image by the fill ratio.
      film *= fill_ratio;
      
      LPoint3f near_point, far_point;
      lens->extrude(film, near_point, far_point);
      LPoint3f point = near_point + t * (far_point - near_point);
      
      // Normals aren't often needed on projection screens, but you
      // never know.
      LVector3f norm;
      lens->extrude_vec(film, norm);
      
      vertex.add_data3f(point * rel_mat);
      normal.add_data3f(-normalize(norm * rel_mat));
    }
  }
  nassertr(vdata->get_num_rows() == num_verts, NULL);

  // Now synthesize a triangle mesh.  We run triangle strips
  // horizontally across the grid.
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Fill up the index array into the vertices.  This lays out the
  // order of the vertices in each tristrip.
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

  PT(Geom) geom = new Geom;
  geom->set_vertex_data(vdata);
  geom->add_primitive(strip);
  
  geom_node->add_geom(geom);

  _stale = true;
  ++_last_screen;
  return geom_node;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::regenerate_screen
//       Access: Published
//  Description: Removes all the children from the ProjectionScreen
//               node, and adds the newly generated child returned by
//               generate_screen().
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
regenerate_screen(const NodePath &projector, const string &screen_name,
                  int num_x_verts, int num_y_verts, float distance,
                  float fill_ratio) {
  // First, remove all existing children.
  remove_all_children();

  // And attach a new child.
  PT(GeomNode) geom_node = 
    generate_screen(projector, screen_name, num_x_verts, num_y_verts, 
                    distance, fill_ratio);
  add_child(geom_node);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_flat_mesh
//       Access: Published
//  Description: Generates a deep copy of the hierarchy at the
//               ProjectionScreen node and below, with vertices
//               flattened into two dimensions as if they were seen by
//               the indicated camera node.
//
//               This is useful for rendering an image as seen through
//               a non-linear lens.  The resulting mesh will have
//               vertices in the range [-1, 1] in both x and y, and
//               may be then rendered with an ordinary orthographic
//               lens, to generate the effect of seeing the image
//               through the specified non-linear lens.
//
//               The returned node has no parent; it is up to the
//               caller to parent it somewhere or store it so that it
//               does not get dereferenced and deleted.
////////////////////////////////////////////////////////////////////
PT(PandaNode) ProjectionScreen::
make_flat_mesh(const NodePath &this_np, const NodePath &camera) {
  nassertr(!this_np.is_empty() && this_np.node() == this, NULL);
  nassertr(!camera.is_empty() && 
           camera.node()->is_of_type(LensNode::get_class_type()),
           NULL);
  LensNode *camera_node = DCAST(LensNode, camera.node());
  nassertr(camera_node->get_lens() != (Lens *)NULL, NULL);

  // First, ensure the UV's are up-to-date.
  recompute_if_stale(this_np);

  PT(PandaNode) top = new PandaNode(get_name());

  LMatrix4f rel_mat;
  bool computed_rel_mat = false;
  make_mesh_children(top, this_np, camera, rel_mat, computed_rel_mat);

  return top;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute
//       Access: Published
//  Description: Recomputes all the UV's for geometry below the
//               ProjectionScreen node, as if the texture were
//               projected from the associated projector.
//
//               This function is normally called automatically
//               whenever the relevant properties change, so it should
//               not normally need to be called directly by the user.
//               However, it does no harm to call this if there is any
//               doubt.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute() {
  NodePath this_np(NodePath::any_path(this));
  do_recompute(this_np);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_if_stale
//       Access: Public
//  Description: Calls recompute() only if the relative transform
//               between the ProjectionScreen and the projector has
//               changed, or if any other relevant property has
//               changed.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_if_stale(const NodePath &this_np) {
  nassertv(!this_np.is_empty() && this_np.node() == this);

  if (_projector_node != (LensNode *)NULL && 
      _projector_node->get_lens() != (Lens *)NULL) {
    UpdateSeq lens_change = _projector_node->get_lens()->get_last_change();
    if (_stale || lens_change != _projector_lens_change) {
      recompute();

    } else {
      // Get the relative transform to ensure it hasn't changed.
      CPT(TransformState) transform = this_np.get_transform(_projector);
      const LMatrix4f &top_mat = transform->get_mat();
      if (!_rel_top_mat.almost_equal(top_mat)) {
        _rel_top_mat = top_mat;
        _computed_rel_top_mat = true;
        do_recompute(this_np);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::do_recompute
//       Access: Private
//  Description: Starts the recomputation process.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
do_recompute(const NodePath &this_np) {
  if (_projector_node != (LensNode *)NULL && 
      _projector_node->get_lens() != (Lens *)NULL) {

    recompute_node(this_np, _rel_top_mat, _computed_rel_top_mat);
    // Make sure this flag is set to false for next time.
    _computed_rel_top_mat = false;

    _projector_lens_change = _projector_node->get_lens()->get_last_change();
    _stale = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_node
//       Access: Private
//  Description: Recurses over all geometry at the indicated node and
//               below, looking for GeomNodes that want to have new
//               UV's computed.  When a new transform space is
//               encountered, a new relative matrix is computed.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_node(const WorkingNodePath &np, LMatrix4f &rel_mat,
               bool &computed_rel_mat) {
  PandaNode *node = np.node();
  if (node->is_geom_node()) {
    recompute_geom_node(np, rel_mat, computed_rel_mat);
  }

  if (node->is_exact_type(SwitchNode::get_class_type())) {
    // We make a special case for switch nodes only.  Other kinds of
    // selective child nodes, like LOD's and sequence nodes, will get
    // all of their children traversed; switch nodes will only
    // traverse the currently active child.
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

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_child
//       Access: Private
//  Description: Works in conjunction with recompute_node() to recurse
//               over the whole graph.  This is called on each child
//               of a given node.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_child(const WorkingNodePath &np, LMatrix4f &rel_mat,
                bool &computed_rel_mat) {
  PandaNode *child = np.node();

  const TransformState *transform = child->get_transform();
  if (!transform->is_identity()) {
    // This child node has a transform; therefore, we must recompute
    // the relative matrix from this point.
    LMatrix4f new_rel_mat;
    bool computed_new_rel_mat = false;

    if (distort_cat.is_spam()) {
      distort_cat.spam()
        << "Saving rel_mat " << (void *)&new_rel_mat << " at " << np << "\n";
    }

    recompute_node(np, new_rel_mat, computed_new_rel_mat);
    
  } else {
    // This child has no transform, so we can use the same transform
    // space from before.
    recompute_node(np, rel_mat, computed_rel_mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_geom_node
//       Access: Private
//  Description: Recomputes the UV's just for the indicated GeomNode.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_geom_node(const WorkingNodePath &np, LMatrix4f &rel_mat, 
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
    Geom *geom = node->get_unique_geom(i);
    recompute_geom(geom, rel_mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_geom
//       Access: Private
//  Description: Recomputes the UV's just for the indicated Geom.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_geom(Geom *geom, const LMatrix4f &rel_mat) {
  static const LMatrix3f lens_to_uv
    (0.5f, 0.0f, 0.0f,
     0.0f, 0.5f, 0.0f,
     0.5f, 0.5f, 1.0f);

  static const LMatrix3f lens_to_uv_inverted
    (0.5f, 0.0f, 0.0f,
     0.0f,-0.5f, 0.0f,
     0.5f, 0.5f, 1.0f);

  Lens *lens = _projector_node->get_lens();
  nassertv(lens != (Lens *)NULL);

  const LMatrix3f &to_uv = _invert_uvs ? lens_to_uv_inverted : lens_to_uv;

  // Iterate through all the vertices in the Geom.

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  if (!vdata->has_column(_texcoord_name)) {
    // We need to add a new column for the new texcoords.
    vdata = vdata->replace_column
      (_texcoord_name, 2, Geom::NT_float32, Geom::C_texcoord);
    geom->set_vertex_data(vdata);
  }
  if (_vignette_on && !vdata->has_column(InternalName::get_color())) {
    // We need to add a column for color.
    vdata = vdata->replace_column
      (InternalName::get_color(), 1, Geom::NT_packed_dabc, Geom::C_color);
    geom->set_vertex_data(vdata);
  }
  
  GeomVertexWriter texcoord(geom->modify_vertex_data(), _texcoord_name);
  GeomVertexWriter color(geom->modify_vertex_data());
  GeomVertexReader vertex(geom->get_vertex_data(), InternalName::get_vertex());
  
  if (_vignette_on) {
    color.set_column(InternalName::get_color());
  }
  
  while (!vertex.is_at_end()) {
    Vertexf vert = vertex.get_data3f();
    
    // For each vertex, project to the film plane.
    LPoint2f film(0.0, 0.0);
    bool good = lens->project(vert * rel_mat, film);
    
    // Now the lens gives us coordinates in the range [-1, 1].
    // Rescale these to [0, 1].
    texcoord.set_data2f(film * to_uv);
    
    // If we have vignette color in effect, color the vertex according
    // to whether it fell in front of the lens or not.
    if (_vignette_on) {
      if (good) {
        color.set_data4f(_frame_color);
      } else {
        color.set_data4f(_vignette_color);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_node
//       Access: Private
//  Description: Recurses over all geometry at the indicated node and
//               below, and generates a corresponding node hierarchy
//               with all the geometry copied, but flattened into 2-d,
//               as seen from the indicated camera.  Returns the newly
//               created node, or NULL if no node was created.
////////////////////////////////////////////////////////////////////
PandaNode *ProjectionScreen::
make_mesh_node(PandaNode *result_parent, const WorkingNodePath &np,
               const NodePath &camera,
               LMatrix4f &rel_mat, bool &computed_rel_mat) {
  PandaNode *node = np.node();
  if (!node->safe_to_flatten()) {
    // If we can't safely flatten this node, ignore it (and all of its
    // children) completely.  It's got no business being here anyway.
    return NULL;
  }

  PT(PandaNode) new_node;
  if (node->is_geom_node()) {
    new_node = make_mesh_geom_node(np, camera, rel_mat, computed_rel_mat);
  } else {
    new_node = node->make_copy();
  }

  // Now attach the new node to the result.
  result_parent->add_child(new_node);
  make_mesh_children(new_node, np, camera, rel_mat, computed_rel_mat);
  return new_node;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_children
//       Access: Private
//  Description: Walks over the list of children for the indicated
//               node, calling make_mesh_node() on each one.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
make_mesh_children(PandaNode *new_node, const WorkingNodePath &np,
                   const NodePath &camera,
                   LMatrix4f &rel_mat, bool &computed_rel_mat) {
  PandaNode *node = np.node();
  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);
    PandaNode *new_child;

    const TransformState *transform = child->get_transform();
    if (!transform->is_identity()) {
      // This child node has a transform; therefore, we must recompute
      // the relative matrix from this point.
      LMatrix4f new_rel_mat;
      bool computed_new_rel_mat = false;
      new_child = make_mesh_node(new_node, WorkingNodePath(np, child), camera,
                                 new_rel_mat, computed_new_rel_mat);

    } else {
      // This child has no transform, so we can use the same transform
      // space from before.
      new_child = make_mesh_node(new_node, WorkingNodePath(np, child), camera,
                                 rel_mat, computed_rel_mat);
    }

    // Copy all of the render state (except TransformState) to the
    // new arc.
    new_child->set_state(child->get_state());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_geom_node
//       Access: Private
//  Description: Makes a new GeomNode, just like the given one, except
//               flattened into two dimensions as seen by the
//               indicated camera.
////////////////////////////////////////////////////////////////////
PT(GeomNode) ProjectionScreen::
make_mesh_geom_node(const WorkingNodePath &np, const NodePath &camera,
                    LMatrix4f &rel_mat, bool &computed_rel_mat) {
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
    if (new_geom != (Geom *)NULL) {
      new_node->add_geom(new_geom, node->get_geom_state(i));
    }
  }

  return new_node;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_geom
//       Access: Private
//  Description: Makes a new Geom, just like the given one, except
//               flattened into two dimensions as seen by the
//               indicated lens.  Any triangle in the original mesh
//               that involves an unprojectable vertex is eliminated.
////////////////////////////////////////////////////////////////////
PT(Geom) ProjectionScreen::
make_mesh_geom(const Geom *geom, Lens *lens, LMatrix4f &rel_mat) {
  PT(Geom) new_geom = new Geom(*geom);

  GeomVertexRewriter vertex(new_geom->modify_vertex_data(), 
                              InternalName::get_vertex());
  while (!vertex.is_at_end()) {
    Vertexf vert = vertex.get_data3f();
    
    // Project each vertex into the film plane, but use three
    // dimensions so the Z coordinate remains meaningful.
    LPoint3f film(0.0f, 0.0f, 0.0f);
    lens->project(vert * rel_mat, film);
    
    vertex.set_data3f(film);
  }      
  
  return new_geom;
}
