/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file daeToEggConverter.cxx
 * @author rdb
 * @date 2008-05-08
 */

#include "daeToEggConverter.h"
#include "fcollada_utils.h"
#include "config_daeegg.h"
#include "daeCharacter.h"
#include "dcast.h"
#include "string_utils.h"
#include "eggData.h"
#include "eggPrimitive.h"
#include "eggLine.h"
#include "eggPolygon.h"
#include "eggTriangleFan.h"
#include "eggTriangleStrip.h"
#include "eggPoint.h"
#include "eggXfmSAnim.h"
#include "eggSAnimData.h"
#include "pt_EggVertex.h"

#include <FCDocument/FCDAsset.h>
#include <FCDocument/FCDocumentTools.h>
#include <FCDocument/FCDSceneNode.h>
#include <FCDocument/FCDSceneNodeTools.h>
#include <FCDocument/FCDGeometry.h>
#include <FCDocument/FCDGeometryInstance.h>
#include <FCDocument/FCDGeometryPolygons.h>
#include <FCDocument/FCDGeometrySource.h>
#include <FCDocument/FCDSkinController.h>
#include <FCDocument/FCDController.h>
#include <FCDocument/FCDControllerInstance.h>
#include <FCDocument/FCDMorphController.h>
#include <FCDocument/FCDMaterialInstance.h>
#include <FCDocument/FCDExtra.h>
#include <FCDocument/FCDEffect.h>
#include <FCDocument/FCDEffectStandard.h>
#if FCOLLADA_VERSION >= 0x00030005
  #include <FCDocument/FCDGeometryPolygonsInput.h>
#endif

using std::endl;
using std::string;

/**
 *
 */
DAEToEggConverter::
DAEToEggConverter() {
  _unit_name = "meter";
  _unit_meters = 1.0;
  _document = nullptr;
  _table = nullptr;
  _error_handler = nullptr;
  _invert_transparency = false;
}

/**
 *
 */
