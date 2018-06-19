/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltToEggConverter.cxx
 * @author drose
 * @date 2001-04-17
 */

#include "fltToEggConverter.h"

#include "fltRecord.h"
#include "fltLOD.h"
#include "fltGroup.h"
#include "fltObject.h"
#include "fltBeadID.h"
#include "fltBead.h"
#include "fltFace.h"
#include "fltVertex.h"
#include "fltVertexList.h"
#include "fltExternalReference.h"
#include "dcast.h"
#include "eggData.h"
#include "eggGroup.h"
#include "eggSwitchCondition.h"
#include "eggPrimitive.h"
#include "eggPolygon.h"
#include "eggPoint.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggExternalReference.h"
#include "string_utils.h"

using std::string;


/**
 *
 */
FltToEggConverter::
FltToEggConverter() {
  _compose_transforms = false;
  _flt_units = DU_invalid;
}

/**
 *
 */
FltToEggConverter::
FltToEggConverter(const FltToEggConverter &copy) :
  SomethingToEggConverter(copy),
  _compose_transforms(copy._compose_transforms)
{
}

/**
 *
 */
FltToEggConverter::
~FltToEggConverter() {
  cleanup();
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *FltToEggConverter::
make_copy() {
  return new FltToEggConverter(*this);
}


/**
 * Returns the English name of the file type this converter supports.
 */
string FltToEggConverter::
get_name() const {
  return "MultiGen";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
string FltToEggConverter::
get_extension() const {
  return "flt";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz extension), false otherwise.
 */
bool FltToEggConverter::
supports_compressed() const {
  return true;
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 *
 * This is designed to be as generic as possible, generally in support of run-
 * time loading.  Command-line converters may choose to use convert_flt()
 * instead, as it provides more control.
 */
bool FltToEggConverter::
convert_file(const Filename &filename) {
  PT(FltHeader) header = new FltHeader(_path_replace);

  nout << "Reading " << filename << "\n";
  FltError result = header->read_flt(filename);
  if (result != FE_ok) {
    nout << "Unable to read: " << result << "\n";
    return false;
  }

  header->check_version();

  _flt_units = header->get_units();

  return convert_flt(header);
}

/**
 * This may be called after convert_file() has been called and returned true,
 * indicating a successful conversion.  It will return the distance units
 * represented by the converted egg file, if known, or DU_invalid if not
 * known.
 */
DistanceUnit FltToEggConverter::
get_input_units() {
  return _flt_units;
}

/**
 * Fills up the egg_data structure according to the indicated lwo structure.
 */
bool FltToEggConverter::
convert_flt(const FltHeader *flt_header) {
  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_zup_right);
  }

  clear_error();
  _flt_header = flt_header;

  // Generate a default vertex pool.
  _main_egg_vpool = new EggVertexPool("vpool");
  _egg_data->add_child(_main_egg_vpool.p());

  // We could populate the vertex pool right away, but it's better to defer
  // each vertex until we encounter it, since some of the vertices may need to
  // be adjusted to match the particular polygon they're assigned to (for
  // instance, to apply a transparency or something).

  FltToEggLevelState state(this);
  state._egg_parent = _egg_data;
  convert_record(_flt_header, state);

  if (_main_egg_vpool->empty()) {
    // If we didn't get any global vertices, remove the vertex pool just for
    // cleanliness.
    _egg_data->remove_child(_main_egg_vpool.p());
  }

  cleanup();

  return !had_error();
}

/**
 * Frees all the internal data structures after we're done converting, and
 * resets the converter to its initial state.
 */
void FltToEggConverter::
cleanup() {
  _flt_header.clear();
  _main_egg_vpool.clear();
  _textures.clear();
}

/**
 * Converts the record and all of its children.
 */
void FltToEggConverter::
convert_record(const FltRecord *flt_record, FltToEggLevelState &state) {
  int num_children = flt_record->get_num_children();

  for (int i = 0; i < num_children; i++) {
    const FltRecord *child = flt_record->get_child(i);
    dispatch_record(child, state);
  }
}

/**
 * Determines what kind of record this is and calls the appropriate convert
 * function.
 */
void FltToEggConverter::
dispatch_record(const FltRecord *flt_record, FltToEggLevelState &state) {
  if (flt_record->is_of_type(FltLOD::get_class_type())) {
    convert_lod(DCAST(FltLOD, flt_record), state);

  } else if (flt_record->is_of_type(FltGroup::get_class_type())) {
    convert_group(DCAST(FltGroup, flt_record), state);

  } else if (flt_record->is_of_type(FltObject::get_class_type())) {
    convert_object(DCAST(FltObject, flt_record), state);

  } else if (flt_record->is_of_type(FltFace::get_class_type())) {
    convert_face(DCAST(FltFace, flt_record), state);

  } else if (flt_record->is_of_type(FltExternalReference::get_class_type())) {
    convert_ext_ref(DCAST(FltExternalReference, flt_record), state);

    // Fallbacks.
  } else if (flt_record->is_of_type(FltBeadID::get_class_type())) {
    convert_bead_id(DCAST(FltBeadID, flt_record), state);

  } else if (flt_record->is_of_type(FltBead::get_class_type())) {
    convert_bead(DCAST(FltBead, flt_record), state);

  } else {
    convert_record(flt_record, state);
  }
}

/**
 * Converts the LOD bead and all of its children.
 */
void FltToEggConverter::
convert_lod(const FltLOD *flt_lod, FltToEggLevelState &state) {
  EggGroup *egg_group = new EggGroup(flt_lod->get_id());
  state._egg_parent->add_child(egg_group);

  EggSwitchConditionDistance lod
    (flt_lod->_switch_in, flt_lod->_switch_out,
     LPoint3d(flt_lod->_center_x, flt_lod->_center_y, flt_lod->_center_z),
     flt_lod->_transition_range);
  egg_group->set_lod(lod);

  state.set_transform(flt_lod, egg_group);
  parse_comment(flt_lod, egg_group);

  FltToEggLevelState next_state(state);
  next_state._egg_parent = egg_group;
  convert_record(flt_lod, next_state);
}

/**
 * Converts the group and all of its children.
 */
void FltToEggConverter::
convert_group(const FltGroup *flt_group, FltToEggLevelState &state) {
  EggGroup *egg_group = new EggGroup(flt_group->get_id());
  state._egg_parent->add_child(egg_group);

  if ((flt_group->_flags & FltGroup::F_forward_animation) != 0) {
    // It's a sequence animation.
    egg_group->set_switch_flag(true);
    egg_group->set_switch_fps(24.0);
  }

  state.set_transform(flt_group, egg_group);
  parse_comment(flt_group, egg_group);

  // *** replicate count.

  FltToEggLevelState next_state(state);
  next_state._egg_parent = egg_group;
  convert_record(flt_group, next_state);
}

/**
 * Converts the object and all of its children.
 */
void FltToEggConverter::
convert_object(const FltObject *flt_object, FltToEggLevelState &state) {
  EggGroup *egg_group = new EggGroup(flt_object->get_id());
  state._egg_parent->add_child(egg_group);

  state.set_transform(flt_object, egg_group);
  parse_comment(flt_object, egg_group);

  FltToEggLevelState next_state(state);
  next_state._flt_object = flt_object;
  next_state._egg_parent = egg_group;
  convert_record(flt_object, next_state);
}

/**
 * Converts the generic bead (with ID) and all of its children.
 */
void FltToEggConverter::
convert_bead_id(const FltBeadID *flt_bead, FltToEggLevelState &state) {
  nout << "Don't know how to convert beads of type " << flt_bead->get_type()
       << "\n";
  EggGroup *egg_group = new EggGroup(flt_bead->get_id());
  state._egg_parent->add_child(egg_group);

  state.set_transform(flt_bead, egg_group);
  parse_comment(flt_bead, egg_group);

  FltToEggLevelState next_state(state);
  next_state._egg_parent = egg_group;
  convert_record(flt_bead, next_state);
}

/**
 * Converts the generic bead (without ID) and all of its children.
 */
void FltToEggConverter::
convert_bead(const FltBead *flt_bead, FltToEggLevelState &state) {
  nout << "Don't know how to convert beads of type " << flt_bead->get_type()
       << "\n";
  EggGroup *egg_group = new EggGroup;
  state._egg_parent->add_child(egg_group);

  state.set_transform(flt_bead, egg_group);
  parse_comment(flt_bead, egg_group);

  FltToEggLevelState next_state(state);
  next_state._egg_parent = egg_group;
  convert_record(flt_bead, next_state);
}

/**
 * Converts the face and all of its children.
 */
void FltToEggConverter::
convert_face(const FltFace *flt_face, FltToEggLevelState &state) {
  bool is_light;
  switch (flt_face->_draw_type) {
  case FltGeometry::DT_omni_light:
  case FltGeometry::DT_uni_light:
  case FltGeometry::DT_bi_light:
    is_light = true;
    break;

  default:
    is_light = false;
  }

  PT(EggPrimitive) egg_prim;
  if (is_light) {
    egg_prim = new EggPoint;
  } else {
    egg_prim = new EggPolygon;
  }

  // Collect the vertices for this primitive.
  pvector< PT_EggVertex > vertices;

  const FltVertexList *vlist = nullptr;
  int num_children = flt_face->get_num_children();
  for (int i = 0; i < num_children && vlist == nullptr; i++) {
    const FltRecord *child = flt_face->get_child(i);
    if (child->is_of_type(FltVertexList::get_class_type())) {
      vlist = DCAST(FltVertexList, child);
    }
  }

  if (vlist != nullptr) {
    int num_vertices = vlist->get_num_vertices();
    for (int i = 0; i < num_vertices; i++) {
      FltVertex *flt_vertex = vlist->get_vertex(i);
      vertices.push_back(make_egg_vertex(flt_vertex));
    }
  }

  setup_geometry(flt_face, state, egg_prim, _main_egg_vpool, vertices);
}

/**
 * Converts the external reference node.
 */
void FltToEggConverter::
convert_ext_ref(const FltExternalReference *flt_ext, FltToEggLevelState &state) {
  // Get a group node to put the reference into.
  EggGroupNode *egg_parent =
    state.get_synthetic_group("", flt_ext);

  handle_external_reference(egg_parent, flt_ext->get_ref_filename());
}

/**
 * Applies the state indicated in the FltGeometry record to the indicated
 * EggPrimitive and all of its indicated vertices, and then officially adds
 * the vertices to the vertex pool and to the primitive, and adds the
 * primitive to its appropriate parent.
 */
void FltToEggConverter::
setup_geometry(const FltGeometry *flt_geom, FltToEggLevelState &state,
               EggPrimitive *egg_prim, EggVertexPool *egg_vpool,
               const FltToEggConverter::EggVertices &vertices) {

  // Determine what the appropriate parent will be.
  EggGroupNode *egg_parent =
    state.get_synthetic_group(flt_geom->get_id(), flt_geom,
                              flt_geom->_billboard_type);

  // Create a new state to reflect the new parent.
  FltToEggLevelState next_state(state);
  next_state._egg_parent = egg_parent;

  // Check for decals onto the primitive.
  convert_subfaces(flt_geom, next_state);

  // Add the primitive to its new home.
  next_state._egg_parent->add_child(egg_prim);

  // Now examine the vertices.
  EggVertices::const_iterator vi;

  bool use_vertex_color = true;
  bool keep_normals = true;
  switch (flt_geom->_light_mode) {
  case FltGeometry::LM_face_no_normal:
    use_vertex_color = false;
    keep_normals = false;
    break;

  case FltGeometry::LM_vertex_no_normal:
    use_vertex_color = true;
    keep_normals = false;
    break;

  case FltGeometry::LM_face_with_normal:
    use_vertex_color = false;
    keep_normals = true;
    break;

  case FltGeometry::LM_vertex_with_normal:
    use_vertex_color = true;
    keep_normals = true;
    break;
  }

  LColor face_color = flt_geom->get_color();

  if (state._flt_object != nullptr) {
    // If we have a FltObject above us, it might also specify a transparency.
    // This combines with our existing transparency.
    PN_stdfloat alpha = 1.0 - (state._flt_object->_transparency / 65535.0);
    face_color[3] *= alpha;
  }

  egg_prim->set_color(face_color);

  if (flt_geom->has_texture()) {
    // If the geometry has a texture, apply it.
    egg_prim->set_texture(make_egg_texture(flt_geom->get_texture()));

    if (flt_geom->_texwhite) {
      // If the geometry should be colored white under the texture, then
      // eliminate vertex colors.
      use_vertex_color = false;
    }
  }

  if (use_vertex_color) {
    // If we're to use vertex color instead of the face color, remove the face
    // color to eliminate any ambiguity.
    egg_prim->clear_color();

    // Also, make sure the transparency is set correctly across all vertices.
    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      EggVertex *vertex = (*vi);
      if (vertex->has_color()) {
        LColor vertex_color = vertex->get_color();
        vertex_color[3] = face_color[3];
        vertex->set_color(vertex_color);
      } else {
        if (flt_geom->has_color()) {
          // If a vertex doesn't have a color but the face does, set the
          // vertex to use the face color.
          vertex->set_color(face_color);
        }
      }
    }

  } else {
    // If we're to use face color instead of vertex color, remove the vertex
    // color to eliminate any ambiguity.
    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      (*vi)->clear_color();
    }
  }

  if (!keep_normals) {
    // If we're not to use the normals, then eliminate them.
    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      (*vi)->clear_normal();
    }
  }

  if (flt_geom->_draw_type == FltGeometry::DT_solid_no_cull) {
    // A double-sided polygon.
    egg_prim->set_bface_flag(true);
  }

  for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
    EggVertex *egg_vertex = egg_vpool->create_unique_vertex(*(*vi));
    egg_prim->add_vertex(egg_vertex);
  }

  parse_comment(flt_geom, egg_prim);
}

