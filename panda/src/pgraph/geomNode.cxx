// Filename: geomNode.cxx
// Created by:  drose (23eb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "geomNode.h"
#include "geom.h"
#include "geomTransformer.h"
#include "sceneGraphReducer.h"
#include "accumulatedAttribs.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "cullFaceAttrib.h"
#include "texMatrixAttrib.h"
#include "textureAttrib.h"
#include "shaderAttrib.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "cullableObject.h"
#include "cullHandler.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "indent.h"
#include "pset.h"
#include "config_pgraph.h"
#include "graphicsStateGuardianBase.h"
#include "boundingBox.h"
#include "boundingSphere.h"
#include "config_mathutil.h"


bool allow_flatten_color = ConfigVariableBool
    ("allow-flatten-color", false,
     PRC_DESC("allows color to always be flattened to vertices"));

TypeHandle GeomNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const string &name) :
  PandaNode(name)
{
  _preserved = preserve_geom_nodes;

  // GeomNodes have a certain set of bits on by default.
  set_into_collide_mask(get_default_collide_mask());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Copy Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
GeomNode(const GeomNode &copy) :
  PandaNode(copy),
  _preserved(copy._preserved),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::
~GeomNode() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated PandaNode that is a shallow
//               copy of this one.  It will be a different pointer,
//               but its internal data may or may not be shared with
//               that of the original PandaNode.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
PandaNode *GeomNode::
make_copy() const {
  return new GeomNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::apply_attribs_to_vertices
//       Access: Public, Virtual
//  Description: Applies whatever attributes are specified in the
//               AccumulatedAttribs object (and by the attrib_types
//               bitmask) to the vertices on this node, if
//               appropriate.  If this node uses geom arrays like a
//               GeomNode, the supplied GeomTransformer may be used to
//               unify shared arrays across multiple different nodes.
//
//               This is a generalization of xform().
////////////////////////////////////////////////////////////////////
void GeomNode::
apply_attribs_to_vertices(const AccumulatedAttribs &attribs, int attrib_types,
                          GeomTransformer &transformer) {
  if (pgraph_cat.is_debug()) {
    pgraph_cat.debug()
      << "Transforming geometry:\n";
    attribs.write(pgraph_cat.debug(false), attrib_types, 2);
  }

  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    if (!attribs._transform->is_identity()) {
      transformer.transform_vertices(this, attribs._transform->get_mat());
    }
  }

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    GeomList::iterator gi;
    PT(GeomList) geoms = cdata->modify_geoms();

    // Iterate based on the number of geoms, not using STL iterators.
    // This allows us to append to the list in the code below (which
    // we might do when doublesiding polys) without visiting those new
    // nodes during the traversal.
    size_t num_geoms = geoms->size();
    for (size_t i = 0; i < num_geoms; ++i) {
      GeomEntry *entry = &(*geoms)[i];
      PT(Geom) new_geom = entry->_geom.get_read_pointer()->make_copy();

      AccumulatedAttribs geom_attribs = attribs;
      entry->_state = geom_attribs.collect(entry->_state, attrib_types);

      bool any_changed = false;

      if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
        CPT(RenderAttrib) ra = geom_attribs._color;
        if (ra != (const RenderAttrib *)NULL) {
          int override = geom_attribs._color_override;
          entry->_state = entry->_state->add_attrib(ra, override);
        }

        ra = entry->_state->get_attrib_def(ColorAttrib::get_class_slot());
        CPT (ColorAttrib) ca = DCAST(ColorAttrib, ra);
        if (ca->get_color_type() != ColorAttrib::T_vertex) {
          if(allow_flatten_color) {
              if(transformer.set_color(new_geom, ca->get_color())) {
                any_changed = true;
                entry->_state = entry->_state->set_attrib(ColorAttrib::make_vertex());
              }
          } else {
            if (transformer.remove_column(new_geom, InternalName::get_color())) {
              any_changed = true;
            }
          }
        }
      }
      if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
        if (geom_attribs._color_scale != (const RenderAttrib *)NULL) {
          CPT(ColorScaleAttrib) csa = DCAST(ColorScaleAttrib, geom_attribs._color_scale);
          if (csa->get_scale() != LVecBase4(1.0f, 1.0f, 1.0f, 1.0f)) {


            // Now, if we have an "off" or "flat" color attribute, we
            // simply modify the color attribute, and leave the
            // vertices alone.
            CPT(RenderAttrib) ra = entry->_state->get_attrib_def(ColorAttrib::get_class_slot());
            CPT(ColorAttrib) ca = DCAST(ColorAttrib, ra);
            if(allow_flatten_color) {
              if (transformer.transform_colors(new_geom, csa->get_scale())) {
                any_changed = true;
              }
            } else {
              if (ca->get_color_type() == ColorAttrib::T_off) {
                entry->_state = entry->_state->set_attrib(ColorAttrib::make_vertex());
                // ColorAttrib::T_off means the color scale becomes
                // the new color.
                entry->_state = entry->_state->set_attrib(ColorAttrib::make_flat(csa->get_scale()));

              } else if (ca->get_color_type() == ColorAttrib::T_flat) {
                // ColorAttrib::T_flat means the color scale modulates
                // the specified color to produce a new color.
                const LColor &c1 = ca->get_color();
                const LVecBase4 &c2 = csa->get_scale();
                LColor color(c1[0] * c2[0], c1[1] * c2[1],
                             c1[2] * c2[2], c1[3] * c2[3]);
                entry->_state = entry->_state->set_attrib(ColorAttrib::make_flat(color));

              } else {
                // Otherwise, we have vertex color, and we just scale
                // it normally.
                if (transformer.transform_colors(new_geom, csa->get_scale())) {
                  any_changed = true;
                }
                entry->_state = entry->_state->set_attrib(ColorAttrib::make_vertex());
              }
            }
          }
        }
      }

      if ((attrib_types & SceneGraphReducer::TT_tex_matrix) != 0) {
        if (geom_attribs._tex_matrix != (const RenderAttrib *)NULL) {
          // Determine which texture coordinate names are used more than
          // once.  This assumes we have discovered all of the textures
          // that are in effect on the GeomNode; this may not be true if
          // there is a texture that has been applied at a node above
          // that from which we started the flatten operation, but
          // caveat programmer.
          NameCount name_count;

          if (geom_attribs._texture != (RenderAttrib *)NULL) {
            const TextureAttrib *ta = DCAST(TextureAttrib, geom_attribs._texture);
            int num_on_stages = ta->get_num_on_stages();
            for (int si = 0; si < num_on_stages; si++) {
              TextureStage *stage = ta->get_on_stage(si);
              const InternalName *name = stage->get_texcoord_name();
              count_name(name_count, name);
            }
          }

          const TexMatrixAttrib *tma =
            DCAST(TexMatrixAttrib, geom_attribs._tex_matrix);

          CPT(TexMatrixAttrib) new_tma = DCAST(TexMatrixAttrib, TexMatrixAttrib::make());

          int num_stages = tma->get_num_stages();
          for (int i = 0; i < num_stages; i++) {
            TextureStage *stage = tma->get_stage(i);
            InternalName *name = stage->get_texcoord_name();
            if (get_name_count(name_count, name) > 1) {
              // We can't transform these texcoords, since the name is
              // used by more than one active stage.
              new_tma = DCAST(TexMatrixAttrib, new_tma->add_stage(stage, tma->get_transform(stage)));

            } else {
              // It's safe to transform these texcoords; the name is
              // used by no more than one active stage.
              if (transformer.transform_texcoords(new_geom, name, name, tma->get_mat(stage))) {
                any_changed = true;
              }
            }
          }

          if (!new_tma->is_empty()) {
            entry->_state = entry->_state->add_attrib(new_tma);
          }
        }
      }

      if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
        entry->_state = geom_attribs._other->compose(entry->_state);
      }

      // We handle cull_face last, since that might involve
      // duplicating the geom, and we'd also like to duplicate all of
      // the changes we may have applied in the above.

      if ((attrib_types & SceneGraphReducer::TT_cull_face) != 0) {
        if (geom_attribs._cull_face != (const RenderAttrib *)NULL) {
          const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, geom_attribs._cull_face);
          CullFaceAttrib::Mode mode = cfa->get_effective_mode();
          switch (mode) {
          case CullFaceAttrib::M_cull_none:
            // Doublesided polys.  Duplicate them.
            {
              bool has_normals = (new_geom->get_vertex_data()->has_column(InternalName::get_normal()));
              if (has_normals) {
                // If the geometry has normals, we have to duplicate
                // it to reverse the normals on the duplicate copy.
                PT(Geom) dup_geom = new_geom->reverse();
                transformer.reverse_normals(dup_geom);

                geoms->push_back(GeomEntry(dup_geom, entry->_state));

                // The above push_back() operation might have
                // invalidated our old pointer into the list, so we
                // reassign it now.
                entry = &(*geoms)[i];

              } else {
                // If there are no normals, we can just doubleside it in
                // place.  This is preferable because we can share vertices.
                new_geom->doubleside_in_place();
                any_changed = true;
              }
            }
            break;

          case CullFaceAttrib::M_cull_counter_clockwise:
            // Reverse winding order.
            new_geom->reverse_in_place();
            transformer.reverse_normals(new_geom);
            any_changed = true;
            break;

          default:
            break;
          }
        }
      }

      if (any_changed) {
        entry->_geom = new_geom;
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if ((attrib_types & SceneGraphReducer::TT_apply_texture_color) != 0) {
    transformer.apply_texture_colors(this, attribs._other);
  }

  transformer.register_vertices(this, false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.  For most
//               kinds of nodes, this does nothing.
//
//               For a GeomNode, this does the right thing, but it is
//               better to use a GeomTransformer instead, since it
//               will share the new arrays properly between different
//               GeomNodes.
////////////////////////////////////////////////////////////////////
void GeomNode::
xform(const LMatrix4 &mat) {
  GeomTransformer transformer;
  transformer.transform_vertices(this, mat);
}


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of PandaNode by duplicating
//               instances (by calling dupe_for_flatten()), false
//               otherwise (for instance, a Camera cannot be safely
//               flattened, because the Camera pointer itself is
//               meaningful).
////////////////////////////////////////////////////////////////////
bool GeomNode::
safe_to_flatten() const {
  if (_preserved) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool GeomNode::
safe_to_combine() const {
  if (_preserved) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::r_prepare_scene
//       Access: Protected, Virtual
//  Description: The recursive implementation of prepare_scene().
//               Don't call this directly; call
//               PandaNode::prepare_scene() or
//               NodePath::prepare_scene() instead.
////////////////////////////////////////////////////////////////////
void GeomNode::
r_prepare_scene(GraphicsStateGuardianBase *gsg, const RenderState *node_state,
                GeomTransformer &transformer, Thread *current_thread) {
  PreparedGraphicsObjects *prepared_objects = gsg->get_prepared_objects();

  CDReader cdata(_cycler, current_thread);
  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    CPT(RenderState) geom_state = node_state->compose(entry._state);
    CPT(Geom) geom = entry._geom.get_read_pointer();

    // Munge the geom as required by the GSG.
    PT(GeomMunger) munger = gsg->get_geom_munger(geom_state, current_thread);
    geom = transformer.premunge_geom(geom, munger);

    // Prepare each of the vertex arrays in the munged Geom.
    CPT(GeomVertexData) vdata = geom->get_vertex_data(current_thread);
    vdata = vdata->animate_vertices(false, current_thread);
    GeomVertexDataPipelineReader vdata_reader(vdata, current_thread);
    int num_arrays = vdata_reader.get_num_arrays();
    for (int i = 0; i < num_arrays; ++i) {
      CPT(GeomVertexArrayData) array = vdata_reader.get_array(i);
      ((GeomVertexArrayData *)array.p())->prepare(prepared_objects);
    }

    // And also each of the index arrays.
    int num_primitives = geom->get_num_primitives();
    for (int i = 0; i < num_primitives; ++i) {
      CPT(GeomPrimitive) prim = geom->get_primitive(i);
      ((GeomPrimitive *)prim.p())->prepare(prepared_objects);
    }

    // And now prepare each of the textures.
    const RenderAttrib *attrib =
      geom_state->get_attrib(TextureAttrib::get_class_slot());
    if (attrib != (const RenderAttrib *)NULL) {
      const TextureAttrib *ta;
      DCAST_INTO_V(ta, attrib);
      int num_stages = ta->get_num_on_stages();
      for (int i = 0; i < num_stages; ++i) {
        Texture *texture = ta->get_on_texture(ta->get_on_stage(i));
        //TODO: prepare the sampler states, if specified.
        if (texture != (Texture *)NULL) {
          texture->prepare(prepared_objects);
        }
      }
    }

    // As well as the shaders.
    attrib = geom_state->get_attrib(ShaderAttrib::get_class_slot());
    if (attrib != (const RenderAttrib *)NULL) {
      const ShaderAttrib *sa;
      DCAST_INTO_V(sa, attrib);
      Shader *shader = (Shader *)sa->get_shader();
      if (shader != (Shader *)NULL) {
        shader->prepare(prepared_objects);
      }
      //TODO: prepare the shader inputs.
      //TODO: Invoke the shader generator if enabled.
    }
  }

  PandaNode::r_prepare_scene(gsg, node_state, transformer, current_thread);
}


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::combine_with
//       Access: Public, Virtual
//  Description: Collapses this node with the other node, if possible,
//               and returns a pointer to the combined node, or NULL
//               if the two nodes cannot safely be combined.
//
//               The return value may be this, other, or a new node
//               altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two nodes and what the
//               collapsed node should look like.
////////////////////////////////////////////////////////////////////
PandaNode *GeomNode::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two GeomNodes can combine by moving Geoms from one to the other.
    GeomNode *gother = DCAST(GeomNode, other);
    add_geoms_from(gother);
    return this;
  }

  return PandaNode::combine_with(other);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::calc_tight_bounds
//       Access: Public, Virtual
//  Description: This is used to support
//               NodePath::calc_tight_bounds().  It is not intended to
//               be called directly, and it has nothing to do with the
//               normal Panda bounding-volume computation.
//
//               If the node contains any geometry, this updates
//               min_point and max_point to enclose its bounding box.
//               found_any is to be set true if the node has any
//               geometry at all, or left alone if it has none.  This
//               method may be called over several nodes, so it may
//               enter with min_point, max_point, and found_any
//               already set.
////////////////////////////////////////////////////////////////////
CPT(TransformState) GeomNode::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform =
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform,
                                 current_thread);

  const LMatrix4 &mat = next_transform->get_mat();

  CDReader cdata(_cycler, current_thread);
  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    CPT(Geom) geom = (*gi)._geom.get_read_pointer();
    geom->calc_tight_bounds(min_point, max_point, found_any,
                            geom->get_vertex_data(current_thread)->animate_vertices(true, current_thread),
                            !next_transform->is_identity(), mat,
                            current_thread);
  }

  return next_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::is_renderable