DAEToEggConverter::
DAEToEggConverter(const DAEToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

/**
 *
 */
DAEToEggConverter::
~DAEToEggConverter() {
  if (_error_handler != nullptr) {
    delete _error_handler;
  }
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *DAEToEggConverter::
make_copy() {
  return new DAEToEggConverter(*this);
}


/**
 * Returns the English name of the file type this converter supports.
 */
string DAEToEggConverter::
get_name() const {
  return "COLLADA";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
string DAEToEggConverter::
get_extension() const {
  return "dae";
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 */
bool DAEToEggConverter::
convert_file(const Filename &filename) {
  // Reset stuff
  clear_error();
  _joints.clear();
  if (_error_handler == nullptr) {
    _error_handler = new FUErrorSimpleHandler;
  }

  // The default coordinate system is Y-up
  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_yup_right);
  }

  // Read the file
  FCollada::Initialize();
  _document = FCollada::LoadDocument(filename.to_os_specific().c_str());
  if (_document == nullptr) {
    daeegg_cat.error() << "Failed to load document: " << _error_handler->GetErrorString() << endl;
    FCollada::Release();
    return false;
  }
  // Make sure the file uses consistent coordinate system and length
  if (_document->GetAsset() != nullptr) {
    FCDocumentTools::StandardizeUpAxisAndLength(_document);
  }

  // Process the scene
  process_asset();
  PT(EggGroup) scene_group;
  string model_name = _character_name;

  FCDSceneNode* visual_scene = _document->GetVisualSceneInstance();
  if (visual_scene != nullptr) {
    if (model_name.empty()) {
      // By lack of anything better...
      model_name = FROM_FSTRING(visual_scene->GetName());
    }
    scene_group = new EggGroup(model_name);
    _egg_data->add_child(scene_group);

    for (size_t ch = 0; ch < visual_scene->GetChildrenCount(); ++ch) {
      process_node(scene_group, visual_scene->GetChild(ch));
    }
  } else {
    daeegg_cat.warning()
      << "No visual scene instance found in COLLADA document.\n";
  }

  // Now process the characters.  This depends on information from collected
  // joints, which is why it's done in a second step.
  if (get_animation_convert() != AC_none) {
    Characters::iterator it;
    DaeCharacter *character;
    for (it = _characters.begin(); it != _characters.end(); ++it) {
      character = *it;
      if (get_animation_convert() != AC_chan) {
        character->bind_joints(_joints);

        const FCDGeometryMesh *mesh = character->_skin_mesh;

        if (mesh != nullptr) {
          PT(DaeMaterials) materials = new DaeMaterials(character->_instance);
          if (daeegg_cat.is_spam()) {
            daeegg_cat.spam() << "Processing mesh for controller\n";
          }
          process_mesh(character->_node_group, mesh, materials, character);
        }
      }
    }

    // Put the joints in bind pose.
    for (size_t ch = 0; ch < visual_scene->GetChildrenCount(); ++ch) {
      character->adjust_joints(visual_scene->GetChild(ch), _joints, LMatrix4d::ident_mat());
    }

    if (scene_group != nullptr) {
      // Mark the scene as character.
      if (get_animation_convert() == AC_chan) {
        _egg_data->remove_child(scene_group);
      } else {
        scene_group->set_dart_type(EggGroup::DT_default);
      }
    }

    if (get_animation_convert() != AC_model) {
      _table = new EggTable();
      _table->set_table_type(EggTable::TT_table);
      _egg_data->add_child(_table);

      PT(EggTable) bundle = new EggTable(model_name);
      bundle->set_table_type(EggTable::TT_bundle);
      _table->add_child(bundle);

      PT(EggTable) skeleton = new EggTable("<skeleton>");
      skeleton->set_table_type(EggTable::TT_table);
      bundle->add_child(skeleton);

      pset<float> keys;

      Characters::iterator it;
      DaeCharacter *character;
      for (it = _characters.begin(); it != _characters.end(); ++it) {
        character = *it;

        // Collect key frame timings.
        if (get_animation_convert() == AC_both ||
            get_animation_convert() == AC_chan) {
          character->collect_keys(keys);
        }
      }

      if (_frame_inc != 0.0) {
        // A frame increment was given, this means that we have to sample the
        // animation.
        float start, end;
        if (_end_frame != _start_frame) {
          start = _start_frame;
          end = _end_frame;
        } else {
          // No range was given.  Infer the frame range from the keys.
          start = *keys.begin();
          end = *keys.rbegin();
        }
        keys.clear();

        for (float t = start; t <= end; t += _frame_inc) {
          keys.insert(t);
        }
      } else {
        // No sampling parameters given; not necessarily a failure, since the
        // animation may already be sampled.  We use the key frames as
        // animation frames.
        if (_end_frame != 0.0) {
          // An end frame was given, chop off all keys after that.
          float end = _end_frame;
          pset<float>::iterator ki;
          for (ki = keys.begin(); ki != keys.end(); ++ki) {
            if (*ki > end && !IS_THRESHOLD_EQUAL(*ki, end, 0.001)) {
              keys.erase(ki, keys.end());
              break;
            }
          }
        }
        if (_start_frame != 0.0) {
          // A start frame was given, chop off all keys before that.
          float start = _start_frame;
          pset<float>::iterator ki;
          for (ki = keys.begin(); ki != keys.end(); ++ki) {
            if (*ki > start && !IS_THRESHOLD_EQUAL(*ki, start, 0.001)) {
              keys.erase(keys.begin(), ki);
              break;
            }
          }
        }

        // Check that this does indeed look like a sampled animation; if not,
        // issue an appropriate warning.
        pset<float>::const_iterator ki = keys.begin();
        if (ki != keys.end()) {
          float last = *ki;
          float diff = 0;

          for (++ki; ki != keys.end(); ++ki) {
            if (diff != 0 && !IS_THRESHOLD_EQUAL((*ki - last), diff, 0.001)) {
              daeegg_cat.error()
                << "This does not appear to be a sampled animation.\n"
                << "Specify the -sf, -ef and -if options to indicate how the "
                << "animations should be sampled.\n";
              break;
            }
            diff = (*ki - last);
            last = *ki;
          }
        }
      }

      // It doesn't really matter which character we grab for this as it'll
      // iterate over the whole graph right now anyway.
      for (size_t ch = 0; ch < visual_scene->GetChildrenCount(); ++ch) {
        character->build_table(skeleton, visual_scene->GetChild(ch), keys);
      }
    }
  }

  // Clean up and return
  SAFE_DELETE(visual_scene);
  SAFE_DELETE(_document);
  FCollada::Release();
  return true;
}

/**
 * This may be called after convert_file() has been called and returned true,
 * indicating a successful conversion.  It will return the distance units
 * represented by the converted egg file, if known, or DU_invalid if not
 * known.
 */
DistanceUnit DAEToEggConverter::
get_input_units() {
  if (IS_NEARLY_EQUAL(_unit_meters, 0.001)) {
    return DU_millimeters;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 0.01)) {
    return DU_centimeters;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 1.0)) {
    return DU_meters;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 1000.0)) {
    return DU_kilometers;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 3.0 * 12.0 * 0.0254)) {
    return DU_yards;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 12.0 * 0.0254)) {
    return DU_feet;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 0.0254)) {
    return DU_inches;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 1852.0)) {
    return DU_nautical_miles;
  }
  if (IS_NEARLY_EQUAL(_unit_meters, 5280.0 * 12.0 * 0.0254)) {
    return DU_statute_miles;
  }

  // Whatever.
  return DU_invalid;
}

