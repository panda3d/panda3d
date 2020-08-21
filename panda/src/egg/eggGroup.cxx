/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroup.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggGroup.h"
#include "eggMiscFuncs.h"
#include "eggVertexPool.h"
#include "eggBin.h"
#include "lexerDefs.h"

#include "indent.h"
#include "string_utils.h"
#include "lmatrix.h"
#include "dcast.h"

using std::ostream;
using std::string;


TypeHandle EggGroup::_type_handle;

/**
 *
 */
EggGroup::
EggGroup(const string &name) : EggGroupNode(name) {
  _flags = 0;
  _flags2 = 0;
  _fps = 0.0;
  _blend_mode = BM_unspecified;
  _blend_operand_a = BO_unspecified;
  _blend_operand_b = BO_unspecified;
  _blend_color = LColor::zero();
  _u_speed = 0;
  _v_speed = 0;
  _w_speed = 0;
  _r_speed = 0;
}

/**
 *
 */
EggGroup::
EggGroup(const EggGroup &copy) {
  (*this) = copy;
}

/**
 *
 */
EggGroup &EggGroup::
operator = (const EggGroup &copy) {
  EggTransform::operator = (copy);
  _flags = copy._flags;
  _flags2 = copy._flags2;
  _collide_mask = copy._collide_mask;
  _from_collide_mask = copy._from_collide_mask;
  _into_collide_mask = copy._into_collide_mask;
  _billboard_center = copy._billboard_center;
  _object_types = copy._object_types;
  _collision_name = copy._collision_name;
  _fps = copy._fps;
  _lod = copy._lod;
  _blend_mode = copy._blend_mode;
  _blend_operand_a = copy._blend_operand_a;
  _blend_operand_b = copy._blend_operand_b;
  _blend_color = copy._blend_color;
  _tag_data = copy._tag_data;
  _u_speed = copy._u_speed;
  _v_speed = copy._v_speed;
  _w_speed = copy._w_speed;
  _r_speed = copy._r_speed;
  _default_pose = copy._default_pose;

  unref_all_vertices();
  _vref = copy._vref;

  // We must walk through the vertex ref list, and flag each vertex as now
  // reffed by this group.
  VertexRef::iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;

    bool inserted = vert->_gref.insert(this).second;
    // Did the group not exist previously in the vertex's gref list?  If it
    // was there already, we must be out of sync between vertices and groups.
    nassertr(inserted, *this);
  }

  // These must be down here, because the EggNode assignment operator will
  // force an update_under().  Therefore, we can't call it until all the
  // attributes that affect adjust_under() are in place.
  EggGroupNode::operator = (copy);
  EggRenderMode::operator = (copy);

  return *this;
}


/**
 *
 */
EggGroup::
~EggGroup() {
  unref_all_vertices();
}

/**
 *
 */
void EggGroup::
set_group_type(GroupType type) {
  if (type != get_group_type()) {
#ifndef NDEBUG
    if (type != GT_instance) {
      // Only valid to change to a non-instance type if we have no group refs.
      nassertv(_group_refs.empty());
    }
#endif

    // Make sure the user didn't give us any stray bits.
    nassertv((type & ~F_group_type)==0);
    _flags = (_flags & ~F_group_type) | type;

    // Now we might have changed the type to or from an instance node, so we
    // have to recompute the under_flags.
    update_under(0);
  }
}

/**
 * Returns true if the indicated object type has been added to the group, or
 * false otherwise.
 */
bool EggGroup::
has_object_type(const string &object_type) const {
  vector_string::const_iterator oi;
  for (oi = _object_types.begin(); oi != _object_types.end(); ++oi) {
    if (cmp_nocase_uh((*oi), object_type) == 0) {
      return true;
    }
  }
  return false;
}

/**
 * Removes the first instance of the indicated object type from the group if
 * it is present.  Returns true if the object type was found and removed,
 * false otherwise.
 */
bool EggGroup::
remove_object_type(const string &object_type) {
  vector_string::iterator oi;
  for (oi = _object_types.begin(); oi != _object_types.end(); ++oi) {
    if (cmp_nocase_uh((*oi), object_type) == 0) {
      _object_types.erase(oi);
      return true;
    }
  }
  return false;
}

/**
 * Writes the group and all of its children to the indicated output stream in
 * Egg format.
 */