//       Access: Public, Virtual
//  Description: Returns true if there is some value to visiting this
//               particular node during the cull traversal for any
//               camera, false otherwise.  This will be used to
//               optimize the result of get_net_draw_show_mask(), so
//               that any subtrees that contain only nodes for which
//               is_renderable() is false need not be visited.
////////////////////////////////////////////////////////////////////
bool GeomNode::
is_renderable() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_for_draw
//       Access: Public, Virtual
//  Description: Adds the node's contents to the CullResult we are
//               building up during the cull traversal, so that it
//               will be drawn at render time.  For most nodes other
//               than GeomNodes, this is a do-nothing operation.
////////////////////////////////////////////////////////////////////
void GeomNode::
add_for_draw(CullTraverser *trav, CullTraverserData &data) {
  trav->_geom_nodes_pcollector.add_level(1);

  if (pgraph_cat.is_spam()) {
    pgraph_cat.spam()
      << "Found " << *this << " in state " << *data._state
      << " draw_mask = " << data._draw_mask << "\n";
  }

  // Get all the Geoms, with no decalling.
  Geoms geoms = get_geoms(trav->get_current_thread());
  int num_geoms = geoms.get_num_geoms();
  trav->_geoms_pcollector.add_level(num_geoms);
  CPT(TransformState) internal_transform = data.get_internal_transform(trav);

  for (int i = 0; i < num_geoms; i++) {
    const Geom *geom = geoms.get_geom(i);
    if (geom->is_empty()) {
      continue;
    }

    CPT(RenderState) state = data._state->compose(geoms.get_geom_state(i));
    if (state->has_cull_callback() && !state->cull_callback(trav, data)) {
      // Cull.
      continue;
    }

    // Cull the Geom bounding volume against the view frustum
    // and/or the cull planes.  Don't bother unless we've got more
    // than one Geom, since otherwise the bounding volume of the
    // GeomNode is (probably) the same as that of the one Geom,
    // and we've already culled against that.
    if (num_geoms > 1) {
      if (data._view_frustum != (GeometricBoundingVolume *)NULL) {
        // Cull the individual Geom against the view frustum.
        CPT(BoundingVolume) geom_volume = geom->get_bounds();
        const GeometricBoundingVolume *geom_gbv =
          DCAST(GeometricBoundingVolume, geom_volume);

        int result = data._view_frustum->contains(geom_gbv);
        if (result == BoundingVolume::IF_no_intersection) {
          // Cull this Geom.
          continue;
        }
      }
      if (!data._cull_planes->is_empty()) {
        // Also cull the Geom against the cull planes.
        CPT(BoundingVolume) geom_volume = geom->get_bounds();
        const GeometricBoundingVolume *geom_gbv =
          DCAST(GeometricBoundingVolume, geom_volume);
        int result;
        data._cull_planes->do_cull(result, state, geom_gbv);
        if (result == BoundingVolume::IF_no_intersection) {
          // Cull.
          continue;
        }
      }
    }

    CullableObject *object =
      new CullableObject(geom, state, internal_transform);
    trav->get_cull_handler()->record_object(object, trav);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::get_legal_collide_mask
//       Access: Published, Virtual
//  Description: Returns the subset of CollideMask bits that may be
//               set for this particular type of PandaNode.  For most
//               nodes, this is 0; it doesn't make sense to set a
//               CollideMask for most kinds of nodes.
//
//               For nodes that can be collided with, such as GeomNode
//               and CollisionNode, this returns all bits on.
////////////////////////////////////////////////////////////////////
CollideMask GeomNode::
get_legal_collide_mask() const {
  return CollideMask::all_on();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geom
//       Access: Published
//  Description: Adds a new Geom to the node.  The geom is given the
//               indicated state (which may be
//               RenderState::make_empty(), to completely inherit its
//               state from the scene graph).
////////////////////////////////////////////////////////////////////
void GeomNode::
add_geom(Geom *geom, const RenderState *state) {
  nassertv(geom != (Geom *)NULL);
  nassertv(geom->check_valid());
  nassertv(state != (RenderState *)NULL);

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    cdata->modify_geoms()->push_back(GeomEntry(geom, state));
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::add_geoms_from
//       Access: Published
//  Description: Copies the Geoms (and their associated RenderStates)
//               from the indicated GeomNode into this one.
////////////////////////////////////////////////////////////////////
void GeomNode::
add_geoms_from(const GeomNode *other) {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    CDStageReader cdata_other(other->_cycler, pipeline_stage, current_thread);

    GeomList::const_iterator gi;
    CPT(GeomList) other_geoms = cdata_other->get_geoms();
    PT(GeomList) this_geoms = cdata->modify_geoms();
    for (gi = other_geoms->begin(); gi != other_geoms->end(); ++gi) {
      const GeomEntry &entry = (*gi);
      nassertv(entry._geom.get_read_pointer()->check_valid());
      this_geoms->push_back(entry);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::set_geom
//       Access: Public
//  Description: Replaces the nth Geom of the node with a new pointer.
//               There must already be a Geom in this slot.
//
//               Note that if this method is called in a downstream
//               stage (for instance, during cull or draw), then it
//               will propagate the new list of Geoms upstream all the
//               way to pipeline stage 0, which may step on changes
//               that were made independently in pipeline stage 0.
//               Use with caution.
////////////////////////////////////////////////////////////////////
void GeomNode::
set_geom(int n, Geom *geom) {
  nassertv(geom != (Geom *)NULL);
  nassertv(geom->check_valid());

  CDWriter cdata(_cycler, true);
  PT(GeomList) geoms = cdata->modify_geoms();
  nassertv(n >= 0 && n < (int)geoms->size());
  (*geoms)[n]._geom = geom;

  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::check_valid
//       Access: Published
//  Description: Verifies that the each Geom within the GeomNode
//               reference vertices that actually exist within its
//               GeomVertexData.  Returns true if the GeomNode appears
//               to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomNode::
check_valid() const {
  int num_geoms = get_num_geoms();
  for (int i = 0; i < num_geoms; i++) {
    const Geom *geom = get_geom(i);
    if (!geom->check_valid()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::decompose
//       Access: Published
//  Description: Calls decompose() on each Geom with the GeomNode.
//               This decomposes higher-order primitive types, like
//               triangle strips, into lower-order types like indexed
//               triangles.  Normally there is no reason to do this,
//               but it can be useful as an early preprocessing step,
//               to allow a later call to unify() to proceed more
//               quickly.
//
//               See also SceneGraphReducer::decompose(), which is the
//               normal way this is called.
////////////////////////////////////////////////////////////////////
void GeomNode::
decompose() {
  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    GeomList::iterator gi;
    PT(GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomEntry &entry = (*gi);
      nassertv(entry._geom.test_ref_count_integrity());
      PT(Geom) geom = entry._geom.get_write_pointer();
      geom->decompose_in_place();
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::unify
//       Access: Published
//  Description: Attempts to unify all of the Geoms contained within
//               this node into a single Geom, or at least as few
//               Geoms as possible.  In turn, the individual
//               GeomPrimitives contained within each resulting Geom
//               are also unified.  The goal is to reduce the number
//               of GeomPrimitives within the node as far as possible.
//               This may result in composite primitives, such as
//               triangle strips and triangle fans, being decomposed
//               into triangles.  See also Geom::unify().
//
//               max_indices represents the maximum number of indices
//               that will be put in any one GeomPrimitive.  If
//               preserve_order is true, then the primitives will not
//               be reordered during the operation, even if this
//               results in a suboptimal result.
//
//               In order for this to be successful, the primitives
//               must reference the same GeomVertexData, have the same
//               fundamental primitive type, and have compatible shade
//               models.
////////////////////////////////////////////////////////////////////
void GeomNode::
unify(int max_indices, bool preserve_order) {
  bool any_changed = false;

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    PT(GeomList) new_geoms = new GeomList;

    // Try to unify each Geom with each preceding Geom.  This is an n^2
    // operation, but usually there are only a handful of Geoms to
    // consider, so that's not a big deal.
    GeomList::const_iterator gi;
    CPT(GeomList) old_geoms = cdata->get_geoms();
    for (gi = old_geoms->begin(); gi != old_geoms->end(); ++gi) {
      const GeomEntry &old_entry = (*gi);

      bool unified = false;

      // Go from back to front, to minimize damage to the primitive ordering.
      GeomList::reverse_iterator gj;
      for (gj = new_geoms->rbegin(); gj != new_geoms->rend() && !unified; ++gj) {
        GeomEntry &new_entry = (*gj);
        if (old_entry._state == new_entry._state) {
          // Both states match, so try to combine the primitives.
          CPT(Geom) old_geom = old_entry._geom.get_read_pointer();
          PT(Geom) new_geom = new_entry._geom.get_write_pointer();
          if (new_geom->copy_primitives_from(old_geom)) {
            // Successfully combined!
            unified = true;
            any_changed = true;
          }
        }

        if (preserve_order) {
          // If we're insisting on preserving the order, we can only
          // attempt to merge with the tail of the list.
          break;
        }
      }

      if (!unified) {
        // Couldn't unify this Geom with anything, so just add it to the
        // output list.
        new_geoms->push_back(old_entry);
      }
    }

    // Done!  We'll keep whatever's left in the output list.
    cdata->set_geoms(new_geoms);

    // Finally, go back through and unify the resulting geom(s).
    GeomList::iterator wgi;
    for (wgi = new_geoms->begin(); wgi != new_geoms->end(); ++wgi) {
      GeomEntry &entry = (*wgi);
      nassertv(entry._geom.test_ref_count_integrity());
      PT(Geom) geom = entry._geom.get_write_pointer();
      geom->unify_in_place(max_indices, preserve_order);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);

  if (any_changed) {
    mark_internal_bounds_stale();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_geoms
//       Access: Published
//  Description: Writes a short description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_geoms(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  write(out, indent_level);
  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    indent(out, indent_level + 2)
      << *entry._geom.get_read_pointer() << " " << *entry._state << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_verbose
//       Access: Published
//  Description: Writes a detailed description of all the Geoms in the
//               node.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_verbose(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);
  write(out, indent_level);
  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    CPT(Geom) geom = entry._geom.get_read_pointer();
    indent(out, indent_level + 2)
      << *geom << " " << *entry._state << "\n";
    geom->write(out, indent_level + 4);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void GeomNode::
output(ostream &out) const {
  // Accumulate the total set of RenderAttrib types that are applied
  // to any of our Geoms, so we can output them too.  The result will
  // be the list of attrib types that might be applied to some Geoms,
  // but not necessarily to all Geoms.

  CDReader cdata(_cycler);

  pset<TypeHandle> attrib_types;

  GeomList::const_iterator gi;
  CPT(RenderState) common = RenderState::make_empty();

  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    common = common->compose(entry._state);
  }

  PandaNode::output(out);
  out << " (" << geoms->size() << " geoms";

  if (!common->is_empty()) {
    out << ": " << *common;
  }

  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::is_geom_node
//       Access: Public, Virtual
//  Description: A simple downcast check.  Returns true if this kind
//               of node happens to inherit from GeomNode, false
//               otherwise.
//
//               This is provided as a a faster alternative to calling
//               is_of_type(GeomNode::get_class_type()), since this
//               test is so important to rendering.
////////////////////////////////////////////////////////////////////
bool GeomNode::
is_geom_node() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::do_premunge
//       Access: Public
//  Description: Uses the indicated GSG to premunge the Geoms in this
//               node to optimize them for eventual rendering.  See
//               SceneGraphReducer::premunge().
////////////////////////////////////////////////////////////////////
void GeomNode::
do_premunge(GraphicsStateGuardianBase *gsg,
            const RenderState *node_state,
            GeomTransformer &transformer) {
  Thread *current_thread = Thread::get_current_thread();

  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    GeomList::iterator gi;
    PT(GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomEntry &entry = (*gi);
      CPT(RenderState) geom_state = node_state->compose(entry._state);
      CPT(Geom) geom = entry._geom.get_read_pointer();
      PT(GeomMunger) munger = gsg->get_geom_munger(geom_state, current_thread);
      entry._geom = transformer.premunge_geom(geom, munger);
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::r_mark_geom_bounds_stale
//       Access: Protected, Virtual
//  Description: Recursively calls Geom::mark_bounds_stale() on every
//               Geom at this node and below.
////////////////////////////////////////////////////////////////////
void GeomNode::
r_mark_geom_bounds_stale(Thread *current_thread) {
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

    GeomList::iterator gi;
    PT(GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomEntry &entry = (*gi);
      entry._geom.get_read_pointer()->mark_bounds_stale();
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  mark_internal_bounds_stale();

  PandaNode::r_mark_geom_bounds_stale(current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated BoundingVolume that
//               represents the internal contents of the node.  Should
//               be overridden by PandaNode classes that contain
//               something internally.
////////////////////////////////////////////////////////////////////
void GeomNode::
compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                        int &internal_vertices,
                        int pipeline_stage,
                        Thread *current_thread) const {
  int num_vertices = 0;

  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);

  pvector<const BoundingVolume *> child_volumes;
  pvector<CPT(BoundingVolume) > child_volumes_ref;
  bool all_box = true;

  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  child_volumes.reserve(geoms->size());
  child_volumes_ref.reserve(geoms->size());

  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    CPT(Geom) geom = entry._geom.get_read_pointer();
    CPT(BoundingVolume) volume = geom->get_bounds();

    if (!volume->is_empty()) {
      child_volumes.push_back(volume);
      child_volumes_ref.push_back(volume);
      if (!volume->is_exact_type(BoundingBox::get_class_type())) {
        all_box = false;
      }
    }
    num_vertices += geom->get_nested_vertices();
  }

  PT(GeometricBoundingVolume) gbv;

  BoundingVolume::BoundsType btype = get_bounds_type();
  if (btype == BoundingVolume::BT_default) {
    btype = bounds_type;
  }

  if (btype == BoundingVolume::BT_box ||
      (btype != BoundingVolume::BT_sphere && all_box)) {
    // If all of the child volumes are a BoundingBox, then our volume
    // is also a BoundingBox.
    gbv = new BoundingBox;
  } else {
    // Otherwise, it's a sphere.
    gbv = new BoundingSphere;
  }

  if (child_volumes.size() > 0) {
    const BoundingVolume **child_begin = &child_volumes[0];
    const BoundingVolume **child_end = child_begin + child_volumes.size();
    ((BoundingVolume *)gbv)->around(child_begin, child_end);
  }

  internal_bounds = gbv;
  internal_vertices = num_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void GeomNode::
finalize(BamReader *manager) {
  if (manager->get_file_minor_ver() < 14) {
    // With version 6.14, we changed the default ColorAttrib
    // behavior from make_vertex() to make_flat().  This means that
    // every Geom that contains vertex colors now needs to have an
    // explicit ColorAttrib::make_vertex() on its state.

    // Since we shouldn't override a different ColorAttrib inherited
    // from above, we create this new attrib with an override of -1.

    CPT(InternalName) color = InternalName::get_color();
    CPT(RenderAttrib) vertex_color = ColorAttrib::make_vertex();

    Thread *current_thread = Thread::get_current_thread();
    OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
      CDStageWriter cdata(_cycler, pipeline_stage, current_thread);

      GeomList::iterator gi;
      PT(GeomList) geoms = cdata->modify_geoms();
      for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
        GeomEntry &entry = (*gi);
        CPT(Geom) geom = entry._geom.get_read_pointer();

        // Force the various GeomVertexArrayFormat objects to finalize
        // themselves.  We have to do this before we can reliably call
        // GeomVertexData::has_column().
        CPT(GeomVertexData) vdata = geom->get_vertex_data(current_thread);
        CPT(GeomVertexFormat) vformat = vdata->get_format();
        for (int i = 0; i < vformat->get_num_arrays(); ++i) {
          const GeomVertexArrayFormat *varray = vformat->get_array(i);
          manager->finalize_now((GeomVertexArrayFormat *)varray);
        }

        if (vdata->has_column(color) &&
            !entry._state->has_attrib(ColorAttrib::get_class_slot())) {
          // We'll be reassigning the RenderState.  Therefore, save it
          // temporarily to increment its reference count.
          PT(BamAuxData) aux_data = new BamAuxData;
          aux_data->_hold_state = entry._state;
          manager->set_aux_data((RenderState *)entry._state.p(), "hold_state", aux_data);

          entry._state = entry._state->add_attrib(vertex_color, -1);
        }
      }
    }
    CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type GeomNode is encountered
//               in the Bam file.  It should create the GeomNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomNode::
make_from_bam(const FactoryParams &params) {
  GeomNode *node = new GeomNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  if (manager->get_file_minor_ver() < 14) {
    manager->register_finalize(node);
  }

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomNode::CData::
CData(const GeomNode::CData &copy) :
  _geoms(copy._geoms)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *GeomNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  CPT(GeomList) geoms = _geoms.get_read_pointer();
  int num_geoms = geoms->size();
  nassertv(num_geoms == (int)(PN_uint16)num_geoms);
  dg.add_uint16(num_geoms);

  GeomList::const_iterator gi;
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    manager->write_pointer(dg, entry._geom.get_read_pointer());
    manager->write_pointer(dg, entry._state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int GeomNode::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  // Get the geom and state pointers.
  GeomList::iterator gi;
  PT(GeomList) geoms = _geoms.get_write_pointer();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    GeomEntry &entry = (*gi);
    entry._geom = DCAST(Geom, p_list[pi++]);
    entry._state = DCAST(RenderState, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomNode.
////////////////////////////////////////////////////////////////////
void GeomNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  int num_geoms = scan.get_uint16();
  // Read the list of geoms and states.  Push back a NULL for each one.
  PT(GeomList) geoms = new GeomList;
  geoms->reserve(num_geoms);
  for (int i = 0; i < num_geoms; i++) {
    manager->read_pointer(scan);
    manager->read_pointer(scan);
    geoms->push_back(GeomEntry(NULL, NULL));
  }
  _geoms = geoms;
}