/**
 * Records all of the subfaces of the indicated group as coplanar polygons
 * (i.e.  decals) of the group.
 *
 * If coplanar polygons exist, the state is modified so that _egg_parent is
 * the new group to which the base polygons should be added.  Therefore,
 * subfaces should be defined before the ordinary children are processed.
 */
void FltToEggConverter::
convert_subfaces(const FltRecord *flt_record, FltToEggLevelState &state) {
  int num_subfaces = flt_record->get_num_subfaces();
  if (num_subfaces == 0) {
    // No subfaces.
    return;
  }

  // Create a new group to contain the base polygons.
  EggGroup *egg_group = new EggGroup("decal_base");
  state._egg_parent->add_child(egg_group);
  state._egg_parent = egg_group;

  egg_group->set_decal_flag(true);

  // Now create a nested group to hold the decals.
  EggGroup *decal_group = new EggGroup("decals");
  egg_group->add_child(decal_group);
  egg_group = decal_group;

  FltToEggLevelState next_state(state);
  next_state._egg_parent = decal_group;

  for (int i = 0; i < num_subfaces; i++) {
    const FltRecord *subface = flt_record->get_subface(i);
    dispatch_record(subface, next_state);
  }
}

/**
 * Scans the comment on this record for "<egg> { ... }" and parses the
 * enclosed string as if it appeared in the egg file.  Returns true on
 * success, false on syntax error (in which case _error is also set to true).
 */
