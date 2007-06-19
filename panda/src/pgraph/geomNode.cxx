// Filename: geomNode.cxx
// Created by:  drose (23Feb02)
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

#include "geomNode.h"
#include "geom.h"
#include "geomTransformer.h"
#include "sceneGraphReducer.h"
#include "accumulatedAttribs.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "texMatrixAttrib.h"
#include "textureAttrib.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "indent.h"
#include "pset.h"
#include "config_pgraph.h"
#include "graphicsStateGuardianBase.h"
#include "boundingBox.h"
#include "config_mathutil.h"

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
  // GeomNodes have a certain set of bits on by default.
  _preserved = false;
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
      << "Transforming geometry.\n";
  }

  if ((attrib_types & SceneGraphReducer::TT_transform) != 0) {
    if (!attribs._transform->is_identity()) {
      transformer.transform_vertices(this, attribs._transform->get_mat());
    }
  }

  if ((attrib_types & SceneGraphReducer::TT_cull_face) != 0) {
    if (attribs._cull_face != (const RenderAttrib *)NULL) {
      const CullFaceAttrib *cfa = DCAST(CullFaceAttrib, attribs._cull_face);
      CullFaceAttrib::Mode mode = cfa->get_effective_mode();
      switch (mode) {
      case CullFaceAttrib::M_cull_none:
        // Doublesided polys.  Duplicate them.
        transformer.doubleside(this);
        break;
        
      case CullFaceAttrib::M_cull_counter_clockwise:
        // Reverse winding order.
        transformer.reverse(this);
        break;
        
      default:
        break;
      }
    }
  }

  Thread *current_thread = Thread::get_current_thread();
  OPEN_ITERATE_CURRENT_AND_UPSTREAM(_cycler, current_thread) {
    CDStageWriter cdata(_cycler, pipeline_stage, current_thread);
    GeomList::iterator gi;
    PT(GeomList) geoms = cdata->modify_geoms();
    for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
      GeomEntry &entry = (*gi);
      PT(Geom) new_geom = entry._geom.get_read_pointer()->make_copy();
      
      AccumulatedAttribs geom_attribs = attribs;
      entry._state = geom_attribs.collect(entry._state, attrib_types);
      
      bool any_changed = false;
      
      if ((attrib_types & SceneGraphReducer::TT_color) != 0) {
        if (geom_attribs._color != (const RenderAttrib *)NULL) {
          const ColorAttrib *ca = DCAST(ColorAttrib, geom_attribs._color);
          if (ca->get_color_type() == ColorAttrib::T_flat) {
            if (transformer.set_color(new_geom, ca->get_color())) {
              any_changed = true;
            }
          }
        }
      }
      if ((attrib_types & SceneGraphReducer::TT_color_scale) != 0) {
        if (geom_attribs._color_scale != (const RenderAttrib *)NULL) {
          const ColorScaleAttrib *csa = DCAST(ColorScaleAttrib, geom_attribs._color_scale);
          if (csa->get_scale() != LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f)) {
            if (transformer.transform_colors(new_geom, csa->get_scale())) {
              any_changed = true;
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
            entry._state = entry._state->add_attrib(new_tma);
          }
        }
      }

      if (any_changed) {
        entry._geom = new_geom;
      }
      
      if ((attrib_types & SceneGraphReducer::TT_other) != 0) {
        entry._state = geom_attribs._other->compose(entry._state);
      }
    }
  }
  CLOSE_ITERATE_CURRENT_AND_UPSTREAM(_cycler);
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
xform(const LMatrix4f &mat) {
  GeomTransformer transformer;
  transformer.transform_vertices(this, mat);
}


////////////////////////////////////////////////////////////////////
//     Function: GeomNode::safe_to_flatten
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
bool GeomNode::
safe_to_flatten() const {
  if(_preserved)
    return false;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomNode::safe_to_combine
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
bool GeomNode::
safe_to_combine() const {
  if(_preserved)
    return false;
  return true;
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
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  CPT(TransformState) next_transform = 
    PandaNode::calc_tight_bounds(min_point, max_point, found_any, transform,
                                 current_thread);

  const LMatrix4f &mat = next_transform->get_mat();

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
//               that will be put in any one GeomPrimitive.
//
//               In order for this to be successful, the primitives
//               must reference the same GeomVertexData, have the same
//               fundamental primitive type, and have compatible shade
//               models.
////////////////////////////////////////////////////////////////////
void GeomNode::
unify(int max_indices) {
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
      GeomList::iterator gj;
      for (gj = new_geoms->begin(); gj != new_geoms->end() && !unified; ++gj) {
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
      geom->unify_in_place(max_indices);
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
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    int num_attribs = entry._state->get_num_attribs();
    for (int i = 0; i < num_attribs; i++) {
      const RenderAttrib *attrib = entry._state->get_attrib(i);
      attrib_types.insert(attrib->get_type());
    }
  }

  PandaNode::output(out);
  out << " (" << geoms->size() << " geoms";

  if (!attrib_types.empty()) {
    out << ":";
    pset<TypeHandle>::const_iterator ai;
    for (ai = attrib_types.begin(); ai != attrib_types.end(); ++ai) {
      out << " " << (*ai);
    }
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
//     Function: GeomNode::compute_internal_bounds
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated BoundingVolume that
//               represents the internal contents of the node.  Should
//               be overridden by PandaNode classes that contain
//               something internally.
////////////////////////////////////////////////////////////////////
void GeomNode::
compute_internal_bounds(PandaNode::BoundsData *bdata, int pipeline_stage, 
                        Thread *current_thread) const {
  int num_vertices = 0;

  CDLockedStageReader cdata(_cycler, pipeline_stage, current_thread);

  pvector<const BoundingVolume *> child_volumes;
  bool all_box = true;

  GeomList::const_iterator gi;
  CPT(GeomList) geoms = cdata->get_geoms();
  for (gi = geoms->begin(); gi != geoms->end(); ++gi) {
    const GeomEntry &entry = (*gi);
    CPT(Geom) geom = entry._geom.get_read_pointer();
    const BoundingVolume *volume = geom->get_bounds();

    if (!volume->is_empty()) {
      child_volumes.push_back(volume);
      if (!volume->is_exact_type(BoundingBox::get_class_type())) {
        all_box = false;
      }
    }
    num_vertices += geom->get_nested_vertices();
  }

  PT(GeometricBoundingVolume) gbv;

  if (bounds_type == BoundingVolume::BT_box ||
      (bounds_type != BoundingVolume::BT_sphere && all_box)) {
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
  
  bdata->_internal_bounds = gbv;
  bdata->_internal_vertices = num_vertices;
  bdata->_internal_bounds_stale = false;
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