void DAEToEggConverter::
process_asset() {
  const FCDAsset *asset = _document->GetAsset();
  if (_document->GetAsset() == nullptr) {
    return;
  }

  _unit_name = FROM_FSTRING(asset->GetUnitName());
  _unit_meters = asset->GetUnitConversionFactor();

  // Read out the coordinate system
  FMVector3 up_axis = asset->GetUpAxis();

  if (up_axis == FMVector3(0, 1, 0)) {
    _egg_data->set_coordinate_system(CS_yup_right);

  } else if (up_axis == FMVector3(0, 0, 1)) {
    _egg_data->set_coordinate_system(CS_zup_right);

  } else {
    _egg_data->set_coordinate_system(CS_invalid);
    daeegg_cat.warning() << "Unrecognized coordinate system!\n";
  }
}

// Process the node.  If forced is true, it will even process it if its known
// to be a skeleton root.
void DAEToEggConverter::
process_node(EggGroupNode *parent, const FCDSceneNode* node, bool forced) {
  nassertv(node != nullptr);
  string node_id = FROM_FSTRING(node->GetDaeId());
  if (daeegg_cat.is_spam()) {
    daeegg_cat.spam() << "Processing node with ID '" << node_id << "'" << endl;
  }

  // Create an egg group for this node
  PT(EggGroup) node_group = new EggGroup(FROM_FSTRING(node->GetDaeId()));
  process_extra(node_group, node->GetExtra());
  parent->add_child(node_group);

  // Check if its a joint
  if (node->IsJoint()) {
    string sid = FROM_FSTRING(node->GetSubId());
    node_group->set_group_type(EggGroup::GT_joint);

    if (!_joints.insert(DaeCharacter::JointMap::value_type(sid,
                        DaeCharacter::Joint(node_group, node))).second) {
      daeegg_cat.error()
        << "Joint with sid " << sid << " occurs more than once!\n";
    }
  }

  // Loop through the transforms and apply them (in reverse order)
  for (size_t tr = node->GetTransformCount(); tr > 0; --tr) {
    apply_transform(node_group, node->GetTransform(tr - 1));
  }
  // node_group->set_transform3d(convert_matrix(node->ToMatrix()));

  // Loop through the instances and process them
  for (size_t in = 0; in < node->GetInstanceCount(); ++in) {
    process_instance(node_group, node->GetInstance(in));
  }

  // Loop through the children and recursively process them
  for (size_t ch = 0; ch < node->GetChildrenCount(); ++ch) {
    process_node(DCAST(EggGroupNode, node_group), node->GetChild(ch));
  }

  // Loop through any possible scene node instances and process those, too.
  for (size_t in = 0; in < node->GetInstanceCount(); ++in) {
    const FCDEntity *entity = node->GetInstance(in)->GetEntity();
    if (entity && entity->GetType() == FCDEntity::SCENE_NODE) {
      process_node(node_group, (const FCDSceneNode*) entity);
    }
  }
}

