// Filename: projectionScreen.cxx
// Created by:  drose (11Dec01)
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

#include "projectionScreen.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTristrip.h"
#include "renderRelation.h"
#include "transformTransition.h"
#include "get_rel_pos.h"

TypeHandle ProjectionScreen::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ProjectionScreen::
ProjectionScreen(const string &name) : NamedNode(name)
{
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
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
ProjectionScreen::
ProjectionScreen(const ProjectionScreen &copy) :
  NamedNode(copy),
  _projector(copy._projector),
  _vignette_on(copy._vignette_on),
  _vignette_color(copy._vignette_color),
  _frame_color(copy._frame_color)
{
  _computed_rel_top_mat = false;
  _stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
operator = (const ProjectionScreen &copy) {
  NamedNode::operator = (copy);
  _projector = copy._projector;
  _vignette_on = copy._vignette_on;
  _vignette_color = copy._vignette_color;
  _frame_color = copy._frame_color;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
Node *ProjectionScreen::
make_copy() const {
  return new ProjectionScreen(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::app_traverse
//       Access: Public, Virtual
//  Description: This is called by the App traversal by virtue of the
//               ProjectionScreen node's being present in the scene graph.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
app_traverse(const ArcChain &) {
  recompute_if_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::generate_screen
//       Access: Published
//  Description: Synthesizes a polygon mesh based on the projection
//               area of the indicated projector.  This generates a
//               new GeomNode and automatically parents it to the
//               ProjectionScreen; the new GeomNode is also returned
//               for reference.
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
////////////////////////////////////////////////////////////////////
GeomNode *ProjectionScreen::
generate_screen(LensNode *projector, const string &screen_name,
                int num_x_verts, int num_y_verts, float distance) {
  nassertr(projector != (LensNode *)NULL, NULL);
  nassertr(projector->get_lens() != NULL, NULL);

  // First, get the relative coordinate space of the projector.
  LMatrix4f rel_mat;
  get_rel_mat(projector, this, rel_mat);

  // Now compute all the vertices for the screen.  These are arranged
  // in order from left to right and bottom to top.
  int num_verts = num_x_verts * num_y_verts;
  Lens *lens = projector->get_lens();
  float t = (distance - lens->get_near()) / (lens->get_far() - lens->get_near());

  PTA_Vertexf coords;
  coords.reserve(num_verts);
  float x_scale = 2.0f / (num_x_verts - 1);
  float y_scale = 2.0f / (num_y_verts - 1);
  for (int yi = 0; yi < num_y_verts; yi++) {
    for (int xi = 0; xi < num_x_verts; xi++) {
      LPoint2f film = LPoint2f((float)xi * x_scale - 1.0f,
                               (float)yi * y_scale - 1.0f);
      LPoint3f near_point, far_point;
      lens->extrude(film, near_point, far_point);
      
      LPoint3f point = near_point + t * (far_point - near_point);
      point = point * rel_mat;
      
      coords.push_back(point);
    }
  }
  nassertr((int)coords.size() == num_verts, NULL);

  // Now synthesize a triangle mesh.  We run triangle strips
  // horizontally across the grid.
  int num_tstrips = (num_y_verts-1);
  int tstrip_length = 2*(num_x_verts-1)+2;

  PTA_int lengths;
  PTA_ushort vindex;

  // Set the lengths array.  we are creating num_tstrips t-strips,
  // each of which has tstrip_length vertices.
  lengths.reserve(num_tstrips);
  int n;
  for (n = 0; n < num_tstrips; n++) {
    lengths.push_back(tstrip_length);
  }
  nassertr((int)lengths.size() == num_tstrips, NULL);

  // Now fill up the index array into the vertices.  This lays out the
  // order of the vertices in each t-strip.
  vindex.reserve(num_tstrips * tstrip_length);
  n = 0;
  int ti, si;
  for (ti = 1; ti < num_y_verts; ti++) {
    vindex.push_back(ti * num_x_verts);
    for (si = 1; si < num_x_verts; si++) {
      vindex.push_back((ti - 1) * num_x_verts + (si-1));
      vindex.push_back(ti * num_x_verts + si);
    }
    vindex.push_back((ti - 1) * num_x_verts + (num_x_verts-1));
  }
  nassertr((int)vindex.size() == num_tstrips * tstrip_length, NULL);

  GeomTristrip *geom = new GeomTristrip;
  geom->set_num_prims(num_tstrips);
  geom->set_lengths(lengths);

  geom->set_coords(coords, G_PER_VERTEX, vindex);

  // Make it white.
  PTA_Colorf colors;
  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  geom->set_colors(colors, G_OVERALL);

  // Now create a GeomNode to hold this mesh.
  GeomNode *geom_node = new GeomNode(screen_name);
  geom_node->add_geom(geom);

  // And parent it to ourselves.
  new RenderRelation(this, geom_node);
  _stale = true;

  return geom_node;
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
//               The returned node has no parent; it is up to the user
//               to caller to parent it somewhere or store it so that
//               it does not get dereferenced and deleted.
////////////////////////////////////////////////////////////////////
PT_Node ProjectionScreen::
make_flat_mesh(LensNode *camera) {
  nassertr(camera != (LensNode *)NULL, NULL);
  nassertr(camera->get_lens() != (Lens *)NULL, NULL);

  // First, ensure the UV's are up-to-date.
  recompute_if_stale();

  PT_Node top = new NamedNode(get_name());

  LMatrix4f rel_mat;
  bool computed_rel_mat = false;
  make_mesh_children(top, this, camera, rel_mat, computed_rel_mat);

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
  if (_projector != (LensNode *)NULL && 
      _projector->get_lens() != (Lens *)NULL) {
    _colors.clear();
    _colors.push_back(_vignette_color);
    _colors.push_back(_frame_color);

    recompute_node(this, _rel_top_mat, _computed_rel_top_mat);
    // Make sure this flag is set to false for next time.
    _computed_rel_top_mat = false;

    _projector_lens_change = _projector->get_lens()->get_last_change();
    _stale = false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_if_stale
//       Access: Private
//  Description: Calls recompute() only if the relative transform
//               between the ProjectionScreen and the projector has
//               changed, or if any other relevant property has
//               changed.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_if_stale() {
  if (_projector != (LensNode *)NULL && 
      _projector->get_lens() != (Lens *)NULL) {
    UpdateSeq lens_change = _projector->get_lens()->get_last_change();
    if (_stale || lens_change != _projector_lens_change) {
      recompute();

    } else {
      // Get the relative transform to ensure it hasn't changed.
      LMatrix4f top_mat;
      get_rel_mat(this, _projector, top_mat);
      if (!_rel_top_mat.almost_equal(top_mat)) {
        _rel_top_mat = top_mat;
        _computed_rel_top_mat = true;
        recompute();
      }
    }
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
recompute_node(Node *node, LMatrix4f &rel_mat, bool &computed_rel_mat) {
  if (node->is_of_type(GeomNode::get_class_type())) {
    recompute_geom_node(DCAST(GeomNode, node), rel_mat, computed_rel_mat);
  }

  // Now recurse on children.
  int num_children = node->get_num_children(RenderRelation::get_class_type());
  for (int i = 0; i < num_children; i++) {
    NodeRelation *arc = node->get_child(RenderRelation::get_class_type(), i);

    if (arc->has_transition(TransformTransition::get_class_type())) {
      // This arc has a transform; therefore, we must recompute the
      // relative matrix from this point.
      LMatrix4f new_rel_mat;
      bool computed_new_rel_mat = false;
      recompute_node(arc->get_child(), new_rel_mat, computed_new_rel_mat);
    } else {
      // This arc has no transform, so we can use the same transform
      // space from before.
      recompute_node(arc->get_child(), rel_mat, computed_rel_mat);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_geom_node
//       Access: Private
//  Description: Recomputes the UV's just for the indicated GeomNode.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_geom_node(GeomNode *node, LMatrix4f &rel_mat, bool &computed_rel_mat) {
  if (!computed_rel_mat) {
    // All right, time to compute the matrix.
    get_rel_mat(node, _projector, rel_mat);
    computed_rel_mat = true;
  }

  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    dDrawable *drawable = node->get_geom(i);
    if (drawable->is_of_type(Geom::get_class_type())) {
      recompute_geom(DCAST(Geom, drawable), rel_mat);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::recompute_geom
//       Access: Private
//  Description: Recomputes the UV's just for the indicated Geom.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
recompute_geom(Geom *geom, const LMatrix4f &rel_mat) {
  static const LMatrix3f LensToUv
    (0.5f, 0.0f, 0.0f,
     0.0f, 0.5f, 0.0f,
     0.5f, 0.5f, 1.0f);

  PTA_TexCoordf uvs;
  PTA_ushort color_index;
  Lens *lens = _projector->get_lens();
  nassertv(lens != (Lens *)NULL);

  // Iterate through all the vertices in the Geom.
  int num_vertices = geom->get_num_vertices();
  Geom::VertexIterator vi = geom->make_vertex_iterator();

  for (int i = 0; i < num_vertices; i++) {
    const Vertexf &vert = geom->get_next_vertex(vi);

    // For each vertex, project to the film plane.
    LPoint2f film(0.0, 0.0);
    bool good = lens->project(vert * rel_mat, film);

    // Now the lens gives us coordinates in the range [-1, 1].
    // Rescale these to [0, 1].
    uvs.push_back(film * LensToUv);

    // If we have vignette color in effect, color the vertex according
    // to whether it fell in front of the lens or not.
    if (_vignette_on) {
      color_index.push_back(good ? 1 : 0);
    }
  }

  // Now set the UV's.
  geom->set_texcoords(uvs, G_PER_VERTEX);

  if (_vignette_on) {
    geom->set_colors(_colors, G_PER_VERTEX, color_index);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_node
//       Access: Private
//  Description: Recurses over all geometry at the indicated node and
//               below, and generates a corresponding node hierarchy
//               with all the geometry copied, but flattened into 2-d,
//               as seen from the indicated camera.  Returns the newly
//               created, arc, or NULL if no arc was created.
////////////////////////////////////////////////////////////////////
NodeRelation *ProjectionScreen::
make_mesh_node(Node *result_parent, Node *node, LensNode *camera,
               LMatrix4f &rel_mat, bool &computed_rel_mat) {
  if (!node->safe_to_flatten()) {
    // If we can't safely flatten this node, ignore it (and all of its
    // children) completely.  It's got no business being here anyway.
    return NULL;
  }

  PT(Node) new_node;
  if (node->is_of_type(GeomNode::get_class_type())) {
    new_node =
      make_mesh_geom_node(DCAST(GeomNode, node), camera,
                          rel_mat, computed_rel_mat);
  } else {
    new_node = node->make_copy();
  }

  // Now attach the new node to the result.
  RenderRelation *arc = new RenderRelation(result_parent, new_node);
  make_mesh_children(new_node, node, camera, rel_mat, computed_rel_mat);
  return arc;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_children
//       Access: Private
//  Description: Walks over the list of children for the indicated
//               node, calling make_mesh_node() on each one.
////////////////////////////////////////////////////////////////////
void ProjectionScreen::
make_mesh_children(Node *new_node, Node *node, LensNode *camera,
                   LMatrix4f &rel_mat, bool &computed_rel_mat) {
  int num_children = node->get_num_children(RenderRelation::get_class_type());
  for (int i = 0; i < num_children; i++) {
    NodeRelation *arc = node->get_child(RenderRelation::get_class_type(), i);
    NodeRelation *new_arc;

    if (arc->has_transition(TransformTransition::get_class_type())) {
      // This arc has a transform; therefore, we must recompute the
      // relative matrix from this point.
      LMatrix4f new_rel_mat;
      bool computed_new_rel_mat = false;
      new_arc = make_mesh_node(new_node, arc->get_child(), camera,
                               new_rel_mat, computed_new_rel_mat);
    } else {
      // This arc has no transform, so we can use the same transform
      // space from before.
      new_arc = make_mesh_node(new_node, arc->get_child(), camera,
                               rel_mat, computed_rel_mat);
    }

    // Copy all of the attributes (except TransformTransition) to the
    // new arc.
    new_arc->copy_transitions_from(arc);
    new_arc->clear_transition(TransformTransition::get_class_type());
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
make_mesh_geom_node(GeomNode *node, LensNode *camera,
                    LMatrix4f &rel_mat, bool &computed_rel_mat) {
  PT(GeomNode) new_node = new GeomNode;
  new_node->set_name(node->get_name());

  if (!computed_rel_mat) {
    // All right, time to compute the matrix.
    get_rel_mat(node, camera, rel_mat);
    computed_rel_mat = true;
  }

  int num_geoms = node->get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    dDrawable *drawable = node->get_geom(i);
    if (drawable->is_of_type(Geom::get_class_type())) {
      PT(dDrawable) new_drawable = 
        make_mesh_geom(DCAST(Geom, drawable), camera->get_lens(), rel_mat);
      if (new_drawable != (dDrawable *)NULL) {
        new_node->add_geom(new_drawable);
      }
    }
  }

  return new_node;
}

////////////////////////////////////////////////////////////////////
//     Function: ProjectionScreen::make_mesh_geom
//       Access: Private
//  Description: Makes a new Geom, just like the given one, except
//               flattened into two dimensions as seen by the
//               indicated lens.
////////////////////////////////////////////////////////////////////
PT(dDrawable) ProjectionScreen::
make_mesh_geom(Geom *geom, Lens *lens, LMatrix4f &rel_mat) {
  Geom *new_geom = geom->make_copy();
  PT(dDrawable) result = new_geom;

  PTA_Vertexf coords;
  GeomBindType bind;
  PTA_ushort vindex;

  new_geom->get_coords(coords, bind, vindex);

  PTA_Vertexf new_coords;
  new_coords.reserve(coords.size());
  for (int i = 0; i < (int)coords.size(); i++) {
    const Vertexf &vert = coords[i];

    // Project each vertex into the film plane, but use three
    // dimensions so the Z coordinate remains meaningful.
    LPoint3f film(0.0f, 0.0f, 0.0f);
    lens->project(vert * rel_mat, film);

    new_coords.push_back(film);
  }

  new_geom->set_coords(new_coords, bind, vindex);

  return result;
}
