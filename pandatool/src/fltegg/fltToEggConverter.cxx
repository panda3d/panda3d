// Filename: fltToEggConverter.cxx
// Created by:  drose (17Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "fltToEggConverter.h"

#include <fltRecord.h>
#include <fltLOD.h>
#include <fltGroup.h>
#include <fltObject.h>
#include <fltBeadID.h>
#include <fltBead.h>
#include <fltFace.h>
#include <fltVertex.h>
#include <fltVertexList.h>
#include <eggGroup.h>
#include <eggSwitchCondition.h>
#include <eggPrimitive.h>
#include <eggPolygon.h>
#include <eggPoint.h>
#include <eggVertex.h>
#include <eggVertexPool.h>
#include <string_utils.h>


////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltToEggConverter::
FltToEggConverter(EggData &egg_data) : _egg_data(egg_data) {
  _input_units = DU_invalid;
  _output_units = DU_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_flt
//       Access: Public
//  Description: Returns a newly-allocated EggData structure
//               corresponding to the indicated flt structure.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_flt(const FltHeader *flt_header) {
  _flt_header = flt_header;
  
  if (_input_units == DU_invalid) {
    switch (_flt_header->_vertex_units) {
    case FltHeader::U_meters:
      _input_units = DU_meters;
      break;

    case FltHeader::U_kilometers:
      _input_units = DU_kilometers;
      break;

    case FltHeader::U_feet:
      _input_units = DU_feet;
      break;

    case FltHeader::U_inches:
      _input_units = DU_inches;
      break;

    case FltHeader::U_nautical_miles:
      _input_units = DU_nautical_miles;
      break;
    }
  }

  // Generate a default vertex pool.
  _main_egg_vpool = new EggVertexPool("vpool");
  _egg_data.add_child(_main_egg_vpool);

  // We could populate the vertex pool right away, but it's better to
  // defer each vertex until we encounter it, since some of the
  // vertices may need to be adjusted to match the particular polygon
  // they're assigned to (for instance, to apply a transparency or
  // something).

  convert_record(_flt_header, (FltObject *)NULL, &_egg_data);

  if (_main_egg_vpool->empty()) {
    // If we didn't get any global vertices, remove the vertex pool
    // just for cleanliness.
    _egg_data.remove_child(_main_egg_vpool);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_record
//       Access: Private
//  Description: Converts the record and all of its children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_record(const FltRecord *flt_record, const FltObject *flt_object,
	       EggGroupNode *egg_group) {
  int num_children = flt_record->get_num_children();

  for (int i = 0; i < num_children; i++) {
    const FltRecord *child = flt_record->get_child(i);

    if (child->is_of_type(FltLOD::get_class_type())) {
      convert_lod(DCAST(FltLOD, child), flt_object, egg_group);

    } else if (child->is_of_type(FltGroup::get_class_type())) {
      convert_group(DCAST(FltGroup, child), flt_object, egg_group);

    } else if (child->is_of_type(FltObject::get_class_type())) {
      convert_object(DCAST(FltObject, child), flt_object, egg_group);

    } else if (child->is_of_type(FltFace::get_class_type())) {
      convert_face(DCAST(FltFace, child), flt_object, egg_group);

      // Fallbacks.
    } else if (child->is_of_type(FltBeadID::get_class_type())) {
      convert_bead_id(DCAST(FltBeadID, child), flt_object, egg_group);

    } else if (child->is_of_type(FltBead::get_class_type())) {
      convert_bead(DCAST(FltBead, child), flt_object, egg_group);

    } else {
      convert_record(child, flt_object, egg_group);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_lod
//       Access: Private
//  Description: Converts the LOD bead and all of its children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_lod(const FltLOD *flt_lod, const FltObject *flt_object,
	    EggGroupNode *egg_parent) {
  EggGroup *egg_group = new EggGroup(flt_lod->get_id());
  egg_parent->add_child(egg_group);

  EggSwitchConditionDistance lod
    (flt_lod->_switch_in, flt_lod->_switch_out,
     LPoint3d(flt_lod->_center_x, flt_lod->_center_y, flt_lod->_center_z),
     flt_lod->_transition_range);
  egg_group->set_lod(lod);

  convert_record(flt_lod, flt_object, egg_group);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_group
//       Access: Private
//  Description: Converts the group and all of its children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_group(const FltGroup *flt_group, const FltObject *flt_object,
	      EggGroupNode *egg_parent) {
  EggGroup *egg_group = new EggGroup(flt_group->get_id());
  egg_parent->add_child(egg_group);

  if ((flt_group->_flags & FltGroup::F_forward_animation) != 0) {
    // It's a sequence animation.
    egg_group->set_switch_flag(true);
    egg_group->set_switch_fps(24.0);
  }

  if (flt_group->has_transform()) {
    egg_group->set_transform(flt_group->get_transform());
  }

  ///*** replicate count.

  convert_record(flt_group, flt_object, egg_group);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_object
//       Access: Private
//  Description: Converts the object and all of its children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_object(const FltObject *flt_object, const FltObject *,
	       EggGroupNode *egg_parent) {
  EggGroup *egg_group = new EggGroup(flt_object->get_id());
  egg_parent->add_child(egg_group);

  if (flt_object->has_transform()) {
    egg_group->set_transform(flt_object->get_transform());
  }

  convert_record(flt_object, flt_object, egg_group);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_bead_id
//       Access: Private
//  Description: Converts the generic bead (with ID) and all of its
//               children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_bead_id(const FltBeadID *flt_bead, const FltObject *flt_object,
		EggGroupNode *egg_parent) {
  EggGroup *egg_group = new EggGroup(flt_bead->get_id());
  egg_parent->add_child(egg_group);

  if (flt_bead->has_transform()) {
    egg_group->set_transform(flt_bead->get_transform());
  }

  convert_record(flt_bead, flt_object, egg_group);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_bead
//       Access: Private
//  Description: Converts the generic bead (without ID) and all of its
//               children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_bead(const FltBead *flt_bead, const FltObject *flt_object,
	     EggGroupNode *egg_parent) {
  EggGroup *egg_group = new EggGroup;
  egg_parent->add_child(egg_group);

  if (flt_bead->has_transform()) {
    egg_group->set_transform(flt_bead->get_transform());
  }

  convert_record(flt_bead, flt_object, egg_group);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::convert_face
//       Access: Private
//  Description: Converts the face and all of its children.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
convert_face(const FltFace *flt_face, const FltObject *flt_object,
	     EggGroupNode *egg_parent) {
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

  EggPrimitive *egg_prim;
  if (is_light) {
    egg_prim = new EggPoint;
  } else {
    egg_prim = new EggPolygon;
  }

  egg_parent->add_child(egg_prim);

  // Collect the vertices for this primitive.
  vector< PT(EggVertex) > vertices;

  const FltVertexList *vlist = (FltVertexList *)NULL;
  int num_children = flt_face->get_num_children();
  for (int i = 0; i < num_children && vlist == (FltVertexList *)NULL; i++) {
    const FltRecord *child = flt_face->get_child(i);
    if (child->is_of_type(FltVertexList::get_class_type())) {
      vlist = DCAST(FltVertexList, child);
    }
  }

  if (vlist != (FltVertexList *)NULL) {
    int num_vertices = vlist->get_num_vertices();
    for (int i = 0; i < num_vertices; i++) {
      FltVertex *flt_vertex = vlist->get_vertex(i);
      vertices.push_back(make_egg_vertex(flt_vertex));
    }
  }

  setup_geometry(flt_face, flt_object, egg_prim, _main_egg_vpool, vertices);
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::setup_geometry
//       Access: Private
//  Description: Applies the state indicated in the FltGeometry record
//               to the indicated EggPrimitive and all of its
//               indicated vertices, and then officially adds the
//               vertices to the vertex pool and to the primitive.
////////////////////////////////////////////////////////////////////
void FltToEggConverter::
setup_geometry(const FltGeometry *flt_geom, const FltObject *,
	       EggPrimitive *egg_prim, EggVertexPool *egg_vpool,
	       const FltToEggConverter::EggVertices &vertices) {
  EggVertices::const_iterator vi;

  if (flt_geom->has_color()) {
    egg_prim->set_color(flt_geom->get_color());
  }

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

  if (flt_geom->has_texture()) {
    // If the geometry has a texture, apply it.
    egg_prim->set_texture(make_egg_texture(flt_geom->get_texture()));

    if (flt_geom->_texwhite) {
      // If the geometry should be colored white under the texture,
      // then eliminate any explicit color; the egg loader will
      // implicitly color it white.
      egg_prim->clear_color();
      use_vertex_color = false;
    }
  }

  if (use_vertex_color) {
    // If we're to use vertex color instead of the face color, remove
    // the face color to eliminate any ambiguity.
    if (flt_geom->has_color()) {
      // Also, if some of our vertices don't have a color, set them to
      // use the face color.
      for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
	if (!(*vi)->has_color()) {
	  (*vi)->set_color(flt_geom->get_color());
	}
      }
      egg_prim->clear_color();
    }

  } else {
    // If we're to use face color instead of vertex color, remove the
    // vertex color to eliminate any ambiguity.
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

  for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
    EggVertex *egg_vertex = egg_vpool->create_unique_vertex(*(*vi));
    egg_prim->add_vertex(egg_vertex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::make_egg_vertex
//       Access: Private
//  Description: Makes a new EggVertex for the indicated FltVertex.
//               The vertex is not automatically added to the vertex
//               pool.
////////////////////////////////////////////////////////////////////
PT(EggVertex) FltToEggConverter::
make_egg_vertex(const FltVertex *flt_vertex) {
  PT(EggVertex) egg_vertex = new EggVertex;
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

////////////////////////////////////////////////////////////////////
//     Function: FltToEggConverter::make_egg_texture
//       Access: Private
//  Description: Makes a new EggTexture for the indicated FltTexture,
//               or returns a pointer to one previously made for the
//               same FltTexture.
////////////////////////////////////////////////////////////////////
PT(EggTexture) FltToEggConverter::
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
  PT(EggTexture) egg_texture = new EggTexture(tref_name, filename);

  _textures.insert(Textures::value_type(flt_texture, egg_texture));
  return egg_texture;
}