void DAEToEggConverter::
process_instance(EggGroup *parent, const FCDEntityInstance* instance) {
  nassertv(instance != nullptr);
  nassertv(instance->GetEntity() != nullptr);
  // Check what kind of instance this is
  switch (instance->GetType()) {
  case FCDEntityInstance::GEOMETRY:
    {
      if (get_animation_convert() != AC_chan) {
        const FCDGeometry* geometry = (const FCDGeometry*) instance->GetEntity();
        assert(geometry != nullptr);
        if (geometry->IsMesh()) {
          // Now, handle the mesh.
          process_mesh(parent, geometry->GetMesh(), new DaeMaterials((const FCDGeometryInstance*) instance));
        }
        if (geometry->IsSpline()) {
          process_spline(parent, FROM_FSTRING(geometry->GetName()), const_cast<FCDGeometrySpline*> (geometry->GetSpline()));
        }
      }
    }
    break;

  case FCDEntityInstance::CONTROLLER:
    // Add the dart tag and process the controller instance
    // parent->set_dart_type(EggGroup::DT_default);
    process_controller(parent, (const FCDControllerInstance*) instance);
    break;

  case FCDEntityInstance::MATERIAL:
    // We don't process this directly, handled per-geometry instead.
    break;

  case FCDEntityInstance::SIMPLE:
    {
      // Grab the entity and check its type.
      const FCDEntity* entity = instance->GetEntity();
      if (entity->GetType() != FCDEntity::SCENE_NODE) {
        daeegg_cat.warning() << "Unsupported entity type found" << endl;
      }
    }
    break;

  default:
    daeegg_cat.warning() << "Unsupported instance type found" << endl;
  }
}