bool FltToEggConverter::
parse_comment(const FltBeadID *flt_bead, EggNode *egg_node) {
  return parse_comment(flt_bead->get_comment(), flt_bead->get_id(), egg_node);
}

/**
 * Scans the comment on this record for "<egg> { ... }" and parses the
 * enclosed string as if it appeared in the egg file.  Returns true on
 * success, false on syntax error (in which case _error is also set to true).
 */
bool FltToEggConverter::
parse_comment(const FltBead *flt_bead, EggNode *egg_node) {
  return parse_comment(flt_bead->get_comment(), "anonymous", egg_node);
}

/**
 * Scans the comment on this record for "<egg> { ... }" and parses the
 * enclosed string as if it appeared in the egg file.  Returns true on
 * success, false on syntax error (in which case _error is also set to true).
 */
bool FltToEggConverter::
parse_comment(const FltTexture *flt_texture, EggNode *egg_node) {
  return parse_comment(flt_texture->get_comment(),
                       flt_texture->get_texture_filename(), egg_node);
}

/**
 * Scans the comment on this record for "<egg> { ... }" and parses the
 * enclosed string as if it appeared in the egg file.  Returns true on
 * success, false on syntax error (in which case _error is also set to true).
 */
bool FltToEggConverter::
parse_comment(const string &comment, const string &name,
              EggNode *egg_node) {
  if (comment.empty()) {
    // No comment.
    return true;
  }

  // Scan for <egg>.
  static const string egg_str = "<egg>";

  size_t p;
  p = 0;
  while (p < comment.length() &&
         cmp_nocase(comment.substr(p, 5), egg_str) != 0) {
    p++;
  }

  if (p >= comment.length()) {
    // No "<egg>" in the comment.
    return true;
  }

  p += 5;
  // Now scan past whitespace for the open curly brace.
  while (p < comment.length() && isspace(comment[p])) {
    ++p;
  }
  if (p >= comment.length() || comment[p] != '{') {
    nout << "No opening brace in comment for "
         << name << "\n\n";
    _error = true;
    return false;
  }

  // Here's the beginning of the string after "<egg> {".  Now lop off the
  // closing brace at the end.
  ++p;
  size_t q = comment.length() - 1;
  while (q > p && comment[q] != '}') {
    --q;
  }
  if (q == p) {
    nout << "No closing brace in comment for "
         << name << "\n\n";
    _error = true;
    return false;
  }

  string egg_syntax = comment.substr(p, q - p);

  if (!egg_node->parse_egg(egg_syntax)) {
    nout << "Syntax error in comment for "
         << name << "\n\n";
    _error = true;
    return false;
  }

  // Correctly parsed!
  return true;
}