void EggGroup::
write(ostream &out, int indent_level) const {
  test_under_integrity();

  switch (get_group_type()) {
  case GT_group:
    write_header(out, indent_level, "<Group>");
    break;

  case GT_instance:
    write_header(out, indent_level, "<Instance>");
    break;

  case GT_joint:
    write_header(out, indent_level, "<Joint>");
    break;

  default:
    // invalid group type
    nassert_raise("invalid EggGroup type");
    return;
  }

  if (is_of_type(EggBin::get_class_type())) {
    indent(out, indent_level + 2)
      << "// Bin " << DCAST(EggBin, this)->get_bin_number() << "\n";
  }

  if (has_lod()) {
    get_lod().write(out, indent_level + 2);
  }

  write_billboard_flags(out, indent_level + 2);
  write_collide_flags(out, indent_level + 2);
  write_model_flags(out, indent_level + 2);
  write_switch_flags(out, indent_level + 2);

  if (has_transform()) {
    EggTransform::write(out, indent_level + 2, "<Transform>");
  }

  if (get_group_type() == GT_joint && _default_pose.has_transform()) {
    _default_pose.write(out, indent_level + 2, "<DefaultPose>");
  }

  if (get_scroll_u() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> scroll_u { " << get_scroll_u() << " }\n";

  }

  if (get_scroll_v() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> scroll_v { " << get_scroll_v() << " }\n";

  }

  if (get_scroll_w() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> scroll_w { " << get_scroll_w() << " }\n";

  }

  if (get_scroll_r() != 0) {
    indent(out, indent_level + 2)
      << "<Scalar> scroll_r { " << get_scroll_r() << " }\n";

  }

  write_object_types(out, indent_level + 2);
  write_decal_flags(out, indent_level + 2);
  write_tags(out, indent_level + 2);
  write_render_mode(out, indent_level + 2);

  if (get_portal_flag()) {
    indent(out, indent_level + 2) << "<Scalar> portal { 1 }\n";
  }

  if (get_occluder_flag()) {
    indent(out, indent_level + 2) << "<Scalar> occluder { 1 }\n";
  }

  if (get_polylight_flag()) {
    indent(out, indent_level + 2) << "<Scalar> polylight { 1 }\n";
  }

  if (has_indexed_flag()) {
    indent(out, indent_level + 2)
      << "<Scalar> indexed { " << get_indexed_flag() << " }\n";
  }

  if (get_blend_mode() != BM_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> blend { " << get_blend_mode() << " }\n";
  }

  if (get_blend_operand_a() != BO_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> blendop-a { " << get_blend_operand_a() << " }\n";
  }

  if (get_blend_operand_b() != BO_unspecified) {
    indent(out, indent_level + 2)
      << "<Scalar> blendop-b { " << get_blend_operand_b() << " }\n";
  }

  if (has_blend_color()) {
    const LColor &c = get_blend_color();
    indent(out, indent_level + 2)
      << "<Scalar> blendr { " << c[0] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blendg { " << c[1] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blendb { " << c[2] << " }\n";
    indent(out, indent_level + 2)
      << "<Scalar> blenda { " << c[3] << " }\n";
  }

  GroupRefs::const_iterator gri;
  for (gri = _group_refs.begin(); gri != _group_refs.end(); ++gri) {
    EggGroup *group_ref = (*gri);
    indent(out, indent_level + 2)
      << "<Ref> { " << group_ref->get_name() << " }\n";
  }

  // We have to write the children nodes before we write the vertex
  // references, since we might be referencing a vertex that's defined in one
  // of those children nodes!
  EggGroupNode::write(out, indent_level + 2);
  write_vertex_ref(out, indent_level + 2);

  indent(out, indent_level) << "}\n";
}

/**
 * Writes just the <Billboard> entry and related fields to the indicated
 * ostream.
 */
void EggGroup::
write_billboard_flags(ostream &out, int indent_level) const {
  if (get_billboard_type() != BT_none) {
    indent(out, indent_level)
      << "<Billboard> { " << get_billboard_type() << " }\n";
  }

  if (has_billboard_center()) {
    indent(out, indent_level)
      << "<BillboardCenter> { " << get_billboard_center() << " }\n";
  }
}

/**
 * Writes just the <Collide> entry and related fields to the indicated
 * ostream.
 */
void EggGroup::
write_collide_flags(ostream &out, int indent_level) const {
  if (get_cs_type() != CST_none) {
    indent(out, indent_level) << "<Collide> ";
    if (has_collision_name()) {
      enquote_string(out, get_collision_name()) << " ";
    }
    out << "{ " << get_cs_type();
    if (get_collide_flags() != CF_none) {
      out << " " << get_collide_flags();
    }
    out << " }\n";
  }

  if (has_collide_mask()) {
    indent(out, indent_level)
      << "<Scalar> collide-mask { 0x";
    get_collide_mask().output_hex(out, 0);
    out << " }\n";
  }

  if (has_from_collide_mask()) {
    indent(out, indent_level)
      << "<Scalar> from-collide-mask { 0x";
    get_from_collide_mask().output_hex(out, 0);
    out << " }\n";
  }

  if (has_into_collide_mask()) {
    indent(out, indent_level)
      << "<Scalar> into-collide-mask { 0x";
    get_into_collide_mask().output_hex(out, 0);
    out << " }\n";
  }
}

/**
 * Writes the <Model> flag and related flags to the indicated ostream.
 */
void EggGroup::
write_model_flags(ostream &out, int indent_level) const {
  if (get_dcs_type() != DC_unspecified) {
    indent(out, indent_level)
      << "<DCS> { " << get_dcs_type() << " }\n";
  }

  if (get_dart_type() != DT_none) {
    indent(out, indent_level)
      << "<Dart> { " << get_dart_type() << " }\n";
  }

  if (get_model_flag()) {
    indent(out, indent_level) << "<Model> { 1 }\n";
  }

  if (get_texlist_flag()) {
    indent(out, indent_level) << "<TexList> { 1 }\n";
  }

  if (get_direct_flag()) {
    indent(out, indent_level) << "<Scalar> direct { 1 }\n";
  }
}

/**
 * Writes the <Switch> flag and related flags to the indicated ostream.
 */
void EggGroup::
write_switch_flags(ostream &out, int indent_level) const {
  if (get_switch_flag()) {
    indent(out, indent_level) << "<Switch> { 1 }\n";
    if (get_switch_fps() != 0.0) {
      indent(out, indent_level)
        << "<Scalar> fps { " << get_switch_fps() << " }\n";
    }
  }
}

/**
 * Writes just the <ObjectTypes> entries, if any, to the indicated ostream.
 */
void EggGroup::
write_object_types(ostream &out, int indent_level) const {
  vector_string::const_iterator oi;
  for (oi = _object_types.begin(); oi != _object_types.end(); ++oi) {
    indent(out, indent_level)
      << "<ObjectType> { ";
    enquote_string(out, (*oi)) << " }\n";
  }
}

/**
 * Writes the flags related to decaling, if any.
 */
void EggGroup::
write_decal_flags(ostream &out, int indent_level) const {
  if (get_decal_flag()) {
    indent(out, indent_level) << "<Scalar> decal { 1 }\n";
  }
}

/**
 * Writes just the <Tag> entries, if any, to the indicated ostream.
 */
void EggGroup::
write_tags(ostream &out, int indent_level) const {
  TagData::const_iterator ti;
  for (ti = _tag_data.begin(); ti != _tag_data.end(); ++ti) {
    const string &key = (*ti).first;
    const string &value = (*ti).second;

    indent(out, indent_level) << "<Tag> ";
    enquote_string(out, key) << " {\n";
    enquote_string(out, value, indent_level + 2) << "\n";
    indent(out, indent_level) << "}\n";
  }
}

/**
 * Writes the flags inherited from EggRenderMode and similar flags that
 * control obscure render effects.
 */
void EggGroup::
write_render_mode(ostream &out, int indent_level) const {
  EggRenderMode::write(out, indent_level);

  if (get_nofog_flag()) {
    indent(out, indent_level) << "<Scalar> no-fog { 1 }\n";
  }
}

/**
 * Returns true if this particular node represents a <Joint> entry or not.
 * This is a handy thing to know since Joints are sorted to the end of their
 * sibling list when writing an egg file.  See EggGroupNode::write().
 */
bool EggGroup::
is_joint() const {
  return (get_group_type() == GT_joint);
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has an alpha_mode
 * other than AM_unspecified.  Returns a valid EggRenderMode pointer if one is
 * found, or NULL otherwise.
 */
EggRenderMode *EggGroup::
determine_alpha_mode() {
  if (get_alpha_mode() != AM_unspecified) {
    return this;
  }
  return EggGroupNode::determine_alpha_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a
 * depth_write_mode other than DWM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggGroup::
determine_depth_write_mode() {
  if (get_depth_write_mode() != DWM_unspecified) {
    return this;
  }
  return EggGroupNode::determine_depth_write_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a
 * depth_test_mode other than DTM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggGroup::
determine_depth_test_mode() {
  if (get_depth_test_mode() != DTM_unspecified) {
    return this;
  }
  return EggGroupNode::determine_depth_test_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a
 * visibility_mode other than VM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggGroup::
determine_visibility_mode() {
  if (get_visibility_mode() != VM_unspecified) {
    return this;
  }
  return EggGroupNode::determine_visibility_mode();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a depth_offset
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggGroup::
determine_depth_offset() {
  if (has_depth_offset()) {
    return this;
  }
  return EggGroupNode::determine_depth_offset();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a draw_order
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggGroup::
determine_draw_order() {
  if (has_draw_order()) {
    return this;
  }
  return EggGroupNode::determine_draw_order();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this group that has a bin
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggGroup::
determine_bin() {
  if (has_bin()) {
    return this;
  }
  return EggGroupNode::determine_bin();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup at this level or above
 * that has the "indexed" scalar set.  Returns the value of the indexed scalar
 * if it is found, or false if it is not.
 *
 * In other words, returns true if the "indexed" flag is in effect for the
 * indicated node, false otherwise.
 */
bool EggGroup::
determine_indexed() {
  if (has_indexed_flag()) {
    return get_indexed_flag();
  }
  return EggGroupNode::determine_indexed();
}

/**
 * Walks back up the hierarchy, looking for an EggGroup at this level or above
 * that has the "decal" flag set.  Returns the value of the decal flag if it
 * is found, or false if it is not.
 *
 * In other words, returns true if the "decal" flag is in effect for the
 * indicated node, false otherwise.
 */
bool EggGroup::
determine_decal() {
  if (get_decal_flag()) {
    return true;
  }
  return EggGroupNode::determine_decal();
}

/**
 * Adds the vertex to the set of those referenced by the group, at the
 * indicated membership level.  If the vertex is already being referenced,
 * increases the membership amount by the indicated amount.
 */
void EggGroup::
ref_vertex(EggVertex *vert, double membership) {
  VertexRef::iterator vri = _vref.find(vert);

  if (vri != _vref.end()) {
    // The vertex was already being reffed; increment its membership amount.
    (*vri).second += membership;

    // If that takes us down to zero, go ahead and unref the vertex.
    if ((*vri).second == 0.0) {
      unref_vertex(vert);
    }

  } else {
    // The vertex was not already reffed; ref it.
    if (membership != 0.0) {
      _vref[vert] = membership;

      bool inserted = vert->_gref.insert(this).second;
      // Did the group not exist previously in the vertex's gref list?  If it
      // was there already, we must be out of sync between vertices and
      // groups.
      nassertv(inserted);
    }
  }
}


/**
 * Removes the vertex from the set of those referenced by the group.  Does
 * nothing if the vertex is not already reffed.
 */
void EggGroup::
unref_vertex(EggVertex *vert) {
  VertexRef::iterator vri = _vref.find(vert);

  if (vri != _vref.end()) {
    _vref.erase(vri);
    int count = vert->_gref.erase(this);
    // Did the group exist in the vertex's gref list?  If it didn't, we must
    // be out of sync between vertices and groups.
    nassertv(count == 1);
  }
}

/**
 * Removes all vertices from the reference list.
 */
void EggGroup::
unref_all_vertices() {
  // We must walk through the vertex ref list, and flag each vertex as
  // unreffed in its own structure.
  VertexRef::iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;
    int count = vert->_gref.erase(this);
    // Did the group exist in the vertex's gref list?  If it didn't, we must
    // be out of sync between vertices and groups.
    nassertv(count == 1);
  }

  _vref.clear();
}


/**
 * Returns the amount of membership of the indicated vertex in this group.  If
 * the vertex is not reffed by the group, returns 0.
 */
double EggGroup::
get_vertex_membership(const EggVertex *vert) const {
  VertexRef::const_iterator vri = _vref.find((EggVertex *)vert);

  if (vri != _vref.end()) {
    return (*vri).second;
  } else {
    return 0.0;
  }
}

/**
 * Explicitly sets the net membership of the indicated vertex in this group to
 * the given value.
 */
void EggGroup::
set_vertex_membership(EggVertex *vert, double membership) {
  if (membership == 0.0) {
    unref_vertex(vert);
    return;
  }

  VertexRef::iterator vri = _vref.find(vert);

  if (vri != _vref.end()) {
    // The vertex was already being reffed; just change its membership amount.
    (*vri).second = membership;

  } else {
    // The vertex was not already reffed; ref it.
    _vref[vert] = membership;

    bool inserted = vert->_gref.insert(this).second;
    // Did the group not exist previously in the vertex's gref list?  If it
    // was there already, we must be out of sync between vertices and groups.
    nassertv(inserted);
  }
}

/**
 * Moves all of the vertex references from the indicated other group into this
 * one.  If a given vertex was previously shared by both groups, the relative
 * memberships will be summed.
 */
void EggGroup::
steal_vrefs(EggGroup *other) {
  nassertv(other != this);
  VertexRef::const_iterator vri;
  for (vri = other->vref_begin(); vri != other->vref_end(); ++vri) {
    EggVertex *vert = (*vri).first;
    double membership = (*vri).second;
    ref_vertex(vert, membership);
  }
  other->unref_all_vertices();
}


#ifdef _DEBUG

/**
 * Verifies that each vertex in the group exists and that it knows it is
 * referenced by the group.
 */
void EggGroup::
test_vref_integrity() const {
  test_ref_count_integrity();

  VertexRef::const_iterator vri;
  for (vri = vref_begin(); vri != vref_end(); ++vri) {
    const EggVertex *vert = (*vri).first;
    vert->test_ref_count_integrity();

    nassertv(vert->has_gref(this));
  }
}

#endif  // _DEBUG

/**
 * Adds a new <Ref> entry to the group.  This declares an internal reference
 * to another node, and is used to implement scene-graph instancing; it is
 * only valid if the group_type is GT_instance.
 */
void EggGroup::
add_group_ref(EggGroup *group) {
  nassertv(get_group_type() == GT_instance);
  _group_refs.push_back(group);
}

/**
 * Returns the number of <Ref> entries within this group.  See
 * add_group_ref().
 */
int EggGroup::
get_num_group_refs() const {
  return _group_refs.size();
}

/**
 * Returns the nth <Ref> entry within this group.  See add_group_ref().
 */
EggGroup *EggGroup::
get_group_ref(int n) const {
  nassertr(n >= 0 && n < (int)_group_refs.size(), nullptr);
  return _group_refs[n];
}

/**
 * Removes the nth <Ref> entry within this group.  See add_group_ref().
 */
void EggGroup::
remove_group_ref(int n) {
  nassertv(n >= 0 && n < (int)_group_refs.size());
  _group_refs.erase(_group_refs.begin() + n);
}

/**
 * Removes all of the <Ref> entries within this group.  See add_group_ref().
 */
void EggGroup::
clear_group_refs() {
  _group_refs.clear();
}



/**
 * Returns the GroupType value associated with the given string
 * representation, or GT_invalid if the string does not match any known
 * GroupType value.
 */
EggGroup::GroupType EggGroup::
string_group_type(const string &strval) {
  if (cmp_nocase_uh(strval, "group") == 0) {
    return GT_group;
  } else if (cmp_nocase_uh(strval, "instance") == 0) {
    return GT_instance;
  } else if (cmp_nocase_uh(strval, "joint") == 0) {
    return GT_joint;
  } else {
    return GT_invalid;
  }
}

/**
 * Returns the DartType value associated with the given string representation,
 * or DT_none if the string does not match any known DartType value.
 */
EggGroup::DartType EggGroup::
string_dart_type(const string &strval) {
  if (cmp_nocase_uh(strval, "sync") == 0) {
    return DT_sync;
  } else if (cmp_nocase_uh(strval, "nosync") == 0) {
    return DT_nosync;
  } else if (cmp_nocase_uh(strval, "default") == 0) {
    return DT_default;
  } else if (cmp_nocase_uh(strval, "structured") == 0) {
    return DT_structured;
  } else {
    return DT_none;
  }
}

/**
 * Returns the DCSType value associated with the given string representation,
 * or DC_unspecified if the string does not match any known DCSType value.
 */
EggGroup::DCSType EggGroup::
string_dcs_type(const string &strval) {
  if (cmp_nocase_uh(strval, "none") == 0) {
    return DC_none;
  } else if (cmp_nocase_uh(strval, "local") == 0) {
    return DC_local;
  } else if (cmp_nocase_uh(strval, "net") == 0) {
    return DC_net;
  } else if (cmp_nocase_uh(strval, "no_touch") == 0) {
    return DC_no_touch;
  } else if (cmp_nocase_uh(strval, "default") == 0) {
    return DC_default;
  } else {
    return DC_unspecified;
  }
}

/**
 * Returns the BillboardType value associated with the given string
 * representation, or BT_none if the string does not match any known
 * BillboardType value.
 */
EggGroup::BillboardType EggGroup::
string_billboard_type(const string &strval) {
  if (cmp_nocase_uh(strval, "axis") == 0) {
    return BT_axis;
  } else if (cmp_nocase_uh(strval, "point_eye") == 0) {
    return BT_point_camera_relative;
  } else if (cmp_nocase_uh(strval, "point_world") == 0) {
    return BT_point_world_relative;
  } else if (cmp_nocase_uh(strval, "point") == 0) {
    return BT_point_world_relative;
  } else {
    return BT_none;
  }
}

/**
 * Returns the CollisionSolidType value associated with the given string
 * representation, or CST_none if the string does not match any known
 * CollisionSolidType value.
 */
EggGroup::CollisionSolidType EggGroup::
string_cs_type(const string &strval) {
  if (cmp_nocase_uh(strval, "plane") == 0) {
    return CST_plane;
  } else if (cmp_nocase_uh(strval, "polygon") == 0) {
    return CST_polygon;
  } else if (cmp_nocase_uh(strval, "polyset") == 0) {
    return CST_polyset;
  } else if (cmp_nocase_uh(strval, "sphere") == 0) {
    return CST_sphere;
  } else if (cmp_nocase_uh(strval, "box") == 0) {
    return CST_box;
  } else if (cmp_nocase_uh(strval, "inv-sphere") == 0 ||
             cmp_nocase_uh(strval, "invsphere") == 0) {
    return CST_inv_sphere;
  } else if (cmp_nocase_uh(strval, "tube") == 0 ||
             cmp_nocase_uh(strval, "capsule") == 0) {
    return CST_tube;
  } else if (cmp_nocase_uh(strval, "floor-mesh") == 0 ||
             cmp_nocase_uh(strval, "floormesh") == 0) {
    return CST_floor_mesh;
  } else {
    return CST_none;
  }
}

/**
 * Returns the CollideFlags value associated with the given string
 * representation, or CF_none if the string does not match any known
 * CollideFlags value.  This only recognizes a single keyword; it does not
 * attempt to parse a string of keywords.
 */
EggGroup::CollideFlags EggGroup::
string_collide_flags(const string &strval) {
  if (cmp_nocase_uh(strval, "intangible") == 0) {
    return CF_intangible;
  } else if (cmp_nocase_uh(strval, "event") == 0) {
    return CF_event;
  } else if (cmp_nocase_uh(strval, "descend") == 0) {
    return CF_descend;
  } else if (cmp_nocase_uh(strval, "keep") == 0) {
    return CF_keep;
  } else if (cmp_nocase_uh(strval, "solid") == 0) {
    return CF_solid;
  } else if (cmp_nocase_uh(strval, "center") == 0) {
    return CF_center;
  } else if (cmp_nocase_uh(strval, "turnstile") == 0) {
    return CF_turnstile;
  } else if (cmp_nocase_uh(strval, "level") == 0) {
    return CF_level;
  } else {
    return CF_none;
  }
}

/**
 * Returns the BlendMode value associated with the given string
 * representation, or BM_none if the string does not match any known
 * BlendMode.
 */
EggGroup::BlendMode EggGroup::
string_blend_mode(const string &strval) {
  if (cmp_nocase_uh(strval, "none") == 0) {
    return BM_none;
  } else if (cmp_nocase_uh(strval, "add") == 0) {
    return BM_add;
  } else if (cmp_nocase_uh(strval, "subtract") == 0) {
    return BM_subtract;
  } else if (cmp_nocase_uh(strval, "inv_subtract") == 0) {
    return BM_inv_subtract;
  } else if (cmp_nocase_uh(strval, "min") == 0) {
    return BM_min;
  } else if (cmp_nocase_uh(strval, "max") == 0) {
    return BM_max;
  } else {
    return BM_unspecified;
  }
}

/**
 * Returns the BlendOperand value associated with the given string
 * representation, or BO_none if the string does not match any known
 * BlendOperand.
 */
EggGroup::BlendOperand EggGroup::
string_blend_operand(const string &strval) {
  if (cmp_nocase_uh(strval, "zero") == 0) {
    return BO_zero;
  } else if (cmp_nocase_uh(strval, "one") == 0) {
    return BO_one;
  } else if (cmp_nocase_uh(strval, "incoming_color") == 0) {
    return BO_incoming_color;
  } else if (cmp_nocase_uh(strval, "one_minus_incoming_color") == 0) {
    return BO_one_minus_incoming_color;
  } else if (cmp_nocase_uh(strval, "fbuffer_color") == 0) {
    return BO_fbuffer_color;
  } else if (cmp_nocase_uh(strval, "one_minus_fbuffer_color") == 0) {
    return BO_one_minus_fbuffer_color;
  } else if (cmp_nocase_uh(strval, "incoming_alpha") == 0) {
    return BO_incoming_alpha;
  } else if (cmp_nocase_uh(strval, "one_minus_incoming_alpha") == 0) {
    return BO_one_minus_incoming_alpha;
  } else if (cmp_nocase_uh(strval, "fbuffer_alpha") == 0) {
    return BO_fbuffer_alpha;
  } else if (cmp_nocase_uh(strval, "one_minus_fbuffer_alpha") == 0) {
    return BO_one_minus_fbuffer_alpha;
  } else if (cmp_nocase_uh(strval, "constant_color") == 0) {
    return BO_constant_color;
  } else if (cmp_nocase_uh(strval, "one_minus_constant_color") == 0) {
    return BO_one_minus_constant_color;
  } else if (cmp_nocase_uh(strval, "constant_alpha") == 0) {
    return BO_constant_alpha;
  } else if (cmp_nocase_uh(strval, "one_minus_constant_alpha") == 0) {
    return BO_one_minus_constant_alpha;
  } else if (cmp_nocase_uh(strval, "incoming_color_saturate") == 0) {
    return BO_incoming_color_saturate;
  } else if (cmp_nocase_uh(strval, "color_scale") == 0) {
    return BO_color_scale;
  } else if (cmp_nocase_uh(strval, "one_minus_color_scale") == 0) {
    return BO_one_minus_color_scale;
  } else if (cmp_nocase_uh(strval, "alpha_scale") == 0) {
    return BO_alpha_scale;
  } else if (cmp_nocase_uh(strval, "one_minus_alpha_scale") == 0) {
    return BO_one_minus_alpha_scale;
  } else {
    return BO_unspecified;
  }
}

/**
 * Returns this object cross-cast to an EggTransform pointer, if it inherits
 * from EggTransform, or NULL if it does not.
 */
EggTransform *EggGroup::
as_transform() {
  return this;
}


/**
 * Writes out the vertex ref component of the group body only.  This may
 * consist of a number of <VertexRef> entries, each with its own membership
 * value.
 */
void EggGroup::
write_vertex_ref(ostream &out, int indent_level) const {
  // We want to put the vertices together into groups first by vertex pool,
  // then by membership value.  Each of these groups becomes a separate
  // VertexRef entry.  Within each group, we'll sort the vertices by index
  // number.

  typedef pset<int> Indices;
  typedef pmap<double, Indices> Memberships;
  typedef pmap<EggVertexPool *, Memberships> Pools;

  Pools _entries;
  bool all_membership_one = true;

  VertexRef::const_iterator vri;
  for (vri = _vref.begin(); vri != _vref.end(); ++vri) {
    EggVertex *vert = (*vri).first;
    double membership = (*vri).second;

    if (membership != 1.0) {
      all_membership_one = false;
    }

    _entries[vert->get_pool()][membership].insert(vert->get_index());
  }

  // Now that we've reordered them, we can simply traverse the entries and
  // write them out.
  Pools::const_iterator pi;
  for (pi = _entries.begin(); pi != _entries.end(); ++pi) {
    EggVertexPool *pool = (*pi).first;
    const Memberships &memberships = (*pi).second;
    Memberships::const_iterator mi;
    for (mi = memberships.begin(); mi != memberships.end(); ++mi) {
      double membership = (*mi).first;
      const Indices &indices = (*mi).second;

      indent(out, indent_level)
        << "<VertexRef> {\n";
      write_long_list(out, indent_level+2, indices.begin(), indices.end(),
                      "", "", 72);

      // If all vrefs in this group have membership of 1, don't bother to
      // write out the membership scalar.
      if (!all_membership_one) {
        indent(out, indent_level + 2)
          << "<Scalar> membership { " << membership << " }\n";
      }
      if (pool == nullptr) {
        indent(out, indent_level + 2)
          << "// Invalid NULL vertex pool.\n";
      } else {
        indent(out, indent_level + 2)
          << "<Ref> { " << pool->get_name() << " }\n";
      }
      indent(out, indent_level)
        << "}\n";
    }
  }
}

/**
 * This function is called within parse_egg().  It should call the appropriate
 * function on the lexer to initialize the parser into the state associated
 * with this object.  If the object cannot be parsed into directly, it should
 * return false.
 */
bool EggGroup::
egg_start_parse_body() {
  egg_start_group_body();
  return true;
}

/**
 * This is called within update_under() after all the various under settings
 * have been inherited directly from the parent node.  It is responsible for
 * adjusting these settings to reflect states local to the current node; for
 * instance, an <Instance> node will force the UF_under_instance bit on.
 */
void EggGroup::
adjust_under() {
  // If we have our own transform, it carries forward.

  // As of 41801, this now also affects the local_coord flag, below.  This
  // means that a <Transform> entry within an <Instance> node transforms the
  // instance itself.
  if (has_transform()) {
    _under_flags |= UF_under_transform;

    // Our own transform also affects our node frame.
    _node_frame =
      new MatrixFrame(get_transform3d() * get_node_frame());

    // To prevent trying to invert a sigular matrix
    LMatrix4d mat;
    bool invert_ok = mat.invert_from(get_node_frame());
    if (invert_ok) {
      _node_frame_inv =
        new MatrixFrame(mat);
    } else {
      _node_frame_inv = nullptr;
    }

    _vertex_to_node =
      new MatrixFrame(get_vertex_frame() * get_node_frame_inv());
    _node_to_vertex =
      new MatrixFrame(get_node_frame() * get_vertex_frame_inv());

  }

  if (is_instance_type()) {
    _under_flags |= UF_under_instance;
    if (_under_flags & UF_under_transform) {
      // If we've reached an instance node and we're under a transform, that
      // means we've just defined a local coordinate system.
      _under_flags |= UF_local_coord;
    }

    // An instance node means that from this point and below, vertices are
    // defined relative to this node.  Thus, the node frame becomes the vertex
    // frame.
    _vertex_frame = _node_frame;
    _vertex_frame_inv = _node_frame_inv;
    _vertex_to_node = nullptr;
    _node_to_vertex = nullptr;
  }
}

/**
 * This is called from within the egg code by transform().  It applies a
 * transformation matrix to the current node in some sensible way, then
 * continues down the tree.
 *
 * The first matrix is the transformation to apply; the second is its inverse.
 * The third parameter is the coordinate system we are changing to, or
 * CS_default if we are not changing coordinate systems.
 */
void EggGroup::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
            CoordinateSystem to_cs) {
  if (has_transform() || get_group_type() == GT_joint) {
    // Since we want to apply this transform to all matrices, including nested
    // matrices, we can't simply premult it in and leave it, because that
    // would leave the rotational component in the scene graph's matrix, and
    // all nested matrices would inherit the same rotational component.  So we
    // have to premult and then postmult by the inverse to undo the rotational
    // component each time.

    LMatrix4d mat1 = mat;
    LMatrix4d inv1 = inv;

    // If we have a translation component, we should only apply it to the top
    // matrix.  All subsequent matrices get just the rotational component.
    mat1.set_row(3, LVector3d(0.0, 0.0, 0.0));
    inv1.set_row(3, LVector3d(0.0, 0.0, 0.0));

    internal_set_transform(inv1 * get_transform3d() * mat);

    if (_default_pose.has_transform()) {
      LMatrix4d t = _default_pose.get_transform3d();
      _default_pose.clear_transform();
      _default_pose.add_matrix4(inv1 * t * mat);
    }

    EggGroupNode::r_transform(mat1, inv1, to_cs);
  } else {
    EggGroupNode::r_transform(mat, inv, to_cs);
  }

  // Convert the LOD description too.
  if (has_lod()) {
    _lod->transform(mat);
  }
  if (has_billboard_center()) {
    _billboard_center = _billboard_center * mat;
  }
}

/**
 * The recursive implementation of flatten_transforms().
 */
void EggGroup::
r_flatten_transforms() {
  EggGroupNode::r_flatten_transforms();

  if (is_local_coord()) {
    LMatrix4d mat = get_vertex_frame();
    if (has_lod()) {
      _lod->transform(mat);
    }

    if (get_billboard_type() != BT_none && !has_billboard_center()) {
      // If we had a billboard without an explicit center, it was an implicit
      // instance.  Now it's not any more.
      set_billboard_center(LPoint3d(0.0, 0.0, 0.0) * mat);

    } else if (has_billboard_center()) {
      _billboard_center = _billboard_center * mat;
    }
  }

  if (get_group_type() == GT_instance) {
    set_group_type(GT_group);
  }

  if (get_group_type() != GT_joint) {
    internal_clear_transform();
  }
}


/**
 * This virtual method is inherited by EggTransform3d; it is called whenever
 * the transform is changed.
 */
void EggGroup::
transform_changed() {
  // Recompute all of the cached transforms at this node and below.  We should
  // probably make this smarter and do lazy evaluation of these transforms,
  // rather than having to recompute the whole tree with every change to a
  // parent node's transform.
  update_under(0);
}



/**
 *
 */
ostream &operator << (ostream &out, EggGroup::GroupType t) {
  switch (t) {
  case EggGroup::GT_invalid:
    return out << "invalid group";
  case EggGroup::GT_group:
    return out << "group";
  case EggGroup::GT_instance:
    return out << "instance";
  case EggGroup::GT_joint:
    return out << "joint";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggGroup::DartType t) {
  switch (t) {
  case EggGroup::DT_none:
    return out << "none";
  case EggGroup::DT_sync:
    return out << "sync";
  case EggGroup::DT_nosync:
    return out << "nosync";
  case EggGroup::DT_structured:
    return out << "structured";
  case EggGroup::DT_default:
    return out << "1";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggGroup::DCSType t) {
  switch (t) {
  case EggGroup::DC_unspecified:
    return out << "unspecified";
  case EggGroup::DC_none:
    return out << "none";
  case EggGroup::DC_local:
    return out << "local";
  case EggGroup::DC_net:
    return out << "net";
  case EggGroup::DC_no_touch:
    return out << "no_touch";
  case EggGroup::DC_default:
    return out << "1";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggGroup::BillboardType t) {
  switch (t) {
  case EggGroup::BT_none:
    return out << "none";
  case EggGroup::BT_axis:
    return out << "axis";
  case EggGroup::BT_point_camera_relative:
    return out << "point_eye";
  case EggGroup::BT_point_world_relative:
    return out << "point_world";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggGroup::CollisionSolidType t) {
  switch (t) {
  case EggGroup::CST_none:
    return out << "None";
  case EggGroup::CST_plane:
    return out << "Plane";
  case EggGroup::CST_polygon:
    return out << "Polygon";
  case EggGroup::CST_polyset:
    return out << "Polyset";
  case EggGroup::CST_sphere:
    return out << "Sphere";
  case EggGroup::CST_inv_sphere:
    return out << "InvSphere";
  case EggGroup::CST_tube:
    return out << "Tube";
  case EggGroup::CST_floor_mesh:
    return out << "FloorMesh";
  case EggGroup::CST_box:
    return out << "Box";
  }

  nassertr(false, out);
  return out << "(**invalid**)";
}

/**
 *
 */
ostream &operator << (ostream &out, EggGroup::CollideFlags t) {
  if (t == EggGroup::CF_none) {
    return out << "none";
  }
  int bits = (int)t;
  const char *space = "";

  if (bits & EggGroup::CF_intangible) {
    out << space << "intangible";
    space = " ";
  }
  if (bits & EggGroup::CF_event) {
    out << space << "event";
    space = " ";
  }
  if (bits & EggGroup::CF_descend) {
    out << space << "descend";
    space = " ";
  }
  if (bits & EggGroup::CF_keep) {
    out << space << "keep";
    space = " ";
  }
  if (bits & EggGroup::CF_solid) {
    out << space << "solid";
    space = " ";
  }
  if (bits & EggGroup::CF_center) {
    out << space << "center";
    space = " ";
  }
  if (bits & EggGroup::CF_turnstile) {
    out << space << "turnstile";
    space = " ";
  }
  if (bits & EggGroup::CF_level) {
    out << space << "level";
    space = " ";
  }
  return out;
}

/**
 *
 */
ostream &
operator << (ostream &out, EggGroup::BlendMode t) {
  switch (t) {
  case EggGroup::BM_unspecified:
    return out << "unspecified";

  case EggGroup::BM_none:
    return out << "none";

  case EggGroup::BM_add:
    return out << "add";

  case EggGroup::BM_subtract:
    return out << "subtract";

  case EggGroup::BM_inv_subtract:
    return out << "inv_subtract";

  case EggGroup::BM_min:
    return out << "min";

  case EggGroup::BM_max:
    return out << "max";
  }

  return out << "**invalid EggGroup::BlendMode(" << (int)t << ")**";
}

/**
 *
 */
ostream &
operator << (ostream &out, EggGroup::BlendOperand t) {
  switch (t) {
  case EggGroup::BO_unspecified:
    return out << "unspecified";

  case EggGroup::BO_zero:
    return out << "zero";

  case EggGroup::BO_one:
    return out << "one";

  case EggGroup::BO_incoming_color:
    return out << "incomfing_color";

  case EggGroup::BO_one_minus_incoming_color:
    return out << "one_minus_incoming_color";

  case EggGroup::BO_fbuffer_color:
    return out << "fbuffer_color";

  case EggGroup::BO_one_minus_fbuffer_color:
    return out << "one_minus_fbuffer_color";

  case EggGroup::BO_incoming_alpha:
    return out << "incoming_alpha";

  case EggGroup::BO_one_minus_incoming_alpha:
    return out << "one_minus_incoming_alpha";

  case EggGroup::BO_fbuffer_alpha:
    return out << "fbuffer_alpha";

  case EggGroup::BO_one_minus_fbuffer_alpha:
    return out << "one_minus_fbuffer_alpha";

  case EggGroup::BO_constant_color:
    return out << "constant_color";

  case EggGroup::BO_one_minus_constant_color:
    return out << "one_minus_constant_color";

  case EggGroup::BO_constant_alpha:
    return out << "constant_alpha";

  case EggGroup::BO_one_minus_constant_alpha:
    return out << "one_minus_constant_alpha";

  case EggGroup::BO_incoming_color_saturate:
    return out << "incoming_color_saturate";

  case EggGroup::BO_color_scale:
    return out << "color_scale";

  case EggGroup::BO_one_minus_color_scale:
    return out << "one_minus_color_scale";

  case EggGroup::BO_alpha_scale:
    return out << "alpha_scale";

  case EggGroup::BO_one_minus_alpha_scale:
    return out << "one_minus_alpha_scale";
  }

  return out << "**invalid EggGroup::BlendOperand(" << (int)t << ")**";
}