// Processes the given mesh.
void DAEToEggConverter::
process_mesh(EggGroup *parent, const FCDGeometryMesh* mesh,
             DaeMaterials *materials, DaeCharacter *character) {

  nassertv(mesh != nullptr);
  if (daeegg_cat.is_debug()) {
    daeegg_cat.debug() << "Processing mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << endl;
  }

  // Create the egg stuff to hold this mesh
  PT(EggGroup) mesh_group = new EggGroup(FROM_FSTRING(mesh->GetDaeId()));
  parent->add_child(mesh_group);
  PT(EggVertexPool) mesh_pool = new EggVertexPool(FROM_FSTRING(mesh->GetDaeId()));
  mesh_group->add_child(mesh_pool);

  // First retrieve the vertex source
  if (mesh->GetSourceCount() == 0) {
    if (daeegg_cat.is_debug()) {
      daeegg_cat.debug() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has no sources" << endl;
    }
    return;
  }
  const FCDGeometrySource* vsource = mesh->FindSourceByType(FUDaeGeometryInput::POSITION);
  if (vsource == nullptr) {
    if (daeegg_cat.is_debug()) {
      daeegg_cat.debug() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has no source for POSITION data" << endl;
    }
    return;
  }

  // Loop through the polygon groups and add them
  if (daeegg_cat.is_spam()) {
    daeegg_cat.spam() << "Mesh with id " << FROM_FSTRING(mesh->GetDaeId()) << " has " << mesh->GetPolygonsCount() << " polygon groups" << endl;
  }
  if (mesh->GetPolygonsCount() == 0) return;

  // This is an array of pointers, I know.  But since they are refcounted, I
  // don't have a better idea.
  PT(EggGroup) *primitive_holders = new PT(EggGroup) [mesh->GetPolygonsCount()];
  for (size_t gr = 0; gr < mesh->GetPolygonsCount(); ++gr) {
    const FCDGeometryPolygons* polygons = mesh->GetPolygons(gr);
    string material_semantic = FROM_FSTRING(polygons->GetMaterialSemantic());

    // Stores which group holds the primitives.
    PT(EggGroup) primitiveholder;
    // If we have materials, make a group for each material.  Then, apply the
    // material's per-group stuff.
    if (materials != nullptr && (!polygons->GetMaterialSemantic().empty()) && mesh->GetPolygonsCount() > 1) {
      // primitiveholder = new EggGroup(FROM_FSTRING(mesh->GetDaeId()) + "." +
      // material_semantic);
      primitiveholder = new EggGroup;
      mesh_group->add_child(primitiveholder);
    } else {
      primitiveholder = mesh_group;
    }
    primitive_holders[gr] = primitiveholder;
    // Apply the per-group data of the materials, if we have it.
    if (materials != nullptr) {
      materials->apply_to_group(material_semantic, primitiveholder, _invert_transparency);
    }
    // Find the position sources
    const FCDGeometryPolygonsInput* pinput = polygons->FindInput(FUDaeGeometryInput::POSITION);
    assert(pinput != nullptr);
    const uint32* indices = pinput->GetIndices();
    // Find the normal sources
    const FCDGeometrySource* nsource = mesh->FindSourceByType(FUDaeGeometryInput::NORMAL);
    const FCDGeometryPolygonsInput* ninput = polygons->FindInput(FUDaeGeometryInput::NORMAL);
    const uint32* nindices;
    if (ninput != nullptr) nindices = ninput->GetIndices();
    // Find texcoord sources
    const FCDGeometrySource* tcsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXCOORD);
    const FCDGeometryPolygonsInput* tcinput = polygons->FindInput(FUDaeGeometryInput::TEXCOORD);
    const uint32* tcindices;
    if (tcinput != nullptr) tcindices = tcinput->GetIndices();
    // Find vcolor sources
    const FCDGeometrySource* csource = mesh->FindSourceByType(FUDaeGeometryInput::COLOR);
    const FCDGeometryPolygonsInput* cinput = polygons->FindInput(FUDaeGeometryInput::COLOR);
    const uint32* cindices;
    if (cinput != nullptr) cindices = cinput->GetIndices();
    // Find binormal sources
    const FCDGeometrySource* bsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXBINORMAL);
    const FCDGeometryPolygonsInput* binput = polygons->FindInput(FUDaeGeometryInput::TEXBINORMAL);
    const uint32* bindices;
    if (binput != nullptr) bindices = binput->GetIndices();
    // Find tangent sources
    const FCDGeometrySource* tsource = mesh->FindSourceByType(FUDaeGeometryInput::TEXTANGENT);
    const FCDGeometryPolygonsInput* tinput = polygons->FindInput(FUDaeGeometryInput::TEXTANGENT);
    const uint32* tindices;
    if (tinput != nullptr) tindices = tinput->GetIndices();
    // Get a name for potential coordinate sets
    string tcsetname;
    if (materials != nullptr && tcinput != nullptr) {
      if (daeegg_cat.is_debug()) {
        daeegg_cat.debug()
          << "Assigning texcoord set " << tcinput->GetSet()
          << " to semantic '" << material_semantic << "'\n";
      }
      tcsetname = materials->get_uvset_name(material_semantic,
                    FUDaeGeometryInput::TEXCOORD, tcinput->GetSet());
    }
    string tbsetname;
    if (materials != nullptr && binput != nullptr) {
      if (daeegg_cat.is_debug()) {
        daeegg_cat.debug()
          << "Assigning texbinormal set " << binput->GetSet()
          << " to semantic '" << material_semantic << "'\n";
      }
      tbsetname = materials->get_uvset_name(material_semantic,
                    FUDaeGeometryInput::TEXBINORMAL, binput->GetSet());
    }
    string ttsetname;
    if (materials != nullptr && tinput != nullptr) {
      if (daeegg_cat.is_debug()) {
        daeegg_cat.debug()
          << "Assigning textangent set " << tinput->GetSet()
          << " to semantic '" << material_semantic << "'\n";
        }
      ttsetname = materials->get_uvset_name(material_semantic,
                    FUDaeGeometryInput::TEXTANGENT, tinput->GetSet());
    }
    // Loop through the indices and add the vertices.
    for (size_t ix = 0; ix < pinput->GetIndexCount(); ++ix) {
      PT_EggVertex vertex = mesh_pool->make_new_vertex();
      const float* data = &vsource->GetData()[indices[ix]*3];
      vertex->set_pos(LPoint3d(data[0], data[1], data[2]));

      if (character != nullptr) {
        // If this is skinned geometry, add the vertex influences.
        character->influence_vertex(indices[ix], vertex);
      }

      // Process the normal
      if (nsource != nullptr && ninput != nullptr) {
        assert(nsource->GetStride() == 3);
        data = &nsource->GetData()[nindices[ix]*3];
        vertex->set_normal(LVecBase3d(data[0], data[1], data[2]));
      }
      // Process the texcoords
      if (tcsource != nullptr && tcinput != nullptr) {
        assert(tcsource->GetStride() == 2 || tcsource->GetStride() == 3);
        data = &tcsource->GetData()[tcindices[ix]*tcsource->GetStride()];
        if (tcsource->GetStride() == 2) {
          vertex->set_uv(tcsetname, LPoint2d(data[0], data[1]));
        } else {
          vertex->set_uvw(tcsetname, LPoint3d(data[0], data[1], data[2]));
        }
      }
      // Process the color
      if (csource != nullptr && cinput != nullptr) {
        assert(csource->GetStride() == 3 || csource->GetStride() == 4);
        if (csource->GetStride() == 3) {
          data = &csource->GetData()[cindices[ix]*3];
          vertex->set_color(LColor(data[0], data[1], data[2], 1.0f));
        } else {
          data = &csource->GetData()[cindices[ix]*4];
          vertex->set_color(LColor(data[0], data[1], data[2], data[3]));
        }
      }
      // Possibly add a UV object
      if ((bsource != nullptr && binput != nullptr) || (tsource != nullptr && tinput != nullptr)) {
        if (bsource != nullptr && binput != nullptr) {
          assert(bsource->GetStride() == 3);
          data = &bsource->GetData()[bindices[ix]*3];
          PT(EggVertexUV) uv_obj = vertex->modify_uv_obj(tbsetname);
          if (uv_obj == nullptr) {
            uv_obj = new EggVertexUV(tbsetname, LTexCoordd());
          }
          uv_obj->set_binormal(LVecBase3d(data[0], data[1], data[2]));
        }
        if (tsource != nullptr && tinput != nullptr) {
          assert(tsource->GetStride() == 3);
          data = &tsource->GetData()[tindices[ix]*3];
          PT(EggVertexUV) uv_obj = vertex->modify_uv_obj(ttsetname);
          if (uv_obj == nullptr) {
            uv_obj = new EggVertexUV(ttsetname, LTexCoordd());
          }
          uv_obj->set_tangent(LVecBase3d(data[0], data[1], data[2]));
        }
      }
      vertex->transform(parent->get_node_to_vertex());
    }
  }
  // Loop again for the polygons
  for (size_t gr = 0; gr < mesh->GetPolygonsCount(); ++gr) {
    const FCDGeometryPolygons* polygons = mesh->GetPolygons(gr);
    // Now loop through the faces
    uint32 offset = 0;
    for (size_t fa = 0; fa < polygons->GetFaceVertexCountCount(); ++fa) {
      PT(EggPrimitive) primitive = nullptr;
      // Create a primitive that matches the fcollada type
      switch (polygons->GetPrimitiveType()) {
      case FCDGeometryPolygons::LINES:
        primitive = new EggLine();
        break;
      case FCDGeometryPolygons::POLYGONS:
        primitive = new EggPolygon();
        break;
      case FCDGeometryPolygons::TRIANGLE_FANS:
        primitive = new EggTriangleFan();
        break;
      case FCDGeometryPolygons::TRIANGLE_STRIPS:
        primitive = new EggTriangleStrip();
        break;
      case FCDGeometryPolygons::POINTS:
        primitive = new EggPoint();
        break;
      case FCDGeometryPolygons::LINE_STRIPS:
        daeegg_cat.warning() << "Linestrips not yet supported!" << endl;
        break;
      default:
        daeegg_cat.warning() << "Unsupported primitive type found!" << endl;
      }
      if (primitive != nullptr) {
        primitive_holders[gr]->add_child(primitive);
        if (materials != nullptr) {
          materials->apply_to_primitive(FROM_FSTRING(polygons->GetMaterialSemantic()), primitive);
        }
        for (size_t ve = 0; ve < polygons->GetFaceVertexCount(fa); ++ve) {
          assert(mesh_pool->has_vertex(ve + polygons->GetFaceVertexOffset() + offset));
          primitive->add_vertex(mesh_pool->get_vertex(ve + polygons->GetFaceVertexOffset() + offset));
        }
      }
      offset += polygons->GetFaceVertexCount(fa);
    }
  }
  delete[] primitive_holders;
}