/**
 * Makes a new EggVertex for the indicated FltVertex.  The vertex is not
 * automatically added to the vertex pool.
 */
PT_EggVertex FltToEggConverter::
make_egg_vertex(const FltVertex *flt_vertex) {
  PT_EggVertex egg_vertex = new EggVertex;
  egg_vertex->set_pos(flt_vertex->_pos);

  if (flt_vertex->_has_normal) {
    egg_vertex->set_normal(LCAST(double, flt_vertex->_normal));
  }

  if (flt_vertex->_has_uv) {
    egg_vertex->set_uv(LCAST(double, flt_vertex->_uv));
  }

  if (flt_vertex->has_color()) {
    egg_vertex->set_color(flt_vertex->get_color());
  }

  return egg_vertex;
}

/**
 * Makes a new EggTexture for the indicated FltTexture, or returns a pointer
 * to one previously made for the same FltTexture.
 */
PT_EggTexture FltToEggConverter::
make_egg_texture(const FltTexture *flt_texture) {
  Textures::const_iterator ti;
  ti = _textures.find(flt_texture);
  if (ti != _textures.end()) {
    // There's one previously created.
    return (*ti).second;
  }

  // Create a new one.
  string tref_name = format_string(flt_texture->_pattern_index);
  Filename filename = flt_texture->get_texture_filename();

  PT_EggTexture egg_texture = new EggTexture(tref_name, filename);

  _textures.insert(Textures::value_type(flt_texture, egg_texture));

  // Set up the texture properties.

  switch (flt_texture->_min_filter) {
  case FltTexture::MN_point:
    egg_texture->set_minfilter(EggTexture::FT_nearest);
    break;

  case FltTexture::MN_bilinear:
    egg_texture->set_minfilter(EggTexture::FT_linear);
    break;

  case FltTexture::MN_mipmap_point:
    egg_texture->set_minfilter(EggTexture::FT_nearest_mipmap_nearest);
    break;

  case FltTexture::MN_mipmap_linear:
    egg_texture->set_minfilter(EggTexture::FT_nearest_mipmap_linear);
    break;

  case FltTexture::MN_mipmap_bilinear:
    egg_texture->set_minfilter(EggTexture::FT_linear_mipmap_nearest);
    break;

  case FltTexture::MN_mipmap_trilinear:
  case FltTexture::MN_OB_mipmap:
    egg_texture->set_minfilter(EggTexture::FT_linear_mipmap_linear);
    break;

  case FltTexture::MN_bicubic:
  case FltTexture::MN_bilinear_gequal:
  case FltTexture::MN_bilinear_lequal:
  case FltTexture::MN_bicubic_gequal:
  case FltTexture::MN_bicubic_lequal:
    // Not supported.
    break;
  }

  switch (flt_texture->_mag_filter) {
  case FltTexture::MG_point:
    egg_texture->set_magfilter(EggTexture::FT_nearest);
    break;

  case FltTexture::MG_bilinear:
    egg_texture->set_magfilter(EggTexture::FT_linear);
    break;

  case FltTexture::MG_bicubic:
  case FltTexture::MG_sharpen:
  case FltTexture::MG_add_detail:
  case FltTexture::MG_modulate_detail:
  case FltTexture::MG_bilinear_gequal:
  case FltTexture::MG_bilinear_lequal:
  case FltTexture::MG_bicubic_gequal:
  case FltTexture::MG_bicubic_lequal:
    // Not supported.
    break;
  }

  switch (flt_texture->_repeat) {
  case FltTexture::RT_repeat:
    egg_texture->set_wrap_mode(EggTexture::WM_repeat);
    break;

  case FltTexture::RT_clamp:
    egg_texture->set_wrap_mode(EggTexture::WM_clamp);
    break;
  }

  switch (flt_texture->_repeat_u) {
  case FltTexture::RT_repeat:
    egg_texture->set_wrap_u(EggTexture::WM_repeat);
    break;

  case FltTexture::RT_clamp:
    egg_texture->set_wrap_u(EggTexture::WM_clamp);
    break;
  }

  switch (flt_texture->_repeat_v) {
  case FltTexture::RT_repeat:
    egg_texture->set_wrap_v(EggTexture::WM_repeat);
    break;

  case FltTexture::RT_clamp:
    egg_texture->set_wrap_v(EggTexture::WM_clamp);
    break;
  }

  switch (flt_texture->_env_type) {
  case FltTexture::ET_modulate:
    egg_texture->set_env_type(EggTexture::ET_modulate);
    break;

  case FltTexture::ET_decal:
    egg_texture->set_env_type(EggTexture::ET_decal);
    break;

  case FltTexture::ET_blend:
  case FltTexture::ET_color:
    // Not supported.
    break;
  }

  switch (flt_texture->_internal_format) {
  case FltTexture::IF_default:
    break;

  case FltTexture::IF_i_12a_4:
  case FltTexture::IF_ia_12:
  case FltTexture::IF_ia_8:
    egg_texture->set_format(EggTexture::F_luminance_alpha);
    break;

  case FltTexture::IF_rgb_5:
    egg_texture->set_format(EggTexture::F_rgb5);
    break;

  case FltTexture::IF_rgba_4:
    egg_texture->set_format(EggTexture::F_rgba4);
    break;


  case FltTexture::IF_rgba_8:
    egg_texture->set_format(EggTexture::F_rgba8);
    break;

  case FltTexture::IF_rgba_12:
    egg_texture->set_format(EggTexture::F_rgba12);
    break;

  case FltTexture::IF_i_16:
    if (flt_texture->_intensity_is_alpha) {
      egg_texture->set_format(EggTexture::F_alpha);
    } else {
      egg_texture->set_format(EggTexture::F_luminance);
    }
    break;

  case FltTexture::IF_rgb_12:
    egg_texture->set_format(EggTexture::F_rgb12);
    break;
  }

  parse_comment(flt_texture, egg_texture);
  return egg_texture;
}