void DAEToEggConverter::
process_spline(EggGroup *parent, const string group_name, FCDGeometrySpline* geometry_spline) {
  assert(geometry_spline != nullptr);
  PT(EggGroup) result = new EggGroup(group_name);
  parent->add_child(result);
  // TODO: if its not a nurbs, make it convert between the types
  if (geometry_spline->GetType() != FUDaeSplineType::NURBS) {
    daeegg_cat.warning() << "Only NURBS curves are supported (yet)!" << endl;
  } else {
    // Loop through the splines
    for (size_t sp = 0; sp < geometry_spline->GetSplineCount(); ++sp) {
      process_spline(result, geometry_spline->GetSpline(sp));
    }
  }
}

void DAEToEggConverter::
process_spline(EggGroup *parent, const FCDSpline* spline) {
  assert(spline != nullptr);
  nassertv(spline->GetSplineType() == FUDaeSplineType::NURBS);
  // Now load in the nurbs curve to the egg library
  PT(EggNurbsCurve) nurbs_curve = new EggNurbsCurve(FROM_FSTRING(spline->GetName()));
  parent->add_child(nurbs_curve);
  // TODO: what value is this?
  nurbs_curve->setup(0, ((const FCDNURBSSpline*) spline)->GetKnotCount());
  for (size_t kn = 0; kn < ((const FCDNURBSSpline*) spline)->GetKnotCount(); ++kn) {
    const float* knot = ((const FCDNURBSSpline*) spline)->GetKnot(kn);
    assert(knot != nullptr);
    nurbs_curve->set_knot(kn, *knot);
  }
  for (size_t cv = 0; cv < spline->GetCVCount(); ++cv) {
    PT_EggVertex c_vtx = new EggVertex();
    c_vtx->set_pos(TO_VEC3(*spline->GetCV(cv)));
    c_vtx->transform(parent->get_node_to_vertex());
    nurbs_curve->add_vertex(c_vtx);
  }
}

void DAEToEggConverter::
process_controller(EggGroup *parent, const FCDControllerInstance *instance) {
  assert(instance != nullptr);
  const FCDController* controller = (const FCDController *)instance->GetEntity();
  assert(controller != nullptr);

  if (get_animation_convert() == AC_none) {
    // If we're exporting a static mesh, export the base geometry as-is.
    const FCDGeometryMesh *mesh = controller->GetBaseGeometry()->GetMesh();
    if (mesh != nullptr) {
      PT(DaeMaterials) materials = new DaeMaterials(instance);
      if (daeegg_cat.is_spam()) {
        daeegg_cat.spam() << "Processing mesh for controller\n";
      }
      process_mesh(parent, mesh, materials);
    }
  } else {
    // Add a character for this to the table, the mesh is processed later
    PT(DaeCharacter) character = new DaeCharacter(parent, instance);
    _characters.push_back(character);
  }

  if (controller->IsMorph()) {
    assert(controller != nullptr);
    const FCDMorphController* morph_controller = controller->GetMorphController();
    assert(morph_controller != nullptr);
    PT(EggTable) bundle = new EggTable(parent->get_name());
    bundle->set_table_type(EggTable::TT_bundle);
    PT(EggTable) morph = new EggTable("morph");
    morph->set_table_type(EggTable::TT_table);
    bundle->add_child(morph);
    // Loop through the morph targets.
    for (size_t mt = 0; mt < morph_controller->GetTargetCount(); ++mt) {
      const FCDMorphTarget* morph_target = morph_controller->GetTarget(mt);
      assert(morph_target != nullptr);
      PT(EggSAnimData) target = new EggSAnimData(FROM_FSTRING(morph_target->GetGeometry()->GetName()));
      if (morph_target->IsAnimated()) {
        // TODO
      } else {
        target->add_data(morph_target->GetWeight());
      }
      morph->add_child(target);
    }
  }
}

void DAEToEggConverter::
process_extra(EggGroup *group, const FCDExtra* extra) {
  if (extra == nullptr) {
    return;
  }
  nassertv(group != nullptr);

  const FCDEType* etype = extra->GetDefaultType();
  if (etype == nullptr) {
    return;
  }

  const FCDENode* enode = (const FCDENode*) etype->FindTechnique("PANDA3D");
  if (enode == nullptr) {
    return;
  }

  FCDENodeList tags;
  enode->FindChildrenNodes("param", tags);
  for (FCDENodeList::iterator it = tags.begin(); it != tags.end(); ++it) {
    const FCDEAttribute* attr = (*it)->FindAttribute("sid");
    if (attr) {
      group->set_tag(FROM_FSTRING(attr->GetValue()), (*it)->GetContent());
    }
  }
}

LMatrix4d DAEToEggConverter::
convert_matrix(const FMMatrix44 &matrix) {
  return LMatrix4d(
    matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3],
    matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3],
    matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3],
    matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
}

void DAEToEggConverter::
apply_transform(EggGroup *to, const FCDTransform* from) {
  assert(from != nullptr);
  assert(to != nullptr);
  // to->set_transform3d(convert_matrix(from->ToMatrix()) *
  // to->get_transform3d());
  switch (from->GetType()) {
  case FCDTransform::TRANSLATION:
    {
      const FCDTTranslation *trans = (const FCDTTranslation *)from;
      to->add_translate3d(TO_VEC3(trans->GetTranslation()));
    }
    break;

  case FCDTransform::ROTATION:
    {
      const FCDTRotation *rot = (const FCDTRotation *)from;
      to->add_rotate3d(rot->GetAngle(), TO_VEC3(rot->GetAxis()));
    }
    break;

  case FCDTransform::SCALE:
    {
      const FCDTScale *scale = (const FCDTScale *)from;
      to->add_scale3d(TO_VEC3(scale->GetScale()));
    }
    break;

  default:
    // Either a matrix, or something we can't handle.
    to->add_matrix4(convert_matrix(from->ToMatrix()));
    break;
  }
}
